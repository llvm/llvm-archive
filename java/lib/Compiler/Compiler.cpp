//===-- Compiler.cpp - Java bytecode compiler -------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains Java bytecode to LLVM bytecode compiler.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "javacompiler"

#include <llvm/Java/Compiler.h>
#include "BasicBlockBuilder.h"
#include "Locals.h"
#include "OperandStack.h"
#include "Resolver.h"
#include <llvm/Java/Bytecode.h>
#include <llvm/Java/BytecodeParser.h>
#include <llvm/Java/ClassFile.h>
#include <llvm/Constants.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Value.h>
#include <llvm/Type.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/SetVector.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/Support/CFG.h>
#include <llvm/Support/Debug.h>
#include <list>
#include <vector>

#define LLVM_JAVA_OBJECT_BASE "struct.llvm_java_object_base"
#define LLVM_JAVA_OBJECT_HEADER "struct.llvm_java_object_header"
#define LLVM_JAVA_OBJECT_TYPEINFO "struct.llvm_java_object_typeinfo"
#define LLVM_JAVA_OBJECT_VTABLE "struct.llvm_java_object_vtable"

#define LLVM_JAVA_STATIC_INIT "llvm_java_static_init"

using namespace llvm;
using namespace llvm::Java;

Type* llvm::Java::VTableBaseTy = OpaqueType::get();
Type* llvm::Java::VTableBaseRefTy = PointerType::get(VTableBaseTy);

namespace llvm { namespace Java { namespace {

  const std::string TMP("tmp");

  llvm::Constant* ONE = ConstantSInt::get(Type::IntTy, 1);
  llvm::Constant* ZERO = ConstantSInt::get(Type::IntTy, 0);
  llvm::Constant* MINUS_ONE = ConstantSInt::get(Type::IntTy, -1);

  llvm::Constant* INT_SHIFT_MASK = ConstantUInt::get(Type::UByteTy, 0x1f);
  llvm::Constant* LONG_SHIFT_MASK = ConstantUInt::get(Type::UByteTy, 0x3f);

  class Compiler : public BytecodeParser<Compiler> {
    Module* module_;
    std::auto_ptr<Resolver> resolver_;
    GlobalVariable* JNIEnvPtr_;
    const Class* class_;
    std::auto_ptr<BasicBlockBuilder> bbBuilder_;
    std::list<BasicBlock*> bbWorkList_;
    typedef std::map<BasicBlock*, unsigned> OpStackDepthMap;
    OpStackDepthMap opStackDepthMap_;
    typedef std::map<std::string, GlobalVariable*> StringMap;
    StringMap stringMap_;
    BasicBlock* currentBB_;
    Locals locals_;
    OperandStack opStack_;
    Function *getVtable_, *setVtable_, *throw_, *isInstanceOf_,
      *memcpy_, *memset_;

    typedef SetVector<Function*> FunctionSet;
    FunctionSet toCompileFunctions_;

    /// This class contains the vtable of a class, a vector with the
    /// vtables of its super classes (with the class higher in the
    /// hierarchy first). It also contains a map from methods to
    /// struct indices for this class (used to index into the vtable).
    struct VTableInfo {
      VTableInfo() : vtable(NULL) { }
      GlobalVariable* vtable;
      std::vector<llvm::Constant*> superVtables;
      typedef std::map<std::string, unsigned> Method2IndexMap;
      typedef Method2IndexMap::iterator iterator;
      typedef Method2IndexMap::const_iterator const_iterator;
      Method2IndexMap m2iMap;

      static StructType* VTableTy;
      static StructType* TypeInfoTy;
    };
    typedef std::map<const Class*, VTableInfo> Class2VTableInfoMap;
    Class2VTableInfoMap c2viMap_;

  public:
    Compiler(Module* m)
      : module_(m),
        resolver_(new Resolver(module_)),
        locals_(resolver_.get(), 0),
        opStack_(resolver_.get(), 0) {
      Type* JNIEnvTy = OpaqueType::get();
      module_->addTypeName("JNIEnv", JNIEnvTy);
      JNIEnvPtr_ = new GlobalVariable(JNIEnvTy,
                                      true,
                                      GlobalVariable::ExternalLinkage,
                                      NULL,
                                      "llvm_java_JNIEnv",
                                      module_);
      module_->addTypeName("llvm_java_object_vtable", VTableBaseTy);
      getVtable_ = module_->getOrInsertFunction(
        "llvm_java_get_vtable", VTableBaseRefTy,
        resolver_->getObjectBaseRefType(), NULL);
      setVtable_ = module_->getOrInsertFunction(
        "llvm_java_set_vtable", Type::VoidTy,
        resolver_->getObjectBaseRefType(), VTableBaseRefTy, NULL);
      throw_ = module_->getOrInsertFunction(
        "llvm_java_throw", Type::IntTy,
        resolver_->getObjectBaseRefType(), NULL);
      isInstanceOf_ = module_->getOrInsertFunction(
        "llvm_java_is_instance_of", Type::IntTy,
        resolver_->getObjectBaseRefType(), VTableBaseRefTy, NULL);
      memcpy_ = module_->getOrInsertFunction(
        "llvm.memcpy", Type::VoidTy,
        PointerType::get(Type::SByteTy),
        PointerType::get(Type::SByteTy),
        Type::ULongTy, Type::UIntTy, NULL);
      memset_ = module_->getOrInsertFunction(
        "llvm.memset", Type::VoidTy,
        PointerType::get(Type::SByteTy),
        Type::UByteTy, Type::ULongTy, Type::UIntTy, NULL);
    }

  private:
    void push(Value* value) {
      opStack_.push(value, currentBB_);
    }

    Value* pop(const Type* type) {
      return opStack_.pop(type, currentBB_);
    }

    /// Schedule a method for compilation. Returns true if this is the
    /// first time this function was scheduled.
    bool scheduleFunction(Function* function) {
      if (toCompileFunctions_.insert(function)) {
        DEBUG(std::cerr << "Scheduling function: " << function->getName()
              << " for compilation\n");
        return true;
      }
      return false;
    }

    template <typename InsertionPointTy>
    void initializeString(Value* globalString,
                          const std::string& str,
                          InsertionPointTy* ip) {
      // Create a new byte[] object and initialize it with the
      // contents of this string constant.
      Value* count = ConstantUInt::get(Type::UIntTy, str.size());
      Value* arrayRef = allocateArray(resolver_->getClass("[B"),
                                      &getPrimitiveArrayVTableInfo(BYTE),
                                      count,
                                      ip);
      // Copy string data.
      std::vector<Value*> indices;
      indices.reserve(3);
      indices.push_back(ConstantUInt::get(Type::UIntTy, 0));
      indices.push_back(ConstantUInt::get(Type::UIntTy, 2));
      indices.push_back(ConstantUInt::get(Type::UIntTy, 0));
      Value* arrayData = new GetElementPtrInst(arrayRef, indices, TMP, ip);
      llvm::Constant* init = ConstantArray::get(str);
      GlobalVariable* chars = new GlobalVariable(
        init->getType(),
        true,
        GlobalVariable::InternalLinkage,
        init,
        str + ".str",
        module_);

      std::vector<Value*> params;
      params.reserve(4);
      params.clear();
      params.push_back(arrayData);
      params.push_back(ConstantExpr::getPtrPtrFromArrayPtr(chars));
      params.push_back(new CastInst(count, Type::ULongTy, TMP, ip));
      params.push_back(ConstantUInt::get(Type::UIntTy, 0));
      new CallInst(memcpy_, params, "", ip);

      // Get class information for java/lang/String.
      const Class* clazz = resolver_->getClass("java/lang/String");
      const VTableInfo* vi = getVTableInfoGeneric(clazz);

      // Install the vtable pointer.
      Value* objBase =
        new CastInst(globalString, resolver_->getObjectBaseRefType(), TMP, ip);
      Value* vtable = new CastInst(vi->vtable, VTableBaseRefTy, TMP, ip);
      new CallInst(setVtable_, objBase, vtable, "", ip);

      // Initialize it: call java/lang/String/<init>(byte[],int)
      Method* method = getMethod("java/lang/String/<init>([BI)V");
      Function* function = getFunction(method);
      scheduleFunction(function);

      params.reserve(3);
      params.clear();
      params.push_back(objBase);
      params.push_back(new CastInst(arrayRef, resolver_->getObjectBaseRefType(), TMP, ip));
      params.push_back(ConstantSInt::get(Type::IntTy, 0));
      new CallInst(function, params, "", ip);
    }

    /// Returns the type of the Java string descriptor for JNI.
    const Type* getJNIType(ConstantUtf8* descr) {
      unsigned i = 0;
      return getJNITypeHelper(descr->str(), i);
    }

    const Type* getJNITypeHelper(const std::string& descr, unsigned& i) {
      assert(i < descr.size());
      switch (descr[i++]) {
      case 'B': return Type::SByteTy;
      case 'C': return Type::UShortTy;
      case 'D': return Type::DoubleTy;
      case 'F': return Type::FloatTy;
      case 'I': return Type::IntTy;
      case 'J': return Type::LongTy;
      case 'S': return Type::ShortTy;
      case 'Z': return Type::BoolTy;
      case 'V': return Type::VoidTy;
        // Both array and object types are pointers to llvm_object_base
      case 'L': {
        unsigned e = descr.find(';', i);
        i = e + 1;
        return resolver_->getObjectBaseRefType();
      }
      case '[':
        // Skip '['s.
        if (descr[i] == '[')
          do { ++i; } while (descr[i] == '[');
        // Consume the element type
        getJNITypeHelper(descr, i);
        return resolver_->getObjectBaseRefType();
      case '(': {
        std::vector<const Type*> params;
        // JNIEnv*
        params.push_back(JNIEnvPtr_->getType());
        params.push_back(resolver_->getObjectBaseRefType());
        while (descr[i] != ')')
          params.push_back(getJNITypeHelper(descr, i));
        return FunctionType::get(getJNITypeHelper(descr, ++i), params, false);
      }
        // FIXME: Throw something
      default:  assert(0 && "Cannot parse type descriptor!");
      }
      return 0; // not reached
    }

    /// Initializes the VTableInfo map; in other words it adds the
    /// VTableInfo for java.lang.Object.
    bool initializeVTableInfoMap() {
      DEBUG(std::cerr << "Building VTableInfo for: java/lang/Object\n");
      const Class* clazz = resolver_->getClass("java/lang/Object");
      VTableInfo& vi = c2viMap_[clazz];

      assert(!vi.vtable && vi.m2iMap.empty() &&
             "java/lang/Object VTableInfo should not be initialized!");

      Type* VTtype = OpaqueType::get();

      std::vector<const Type*> elements;
      std::vector<llvm::Constant*> init;

      // This is java/lang/Object so we must add a
      // llvm_java_object_typeinfo struct first.

      // depth
      elements.push_back(Type::IntTy);
      init.push_back(llvm::ConstantSInt::get(elements[0], 0));
      // superclasses vtable pointers
      elements.push_back(PointerType::get(PointerType::get(VTtype)));
      init.push_back(llvm::Constant::getNullValue(elements[1]));
      // last interface index
      elements.push_back(Type::IntTy);
      init.push_back(llvm::ConstantSInt::get(elements[2], -1));
      // interfaces vtable pointers
      elements.push_back(PointerType::get(PointerType::get(VTtype)));
      init.push_back(llvm::Constant::getNullValue(elements[3]));
      // the element size (0 for classes)
      elements.push_back(Type::IntTy);
      init.push_back(llvm::ConstantSInt::get(elements[4], 0));

      // This is a static variable.
      VTableInfo::TypeInfoTy = StructType::get(elements);
      module_->addTypeName(LLVM_JAVA_OBJECT_TYPEINFO, VTableInfo::TypeInfoTy);
      llvm::Constant* typeInfoInit =
        ConstantStruct::get(VTableInfo::TypeInfoTy, init);

      // Now that we have both the type and initializer for the
      // llvm_java_object_typeinfo struct we can start adding the
      // function pointers.
      elements.clear();
      init.clear();

      /// First add the typeinfo struct itself.
      elements.push_back(typeInfoInit->getType());
      // Add the typeinfo block for this class.
      init.push_back(typeInfoInit);

      const Methods& methods = clazz->getClassFile()->getMethods();

      // Add member functions to the vtable.
      for (unsigned i = 0, e = methods.size(); i != e; ++i) {
        Method* method = methods[i];
        // Static methods, private instance methods and the contructor
        // are statically bound so we don't add them to the vtable.
        if (!method->isStatic() &&
            !method->isPrivate() &&
            method->getName()->str()[0] != '<') {
          std::string methodDescr =
            method->getName()->str() +
            method->getDescriptor()->str();

          std::string funcName = "java/lang/Object/" + methodDescr;
          const FunctionType* funcTy = cast<FunctionType>(
            resolver_->getType(method->getDescriptor()->str(), true));

          Function* vfun = module_->getOrInsertFunction(funcName, funcTy);
          scheduleFunction(vfun);

          unsigned& index = vi.m2iMap[methodDescr];
          if (!index) {
            index = elements.size();
            elements.resize(index + 1, NULL);
            init.resize(index + 1, NULL);
          }
          elements[index] = vfun->getType();
          init[index] = vfun;
        }
      }

      PATypeHolder holder = VTtype;
      cast<OpaqueType>(VTtype)->refineAbstractTypeTo(StructType::get(elements));

      VTableInfo::VTableTy = cast<StructType>(holder.get());
      module_->addTypeName("java/lang/Object<vtable>", VTableInfo::VTableTy);

      vi.vtable = new GlobalVariable(VTableInfo::VTableTy,
                                     true, GlobalVariable::ExternalLinkage,
                                     ConstantStruct::get(init),
                                     "java/lang/Object<vtable>",
                                     module_);
      DEBUG(std::cerr << "Built VTableInfo for: java/lang/Object\n");
      return true;
    }

    /// Builds the super classes' vtable array for this classfile and
    /// its corresponding VTable. The direct superclass goes first in
    /// the array.
    llvm::Constant*
    buildSuperClassesVTables(const Class* clazz, const VTableInfo& vi) const {
      std::vector<llvm::Constant*> superVtables(vi.superVtables.size());
      for (unsigned i = 0, e = vi.superVtables.size(); i != e; ++i)
        superVtables[i] = ConstantExpr::getCast(
          vi.superVtables[i],
          PointerType::get(VTableInfo::VTableTy));

      llvm::Constant* init = ConstantArray::get(
        ArrayType::get(PointerType::get(VTableInfo::VTableTy),
                       superVtables.size()),
        superVtables);

      GlobalVariable* vtablesArray = new GlobalVariable(
        init->getType(),
        true,
        GlobalVariable::ExternalLinkage,
        init,
        clazz->getClassFile()->getThisClass()->getName()->str() +
        "<superclassesvtables>",
        module_);

      return ConstantExpr::getPtrPtrFromArrayPtr(vtablesArray);
    }

    /// Builds an interface VTable for the specified <class,interface>
    /// pair.
    llvm::Constant* buildInterfaceVTable(const Class* clazz,
                                         const Class* interface) {
      DEBUG(std::cerr << "Building interface vtable: "
            << interface->getName() << " for: " << clazz->getName() << '\n');

      const VTableInfo& classVI = getVTableInfo(clazz);
      const VTableInfo& interfaceVI = getVTableInfo(interface);
      const Methods& methods = interface->getClassFile()->getMethods();

      // The size of the initializer will be 1 greater than the number
      // of methods for this interface (the first slot is the typeinfo
      // struct.
      std::vector<llvm::Constant*> init(interfaceVI.m2iMap.size()+1, NULL);
      init[0] = llvm::Constant::getNullValue(VTableInfo::TypeInfoTy);

      // For each method in this interface find the implementing
      // method in the class' VTable and add it to the appropriate
      // slot.
      for (VTableInfo::Method2IndexMap::const_iterator
             i = interfaceVI.m2iMap.begin(), e = interfaceVI.m2iMap.end();
           i != e; ++i) {
        if (!clazz->getClassFile()->isAbstract()) {
          assert(classVI.m2iMap.find(i->first) != classVI.m2iMap.end() &&
                 "Interface method not found in class definition!");
          unsigned classMethodIdx = classVI.m2iMap.find(i->first)->second;
          init[i->second] = cast<ConstantStruct>(
            classVI.vtable->getInitializer())->getOperand(classMethodIdx);
        }
        else
          init[i->second] =
            llvm::Constant::getNullValue(
              PointerType::get(VTableInfo::VTableTy));
      }

      llvm::Constant* vtable = ConstantStruct::get(init);
      const std::string& globalName =
        clazz->getName() + '+' + interface->getName() + "<vtable>";
      module_->addTypeName(globalName, vtable->getType());

      GlobalVariable* gv = new GlobalVariable(
        vtable->getType(),
        true,
        GlobalVariable::ExternalLinkage,
        vtable,
        globalName,
        module_);

      return ConstantExpr::getCast(gv, PointerType::get(VTableInfo::VTableTy));
    }

    void insertVtablesForInterface(std::vector<llvm::Constant*>& vtables,
                                   const Class* clazz,
                                   const Class* interface) {
      static llvm::Constant* nullVTable =
        llvm::Constant::getNullValue(PointerType::get(VTableInfo::VTableTy));

      assert(interface->isInterface() && "Classfile must be an interface!");
      unsigned index = interface->getInterfaceIndex();
      if (index >= vtables.size())
        vtables.resize(index+1, nullVTable);
      assert(vtables[index] == nullVTable && "Interface vtable already added!");
      vtables[index] = buildInterfaceVTable(clazz, interface);
    }

    /// Builds the interfaces vtable array for this classfile and its
    /// corresponding VTableInfo. If this classfile is an interface we
    /// return a pointer to 0xFFFFFFFF.
    std::pair<int, llvm::Constant*>
    buildInterfacesVTables(const Class* clazz, const VTableInfo& vi) {
      // If this is an interface then we are not implementing any
      // interfaces so the lastInterface field is our index and the
      // pointer to the array of interface vtables is an all-ones
      // value.
      if (clazz->isInterface())
        return std::make_pair(
          clazz->getInterfaceIndex(),
          ConstantExpr::getCast(
            ConstantIntegral::getAllOnesValue(Type::LongTy),
            PointerType::get(PointerType::get(VTableInfo::VTableTy))));

      // Otherwise we must fill in the interfaces vtables array. For
      // each implemented interface we insert a pointer to the
      // <class,interface> vtable for this class. Note that we only
      // fill in up to the highest index of the implemented
      // interfaces.
      std::vector<llvm::Constant*> vtables;
      llvm::Constant* nullVTable =
        llvm::Constant::getNullValue(PointerType::get(VTableInfo::VTableTy));

      for (unsigned i = 0, e = clazz->getNumInterfaces(); i != e; ++i)
        insertVtablesForInterface(vtables, clazz, clazz->getInterface(i));

      const std::string& globalName = clazz->getName() + "<interfacesvtables>";

      llvm::Constant* init = ConstantArray::get(
        ArrayType::get(PointerType::get(VTableInfo::VTableTy), vtables.size()),
        vtables);
      module_->addTypeName(globalName, init->getType());

      GlobalVariable* interfacesArray = new GlobalVariable(
        init->getType(),
        true,
        GlobalVariable::ExternalLinkage,
        init,
        globalName,
        module_);

      return std::make_pair(
        int(vtables.size())-1,
        ConstantExpr::getPtrPtrFromArrayPtr(interfacesArray));
    }

    /// Given the classfile and its corresponding VTableInfo,
    /// construct the typeinfo constant for it.
    llvm::Constant* buildClassTypeInfo(const Class* clazz,
                                       const VTableInfo& vi) {
      std::vector<llvm::Constant*> typeInfoInit;

      llvm::Constant* superClassesVTables = buildSuperClassesVTables(clazz, vi);

      // The depth (java/lang/Object has depth 0).
      typeInfoInit.push_back(
        ConstantSInt::get(Type::IntTy, clazz->getNumSuperClasses()));
      // The super classes' vtables.
      typeInfoInit.push_back(superClassesVTables);

      int lastInterface;
      llvm::Constant* interfacesVTables;
      tie(lastInterface, interfacesVTables) = buildInterfacesVTables(clazz, vi);

      // The last interface index or the interface index if this is an
      // interface.
      typeInfoInit.push_back(ConstantSInt::get(Type::IntTy, lastInterface));
      // The interfaces' vtables.
      typeInfoInit.push_back(interfacesVTables);
      // the element size (0 for classes)
      typeInfoInit.push_back(llvm::ConstantSInt::get(Type::IntTy, 0));

      return ConstantStruct::get(VTableInfo::TypeInfoTy, typeInfoInit);
    }

    /// Returns the VTableInfo associated with this classfile.
    const VTableInfo& getVTableInfo(const Class* clazz) {
      static bool initialized = initializeVTableInfoMap();

      Class2VTableInfoMap::iterator it = c2viMap_.lower_bound(clazz);
      if (it != c2viMap_.end() && it->first == clazz)
        return it->second;

      const std::string& className =
        clazz->getClassFile()->getThisClass()->getName()->str();
      DEBUG(std::cerr << "Building VTableInfo for: " << className << '\n');
      VTableInfo& vi = c2viMap_[clazz];

      assert(!vi.vtable && vi.m2iMap.empty() &&
             "got already initialized VTableInfo!");

      std::vector<llvm::Constant*> init(1);
      // Use a null typeinfo struct for now.
      init[0] = llvm::Constant::getNullValue(VTableInfo::TypeInfoTy);

      // If this is an interface, add all methods from each interface
      // this inherits from.
      if (clazz->isInterface()) {
        for (unsigned i = 0, e = clazz->getNumInterfaces(); i != e; ++i) {
          const Class* interface = clazz->getInterface(i);
          const VTableInfo& ifaceVI = getVTableInfo(interface);
          const ClassFile* ifaceCF = interface->getClassFile();
          ConstantStruct* ifaceInit =
            cast<ConstantStruct>(ifaceVI.vtable->getInitializer());
          for (VTableInfo::const_iterator MI = ifaceVI.m2iMap.begin(),
                 ME = ifaceVI.m2iMap.end(); MI != ME; ++MI) {
            const std::string& methodDescr = MI->first;
            unsigned slot = MI->second;

            unsigned& index = vi.m2iMap[methodDescr];
            if (!index) {
              index = init.size();
              init.resize(index + 1);
            }
            init[index] = ifaceInit->getOperand(slot);
          }
        }
      }
      // Otherwise this is a class, so add all methods from its super
      // class.
      else {
        const Class* superClass = clazz->getSuperClass();
        assert(superClass && "Class does not have superclass!");
        const VTableInfo& superVI = getVTableInfo(superClass);

        // Copy the super vtables array.
        vi.superVtables.reserve(superVI.superVtables.size() + 1);
        vi.superVtables.push_back(superVI.vtable);
        std::copy(superVI.superVtables.begin(), superVI.superVtables.end(),
                  std::back_inserter(vi.superVtables));

        assert(superVI.vtable && "No vtable found for super class!");
        ConstantStruct* superInit =
          cast<ConstantStruct>(superVI.vtable->getInitializer());
        // Fill in the function pointers as they are in the super
        // class. Overriden methods will be replaced later.
        init.resize(superInit->getNumOperands());
        for (unsigned i = 1, e = superInit->getNumOperands(); i != e; ++i)
          init[i] = superInit->getOperand(i);
        vi.m2iMap = superVI.m2iMap;
      }

      // Add member functions to the vtable.
      const Methods& methods = clazz->getClassFile()->getMethods();

      for (unsigned i = 0, e = methods.size(); i != e; ++i) {
        Method* method = methods[i];
        // Static methods, private instance methods and the contructor
        // are statically bound so we don't add them to the vtable.
        if (!method->isStatic() &&
            !method->isPrivate() &&
            method->getName()->str()[0] != '<') {
          const std::string& methodDescr =
            method->getName()->str() + method->getDescriptor()->str();

          std::string funcName = className + '/' + methodDescr;

          const FunctionType* funcTy = cast<FunctionType>(
            resolver_->getType(method->getDescriptor()->str(), true));
          llvm::Constant* vfun = NULL;
          if (clazz->isInterface() || method->isAbstract())
            vfun = llvm::Constant::getNullValue(PointerType::get(funcTy));
          else {
            vfun = module_->getOrInsertFunction(funcName, funcTy);
            scheduleFunction(cast<Function>(vfun));
          }

          unsigned& index = vi.m2iMap[methodDescr];
          if (!index) {
            index = init.size();
            init.resize(index + 1);
          }
          init[index] = vfun;
        }
      }

#ifndef NDEBUG
      for (unsigned i = 0, e = init.size(); i != e; ++i)
        assert(init[i] && "No elements in the initializer should be NULL!");
#endif

      const std::string& globalName = className + "<vtable>";

      llvm::Constant* vtable = ConstantStruct::get(init);
      module_->addTypeName(globalName, vtable->getType());
      vi.vtable = new GlobalVariable(vtable->getType(),
                                     true,
                                     GlobalVariable::ExternalLinkage,
                                     vtable,
                                     globalName,
                                     module_);

      // Now the vtable is complete, install the new typeinfo block
      // for this class: we install it last because we need the vtable
      // to exist in order to build it.
      init[0] = buildClassTypeInfo(clazz, vi);
      vi.vtable->setInitializer(ConstantStruct::get(init));

      DEBUG(std::cerr << "Built VTableInfo for: " << className << '\n');
      return vi;
    }

    VTableInfo buildArrayVTableInfo(const Type* elementTy) {
      VTableInfo vi;
      const VTableInfo& superVI =
        getVTableInfo(resolver_->getClass("java/lang/Object"));

      // Add java/lang/Object as its superclass.
      vi.superVtables.reserve(1);
      vi.superVtables.push_back(superVI.vtable);

      // Copy the constants from java/lang/Object vtable.
      ConstantStruct* superInit =
        cast<ConstantStruct>(superVI.vtable->getInitializer());
      std::vector<llvm::Constant*> init(superInit->getNumOperands());
      // Use a null typeinfo struct for now.
      init[0] = llvm::Constant::getNullValue(VTableInfo::TypeInfoTy);

      // Fill in the function pointers as they are in
      // java/lang/Object. There are no overriden methods.
      for (unsigned i = 1, e = superInit->getNumOperands(); i != e; ++i)
        init[i] = superInit->getOperand(i);
      vi.m2iMap = superVI.m2iMap;

#ifndef NDEBUG
      for (unsigned i = 0, e = init.size(); i != e; ++i)
        assert(init[i] && "No elements in the initializer should be NULL!");
#endif

      const std::string& globalName =
        elementTy->getDescription() + "[]<vtable>";

      llvm::Constant* vtable = ConstantStruct::get(init);
      module_->addTypeName(globalName, vtable->getType());
      vi.vtable = new GlobalVariable(vtable->getType(),
                                     true,
                                     GlobalVariable::ExternalLinkage,
                                     vtable,
                                     globalName,
                                     module_);

      // Construct the typeinfo now.
      std::vector<llvm::Constant*> typeInfoInit;
      typeInfoInit.push_back(ConstantSInt::get(Type::IntTy, 1));
      // Build the super classes' vtable array.
      ArrayType* vtablesArrayTy =
        ArrayType::get(PointerType::get(VTableInfo::VTableTy),
                       vi.superVtables.size());

      GlobalVariable* vtablesArray = new GlobalVariable(
        vtablesArrayTy,
        true,
        GlobalVariable::ExternalLinkage,
        ConstantArray::get(vtablesArrayTy, vi.superVtables),
        elementTy->getDescription() + "[]<superclassesvtables>",
        module_);

      typeInfoInit.push_back(ConstantExpr::getPtrPtrFromArrayPtr(vtablesArray));
      typeInfoInit.push_back(ConstantSInt::get(Type::IntTy, 0));
      typeInfoInit.push_back(
        llvm::Constant::getNullValue(
          PointerType::get(PointerType::get(VTableInfo::VTableTy))));
      // the element size
      typeInfoInit.push_back(
        ConstantExpr::getCast(
          ConstantExpr::getSizeOf(elementTy), Type::IntTy));

      init[0] = ConstantStruct::get(VTableInfo::TypeInfoTy, typeInfoInit);
      vi.vtable->setInitializer(ConstantStruct::get(init));

      return vi;
    }

    const VTableInfo& getPrimitiveArrayVTableInfo(const Type* type) {
      if (Type::BoolTy == type) return getPrimitiveArrayVTableInfo(BOOLEAN);
      else if (Type::UShortTy == type) return getPrimitiveArrayVTableInfo(CHAR);
      else if (Type::FloatTy == type) return getPrimitiveArrayVTableInfo(FLOAT);
      else if (Type::DoubleTy == type) return getPrimitiveArrayVTableInfo(DOUBLE);
      else if (Type::SByteTy == type) return getPrimitiveArrayVTableInfo(BYTE);
      else if (Type::ShortTy == type) return getPrimitiveArrayVTableInfo(SHORT);
      else if (Type::IntTy == type) return getPrimitiveArrayVTableInfo(INT);
      else if (Type::LongTy == type) return getPrimitiveArrayVTableInfo(LONG);
      else abort();
    }

    // Returns the VTableInfo object for an array of the specified
    // element type.
    const VTableInfo& getPrimitiveArrayVTableInfo(JType type) {
      switch (type) {
      case BOOLEAN: {
        // Because baload/bastore is used to load/store to both byte
        // arrays and boolean arrays we use sbyte for java boolean
        // arrays as well.
        static VTableInfo arrayInfo = buildArrayVTableInfo(Type::SByteTy);
        return arrayInfo;
      }
      case CHAR: {
        static VTableInfo arrayInfo = buildArrayVTableInfo(Type::UShortTy);
        return arrayInfo;
      }
      case FLOAT: {
        static VTableInfo arrayInfo = buildArrayVTableInfo(Type::FloatTy);
        return arrayInfo;
      }
      case DOUBLE: {
        static VTableInfo arrayInfo = buildArrayVTableInfo(Type::DoubleTy);
        return arrayInfo;
      }
      case BYTE: {
        static VTableInfo arrayInfo = buildArrayVTableInfo(Type::SByteTy);
        return arrayInfo;
      }
      case SHORT: {
        static VTableInfo arrayInfo = buildArrayVTableInfo(Type::ShortTy);
        return arrayInfo;
      }
      case INT: {
        static VTableInfo arrayInfo = buildArrayVTableInfo(Type::IntTy);
        return arrayInfo;
      }
      case LONG: {
        static VTableInfo arrayInfo = buildArrayVTableInfo(Type::LongTy);
        return arrayInfo;
      }
      }
      abort();
    }

    const VTableInfo& getObjectArrayVTableInfo() {
      static VTableInfo arrayInfo =
        buildArrayVTableInfo(resolver_->getObjectBaseRefType());

      return arrayInfo;
    }

    /// Emits the necessary code to get a pointer to a static field of
    /// an object.
    GlobalVariable* getStaticField(unsigned index) {
      ConstantFieldRef* fieldRef =
        class_->getClassFile()->getConstantFieldRef(index);
      ConstantNameAndType* nameAndType = fieldRef->getNameAndType();

      const std::string& className = fieldRef->getClass()->getName()->str();
      GlobalVariable* global =
        getStaticField(ClassFile::get(className),
                       nameAndType->getName()->str(),
                       resolver_->getType(nameAndType->getDescriptor()->str()));

      assert(global && "Cannot find global for static field!");

      return global;
    }

    /// Finds a static field in the specified class, any of its
    /// super clases, or any of the interfaces it implements.
    GlobalVariable* getStaticField(const ClassFile* cf,
                                   const std::string& name,
                                   const Type* type) {
      // Emit the static initializers for this class, making sure that
      // the globals are inserted into the module.
      emitStaticInitializers(cf);
      const std::string& className = cf->getThisClass()->getName()->str();
      const std::string& globalName = className + '/' + name;

      DEBUG(std::cerr << "Looking up global: " << globalName << '\n');
      GlobalVariable* global = module_->getGlobalVariable(globalName, type);
      if (global)
        return global;

      for (unsigned i = 0, e = cf->getNumInterfaces(); i != e; ++i) {
        const ClassFile* ifaceCF =
          ClassFile::get(cf->getInterface(i)->getName()->str());
        if (global = getStaticField(ifaceCF, name, type))
          return global;
      }

      // If we have no super class it means the lookup terminates
      // unsuccesfully.
      if (!cf->getSuperClass())
        return NULL;

      const ClassFile* superCF =
        ClassFile::get(cf->getSuperClass()->getName()->str());
      return getStaticField(superCF, name, type);
    }

    /// Emits the necessary code to get a field from the passed
    /// pointer to an object.
    Value* getField(unsigned index, Value* ptr) {
      ConstantFieldRef* fieldRef =
        class_->getClassFile()->getConstantFieldRef(index);
      ConstantNameAndType* nameAndType = fieldRef->getNameAndType();
      return getField(
        class_->getClass(fieldRef->getClassIndex()),
        nameAndType->getName()->str(),
        ptr);
    }

    /// Emits the necessary code to get a field from the passed
    /// pointer to an object.
    Value* getField(const Class* clazz,
                    const std::string& fieldName,
                    Value* ptr) {
      // Cast ptr to correct type.
      ptr = new CastInst(ptr, clazz->getType(), TMP, currentBB_);

      // Deref pointer.
      std::vector<Value*> indices(1, ConstantUInt::get(Type::UIntTy, 0));
      while (true) {
        int slot = clazz->getFieldIndex(fieldName);
        if (slot == -1) {
          indices.push_back(ConstantUInt::get(Type::UIntTy, 0));
          clazz = clazz->getSuperClass();
        }
        else {
          indices.push_back(ConstantUInt::get(Type::UIntTy, slot));
          break;
        }
      }

      return new GetElementPtrInst(ptr, indices, fieldName + '*', currentBB_);
    }

    std::string getMangledString(const std::string& str) {
      std::string mangledStr;

      for (unsigned i = 0, e = str.size(); i != e; ++i) {
        if (str[i] == '/')
          mangledStr += '_';
        else if (str[i] == '_')
          mangledStr += "_1";
        else if (str[i] == ';')
          mangledStr += "_2";
        else if (str[i] == '[')
          mangledStr += "_3";
        else
          mangledStr += str[i];
      }
      return mangledStr;
    }

    /// Compiles the passed method only (it does not compile any
    /// callers or methods of objects it creates).
    Function* compileMethodOnly(const std::string& classMethodDesc) {
      Method* method = getMethod(classMethodDesc);
      const std::string& className =
        method->getParent()->getThisClass()->getName()->str();
      class_ = resolver_->getClass(className);

      Function* function = getFunction(method);
      if (!function->empty()) {
        DEBUG(std::cerr << "Function: " << function->getName() << " is already compiled!\n");
        return function;
      }

      if (method->isNative()) {
        DEBUG(std::cerr << "Adding stub for natively implemented method: "
              << classMethodDesc << '\n');
        const FunctionType* jniFuncTy =
          cast<FunctionType>(getJNIType(method->getDescriptor()));

        std::string funcName =
          "Java_" +
          getMangledString(className) + '_' +
          getMangledString(method->getName()->str());
        if (class_->getClassFile()->isNativeMethodOverloaded(*method)) {
          // We need to add two underscores and a mangled argument signature
          funcName += "__";
          const std::string descr = method->getDescriptor()->str();
          funcName += getMangledString(
            std::string(descr.begin() + descr.find('(') + 1,
                        descr.begin() + descr.find(')')));
        }

        Function* jniFunction = module_->getOrInsertFunction(funcName,jniFuncTy);

        BasicBlock* bb = new BasicBlock("entry", function);
        std::vector<Value*> params;
        params.push_back(JNIEnvPtr_);
        if (method->isStatic())
          params.push_back(llvm::Constant::getNullValue(resolver_->getObjectBaseRefType()));
        for (Function::arg_iterator A = function->arg_begin(),
               E = function->arg_end(); A != E; ++A) {
          params.push_back(
            new CastInst(A, jniFuncTy->getParamType(params.size()), TMP, bb));
        }
        Value* result = new CallInst(jniFunction, params, "", bb);
        if (result->getType() != Type::VoidTy)
          result = new CastInst(result, function->getReturnType(), TMP,bb);
        new ReturnInst(result, bb);

        return function;
      }

      assert (!method->isAbstract() && "Trying to compile an abstract method!");

      // HACK: skip most of the class libraries.
      if ((classMethodDesc.find("java/") == 0 &&
           classMethodDesc.find("java/lang/Object") != 0 &&
           (classMethodDesc.find("java/lang/Throwable") != 0 ||
            classMethodDesc.find("java/lang/Throwable$StaticData/<cl") == 0) &&
           classMethodDesc.find("java/lang/Exception") != 0 &&
           classMethodDesc.find("java/lang/IllegalArgumentException") != 0 &&
           classMethodDesc.find("java/lang/IllegalStateException") != 0 &&
           classMethodDesc.find("java/lang/IndexOutOfBoundsException") != 0 &&
           classMethodDesc.find("java/lang/RuntimeException") != 0 &&
           classMethodDesc.find("java/lang/Math") != 0 &&
           classMethodDesc.find("java/lang/Number") != 0 &&
           classMethodDesc.find("java/lang/Byte") != 0 &&
           classMethodDesc.find("java/lang/Float") != 0 &&
           classMethodDesc.find("java/lang/Integer") != 0 &&
           classMethodDesc.find("java/lang/Long") != 0 &&
           classMethodDesc.find("java/lang/Short") != 0 &&
           (classMethodDesc.find("java/lang/String") != 0 ||
            classMethodDesc.find("java/lang/String/<cl") == 0) &&
           classMethodDesc.find("java/lang/StringBuffer") != 0 &&
           classMethodDesc.find("java/lang/System") != 0 &&
           classMethodDesc.find("java/lang/VMSystem") != 0 &&
           (classMethodDesc.find("java/util/") != 0 ||
            classMethodDesc.find("java/util/Locale/<cl") == 0 ||
            classMethodDesc.find("java/util/ResourceBundle/<cl") == 0 ||
            classMethodDesc.find("java/util/Calendar/<cl") == 0)) ||
          (classMethodDesc.find("gnu/") == 0)) {
        DEBUG(std::cerr << "Skipping compilation of method: "
              << classMethodDesc << '\n');
        return function;
      }

      DEBUG(std::cerr << "Compiling method: " << classMethodDesc << '\n');

      Java::CodeAttribute* codeAttr = method->getCodeAttribute();

      opStackDepthMap_.clear();
      bbBuilder_.reset(new BasicBlockBuilder(function, codeAttr));

      // Put arguments into locals.
      locals_ = Locals(resolver_.get(), codeAttr->getMaxLocals());

      unsigned index = 0;
      for (Function::arg_iterator a = function->arg_begin(),
             ae = function->arg_end(); a != ae; ++a) {
        locals_.store(index, a, &function->getEntryBlock());
        index += resolver_->isTwoSlotType(a->getType()) ? 2 : 1;
      }

      BasicBlock* bb0 = bbBuilder_->getBasicBlock(0);

      // For bb0 the operand stack is empty and the locals contain the
      // arguments to the function.
      //
      // NOTE: We create an operand stack one size too big because we
      // push extra values on the stack to simplify code generation
      // (see implementation of ifne).
      opStack_ = OperandStack(resolver_.get(), codeAttr->getMaxStack()+2);
      opStackDepthMap_.insert(std::make_pair(bb0, 0));

      // Insert bb0 in the work list.
      bbWorkList_.push_back(bb0);

      // Process the work list until we compile the whole function.
      while (!bbWorkList_.empty()) {
        currentBB_ = bbWorkList_.front();
        bbWorkList_.pop_front();

        OpStackDepthMap::iterator opStackDepth =
          opStackDepthMap_.find(currentBB_);
        assert(opStackDepth != opStackDepthMap_.end() &&
               "Unknown operand stack depth for basic block in work list!");

        opStack_.setDepth(opStackDepth->second);

        unsigned start, end;
        tie(start, end) = bbBuilder_->getBytecodeIndices(currentBB_);

        // Compile this basic block.
        parse(codeAttr->getCode(), start, end);

        // If this basic block does not have a terminator, it should
        // have an unconditional branch to the next basic block
        // (fallthrough).
        if (!currentBB_->getTerminator())
          new BranchInst(bbBuilder_->getBasicBlock(end), currentBB_);

        // For each successor of this basic block we can compute its
        // entry operand stack depth. We do so, and add it to the work
        // list. If a successor already has an entry operand stack and
        // locals we assume the computation was correct and do not add
        // it to the work list.
        for (succ_iterator
               SI = succ_begin(currentBB_), SE = succ_end(currentBB_);
             SI != SE; ++SI) {
          BasicBlock* Succ = *SI;
          OpStackDepthMap::iterator succOpStackDepth =
            opStackDepthMap_.lower_bound(Succ);
          if (succOpStackDepth == opStackDepthMap_.end() ||
              succOpStackDepth->first != Succ) {
            opStackDepthMap_.insert(succOpStackDepth,
                                    std::make_pair(Succ, opStack_.getDepth()));
            bbWorkList_.push_back(Succ);
          }
        }
      }

      // Add an unconditional branch from the entry block to bb0.
      new BranchInst(bb0, &function->getEntryBlock());

      // FIXME: remove empty basic blocks (we have empty basic blocks
      // because of our lack of exception support).
      for (Function::iterator bb = function->begin(), be = function->end();
           bb != be; )
        if (bb->empty())
          bb = function->getBasicBlockList().erase(bb);
        else
          ++bb;

      DEBUG(std::cerr << "Finished compilation of method: "
            << classMethodDesc << '\n');
      // DEBUG(function->dump());

      return function;
    }

    /// Emits static initializers for this class if not done already.
    void emitStaticInitializers(const ClassFile* classfile) {
      typedef SetVector<const ClassFile*> ClassFileSet;
      static ClassFileSet toInitClasses;

      if (toInitClasses.insert(classfile)) {
        // If this class has a super class, initialize that first.
        if (classfile->getSuperClass())
          emitStaticInitializers(
            ClassFile::get(classfile->getSuperClass()->getName()->str()));

        const std::string& className =
          classfile->getThisClass()->getName()->str();
        const Class* clazz = resolver_->getClass(className);

        // Create the global variables of this class.
        const Fields& fields = classfile->getFields();
        for (unsigned i = 0, e = fields.size(); i != e; ++i) {
          Field* field = fields[i];
          if (field->isStatic()) {
            const Type* globalTy = resolver_->getType(field->getDescriptor()->str());
            // A java field can be final/constant even if it has a
            // dynamic initializer. Because LLVM does not currently
            // support these semantics, we consider constants only
            // final fields with static initializers.
            bool isConstant = false;
            llvm::Constant* init = llvm::Constant::getNullValue(globalTy);
            if (field->getConstantValueAttribute()) {
              unsigned i = field->getConstantValueAttribute()->getValueIndex();
              init = ConstantExpr::getCast(clazz->getConstant(i), globalTy);
              isConstant = field->isFinal();
            }

            std::string globalName =
              classfile->getThisClass()->getName()->str() + '/' +
              field->getName()->str();
            DEBUG(std::cerr << "Adding global: " << globalName << '\n');
            new GlobalVariable(globalTy,
                               isConstant,
                               GlobalVariable::ExternalLinkage,
                               init,
                               globalName,
                               module_);
          }
        }

        Function* hook = module_->getOrInsertFunction(LLVM_JAVA_STATIC_INIT,
                                                      Type::VoidTy, 0);
        Instruction* I = hook->front().getTerminator();
        assert(I && LLVM_JAVA_STATIC_INIT " should have a terminator!");

        // Create constant strings for this class.
        for (unsigned i = 0, e = classfile->getNumConstants(); i != e; ++i)
          if (ConstantString* s = dynamic_cast<ConstantString*>(classfile->getConstant(i)))
            initializeString(clazz->getConstant(i), s->getValue()->str(), I);

        // Call its class initialization method if it exists.
        if (const Method* method = classfile->getMethod("<clinit>()V")) {
          const std::string& functionName = className + '/' +
            method->getName()->str() + method->getDescriptor()->str();
          Function* init =
            module_->getOrInsertFunction(functionName, Type::VoidTy, 0);

          // Insert a call to it right before the terminator of the only
          // basic block in llvm_java_static_init.
          bool inserted =  scheduleFunction(init);
          assert(inserted && "Class initialization method already called!");
          new CallInst(init, "", I);
        }
      }
    }

    /// Returns the llvm::Function corresponding to the specified
    /// llvm::Java::Method.
    Function* getFunction(Method* method) {
      const ClassFile* clazz = method->getParent();

      const FunctionType* funcTy = cast<FunctionType>(
        resolver_->getType(method->getDescriptor()->str(),
                           !method->isStatic()));
      std::string funcName =
        clazz->getThisClass()->getName()->str() + '/' +
        method->getName()->str() + method->getDescriptor()->str();

      Function* function = module_->getOrInsertFunction(funcName, funcTy);

      return function;
    }

    /// Returns the llvm::Java::Method given a
    /// llvm::Java::ClassMethodRef.
    Method* getMethod(ConstantMethodRef* methodRef) {
      return getMethod(methodRef->getClass()->getName()->str() + '/' +
                       methodRef->getNameAndType()->getName()->str() +
                       methodRef->getNameAndType()->getDescriptor()->str());
    }

    /// Returns the llvm::Java::Method given a <class,method>
    /// descriptor.
    Method* getMethod(const std::string& classMethodDesc) {
      unsigned slash = classMethodDesc.rfind('/', classMethodDesc.find('('));
      std::string className = classMethodDesc.substr(0, slash);
      std::string methodNameAndDescr = classMethodDesc.substr(slash+1);

      while (true) {
        const ClassFile* classfile = ClassFile::get(className);
        emitStaticInitializers(classfile);

        Method* method = classfile->getMethod(methodNameAndDescr);
        if (method)
          return method;

        if (!classfile->getSuperClass())
          break;

        className = classfile->getSuperClass()->getName()->str();
      }

      throw InvocationTargetException("Method " + methodNameAndDescr +
                                      " not found in class " + className);
    }

  public:
    /// Compiles the specified method given a <class,method>
    /// descriptor and the transitive closure of all methods
    /// (possibly) called by it.
    Function* compileMethod(const std::string& classMethodDesc) {
      // Initialize the static initializer function.
      Function* staticInit =
        module_->getOrInsertFunction(LLVM_JAVA_STATIC_INIT, Type::VoidTy, 0);
      BasicBlock* staticInitBB = new BasicBlock("entry", staticInit);
      new ReturnInst(NULL, staticInitBB);

      // Create the method requested.
      Function* function = getFunction(getMethod(classMethodDesc));
      scheduleFunction(function);
      // Compile the transitive closure of methods called by this method.
      for (unsigned i = 0; i != toCompileFunctions_.size(); ++i) {
        Function* f = toCompileFunctions_[i];
        compileMethodOnly(f->getName());
        DEBUG(std::cerr << i+1 << '/' << toCompileFunctions_.size()
              << " functions compiled\n");
      }

      return function;
    }

    void do_aconst_null() {
      push(llvm::Constant::getNullValue(resolver_->getObjectBaseRefType()));
    }

    void do_iconst(int value) {
      push(ConstantSInt::get(Type::IntTy, value));
    }

    void do_lconst(long long value) {
      push(ConstantSInt::get(Type::LongTy, value));
    }

    void do_fconst(float value) {
      push(ConstantFP::get(Type::FloatTy, value));
    }

    void do_dconst(double value) {
      push(ConstantFP::get(Type::DoubleTy, value));
    }

    void do_ldc(unsigned index) {
      llvm::Constant* c = class_->getConstant(index);
      assert(c && "Java constant not handled!");
      push(c);
    }

    void do_ldc2(unsigned index) {
      do_ldc(index);
    }

    void do_iload(unsigned index) { do_load_common(index, Type::IntTy); }
    void do_lload(unsigned index) { do_load_common(index, Type::LongTy); }
    void do_fload(unsigned index) { do_load_common(index, Type::FloatTy); }
    void do_dload(unsigned index) { do_load_common(index, Type::DoubleTy); }
    void do_aload(unsigned index) { do_load_common(index, resolver_->getObjectBaseRefType()); }

    void do_load_common(unsigned index, const Type* type) {
      Value* val = locals_.load(index, type, currentBB_);
      push(val);
    }

    void do_iaload() { do_aload_common("[I"); }
    void do_laload() { do_aload_common("[J"); }
    void do_faload() { do_aload_common("[F"); }
    void do_daload() { do_aload_common("[D"); }
    void do_aaload() { do_aload_common("[Ljava/lang/Object;"); }
    void do_baload() { do_aload_common("[B"); }
    void do_caload() { do_aload_common("[C"); }
    void do_saload() { do_aload_common("[S"); }

    void do_aload_common(const std::string& className) {
      const Class* arrayClass = resolver_->getClass(className);
      assert(arrayClass->isArray() && "Not an array class!");
      Value* index = pop(Type::IntTy);
      Value* arrayRef = pop(arrayClass->getType());

      std::vector<Value*> indices;
      indices.reserve(3);
      indices.push_back(ConstantUInt::get(Type::UIntTy, 0));
      indices.push_back(ConstantUInt::get(Type::UIntTy, 2));
      indices.push_back(index);
      Value* elementPtr =
        new GetElementPtrInst(arrayRef, indices, TMP, currentBB_);
      Value* result = new LoadInst(elementPtr, TMP, currentBB_);
      push(result);
    }

    void do_istore(unsigned index) { do_store_common(index, Type::IntTy); }
    void do_lstore(unsigned index) { do_store_common(index, Type::LongTy); }
    void do_fstore(unsigned index) { do_store_common(index, Type::FloatTy); }
    void do_dstore(unsigned index) { do_store_common(index, Type::DoubleTy); }
    void do_astore(unsigned index) { do_store_common(index, resolver_->getObjectBaseRefType()); }

    void do_store_common(unsigned index, const Type* type) {
      Value* val = pop(type);
      locals_.store(index, val, currentBB_);
    }

    void do_iastore() { do_astore_common("[I"); }
    void do_lastore() { do_astore_common("[J"); }
    void do_fastore() { do_astore_common("[F"); }
    void do_dastore() { do_astore_common("[D"); }
    void do_aastore() { do_astore_common("[Ljava/lang/Object;"); }
    void do_bastore() { do_astore_common("[B"); }
    void do_castore() { do_astore_common("[C"); }
    void do_sastore() { do_astore_common("[S"); }

    void do_astore_common(const std::string& className) {
      const Class* arrayClass = resolver_->getClass(className);
      assert(arrayClass->isArray() && "Not an array class!");
      const Class* componentClass = arrayClass->getComponentClass();
      Value* value = pop(componentClass->getType());
      Value* index = pop(Type::IntTy);
      Value* arrayRef = pop(arrayClass->getType());

      std::vector<Value*> indices;
      indices.reserve(3);
      indices.push_back(ConstantUInt::get(Type::UIntTy, 0));
      indices.push_back(ConstantUInt::get(Type::UIntTy, 2));
      indices.push_back(index);
      Value* elementPtr =
        new GetElementPtrInst(arrayRef, indices, TMP, currentBB_);

      new StoreInst(value, elementPtr, currentBB_);
    }

    void do_pop() {
      opStack_.do_pop(currentBB_);
    }

    void do_pop2() {
      opStack_.do_pop2(currentBB_);
    }

    void do_dup() {
      opStack_.do_dup(currentBB_);
    }

    void do_dup_x1() {
      opStack_.do_dup_x1(currentBB_);
    }

    void do_dup_x2() {
      opStack_.do_dup_x2(currentBB_);
    }

    void do_dup2() {
      opStack_.do_dup2(currentBB_);
    }

    void do_dup2_x1() {
      opStack_.do_dup2_x1(currentBB_);
    }

    void do_dup2_x2() {
      opStack_.do_dup2_x2(currentBB_);
    }

    void do_swap() {
      opStack_.do_swap(currentBB_);
    }

    void do_iadd() { do_binary_op_common(Instruction::Add, Type::IntTy); }
    void do_ladd() { do_binary_op_common(Instruction::Add, Type::LongTy); }
    void do_fadd() { do_binary_op_common(Instruction::Add, Type::FloatTy); }
    void do_dadd() { do_binary_op_common(Instruction::Add, Type::DoubleTy); }

    void do_isub() { do_binary_op_common(Instruction::Sub, Type::IntTy); }
    void do_lsub() { do_binary_op_common(Instruction::Sub, Type::LongTy); }
    void do_fsub() { do_binary_op_common(Instruction::Sub, Type::FloatTy); }
    void do_dsub() { do_binary_op_common(Instruction::Sub, Type::DoubleTy); }

    void do_imul() { do_binary_op_common(Instruction::Mul, Type::IntTy); }
    void do_lmul() { do_binary_op_common(Instruction::Mul, Type::LongTy); }
    void do_fmul() { do_binary_op_common(Instruction::Mul, Type::FloatTy); }
    void do_dmul() { do_binary_op_common(Instruction::Mul, Type::DoubleTy); }

    void do_idiv() { do_binary_op_common(Instruction::Div, Type::IntTy); }
    void do_ldiv() { do_binary_op_common(Instruction::Div, Type::LongTy); }
    void do_fdiv() { do_binary_op_common(Instruction::Div, Type::FloatTy); }
    void do_ddiv() { do_binary_op_common(Instruction::Div, Type::DoubleTy); }

    void do_irem() { do_binary_op_common(Instruction::Rem, Type::IntTy); }
    void do_lrem() { do_binary_op_common(Instruction::Rem, Type::LongTy); }
    void do_frem() { do_binary_op_common(Instruction::Rem, Type::FloatTy); }
    void do_drem() { do_binary_op_common(Instruction::Rem, Type::DoubleTy); }

    void do_ineg() { do_neg_common(Type::IntTy); }
    void do_lneg() { do_neg_common(Type::LongTy); }
    void do_fneg() { do_neg_common(Type::FloatTy); }
    void do_dneg() { do_neg_common(Type::DoubleTy); }

    void do_neg_common(const Type* type) {
      Value* v1 = pop(type);
      Value* r = BinaryOperator::createNeg(v1, TMP, currentBB_);
      push(r);
    }

    void do_ishl() { do_shift_common(Instruction::Shl, Type::IntTy); }
    void do_lshl() { do_shift_common(Instruction::Shl, Type::LongTy); }
    void do_ishr() { do_shift_common(Instruction::Shr, Type::IntTy); }
    void do_lshr() { do_shift_common(Instruction::Shr, Type::LongTy); }

    void do_iushr() { do_shift_common(Instruction::Shr, Type::UIntTy); }
    void do_lushr() { do_shift_common(Instruction::Shr, Type::ULongTy); }

    void do_shift_common(Instruction::OtherOps op, const Type* type) {
      llvm::Constant* mask =
        type == Type::IntTy ? INT_SHIFT_MASK : LONG_SHIFT_MASK;
      Value* a = pop(Type::UByteTy);
      a = BinaryOperator::create(Instruction::And, a, mask, TMP, currentBB_);
      Value* v = pop(type);
      Value* r = new ShiftInst(op, v, a, TMP, currentBB_);
      push(r);
    }

    void do_iand() { do_binary_op_common(Instruction::And, Type::IntTy); }
    void do_land() { do_binary_op_common(Instruction::And, Type::LongTy); }
    void do_ior() { do_binary_op_common(Instruction::Or, Type::IntTy); }
    void do_lor() { do_binary_op_common(Instruction::Or, Type::LongTy); }
    void do_ixor() { do_binary_op_common(Instruction::Xor, Type::IntTy); }
    void do_lxor() { do_binary_op_common(Instruction::Xor, Type::LongTy); }

    void do_binary_op_common(Instruction::BinaryOps op, const Type* type) {
      Value* v2 = pop(type);
      Value* v1 = pop(type);
      Value* r = BinaryOperator::create(op, v1, v2, TMP, currentBB_);
      push(r);
    }

    void do_iinc(unsigned index, int amount) {
      Value* v = locals_.load(index, Type::IntTy, currentBB_);
      Value* a = ConstantSInt::get(Type::IntTy, amount);
      v = BinaryOperator::createAdd(v, a, TMP, currentBB_);
      locals_.store(index, v, currentBB_);
    }

    void do_i2l() { do_cast_common(Type::IntTy, Type::LongTy); }
    void do_i2f() { do_cast_common(Type::IntTy, Type::FloatTy); }
    void do_i2d() { do_cast_common(Type::IntTy, Type::DoubleTy); }
    void do_l2i() { do_cast_common(Type::LongTy, Type::IntTy); }
    void do_l2f() { do_cast_common(Type::LongTy, Type::FloatTy); }
    void do_l2d() { do_cast_common(Type::LongTy, Type::DoubleTy); }
    void do_f2i() { do_cast_common(Type::FloatTy, Type::IntTy); }
    void do_f2l() { do_cast_common(Type::FloatTy, Type::LongTy); }
    void do_f2d() { do_cast_common(Type::FloatTy, Type::DoubleTy); }
    void do_d2i() { do_cast_common(Type::DoubleTy, Type::IntTy); }
    void do_d2l() { do_cast_common(Type::DoubleTy, Type::LongTy); }
    void do_d2f() { do_cast_common(Type::DoubleTy, Type::FloatTy); }
    void do_i2b() { do_cast_common(Type::IntTy, Type::SByteTy); }
    void do_i2c() { do_cast_common(Type::IntTy, Type::UShortTy); }
    void do_i2s() { do_cast_common(Type::IntTy, Type::ShortTy); }

    void do_cast_common(const Type* from, const Type* to) {
      Value* v1 = pop(from);
      push(new CastInst(v1, to, TMP, currentBB_));
    }

    void do_lcmp() {
      Value* v2 = pop(Type::LongTy);
      Value* v1 = pop(Type::LongTy);
      Value* c = BinaryOperator::createSetGT(v1, v2, TMP, currentBB_);
      Value* r = new SelectInst(c, ONE, ZERO, TMP, currentBB_);
      c = BinaryOperator::createSetLT(v1, v2, TMP, currentBB_);
      r = new SelectInst(c, MINUS_ONE, r, TMP, currentBB_);
      push(r);
    }

    void do_fcmpl() { do_cmp_common(Type::FloatTy, false); }
    void do_dcmpl() { do_cmp_common(Type::DoubleTy, false); }
    void do_fcmpg() { do_cmp_common(Type::FloatTy, true); }
    void do_dcmpg() { do_cmp_common(Type::DoubleTy, true); }

    void do_cmp_common(const Type* type, bool pushOne) {
      Value* v2 = pop(type);
      Value* v1 = pop(type);
      Value* c = BinaryOperator::createSetGT(v1, v2, TMP, currentBB_);
      Value* r = new SelectInst(c, ONE, ZERO, TMP, currentBB_);
      c = BinaryOperator::createSetLT(v1, v2, TMP, currentBB_);
      r = new SelectInst(c, MINUS_ONE, r, TMP, currentBB_);
      c = new CallInst(module_->getOrInsertFunction
                       ("llvm.isunordered",
                        Type::BoolTy, v1->getType(), v2->getType(), 0),
                       v1, v2, TMP, currentBB_);
      r = new SelectInst(c, pushOne ? ONE : MINUS_ONE, r, TMP, currentBB_);
      push(r);
    }

    void do_ifeq(unsigned t, unsigned f) {
      do_iconst(0);
      do_if_common(Instruction::SetEQ, Type::IntTy, t, f);
    }
    void do_ifne(unsigned t, unsigned f) {
      do_iconst(0);
      do_if_common(Instruction::SetNE, Type::IntTy, t, f);
    }
    void do_iflt(unsigned t, unsigned f) {
      do_iconst(0);
      do_if_common(Instruction::SetLT, Type::IntTy, t, f);
    }
    void do_ifge(unsigned t, unsigned f) {
      do_iconst(0);
      do_if_common(Instruction::SetGE, Type::IntTy, t, f);
    }
    void do_ifgt(unsigned t, unsigned f) {
      do_iconst(0);
      do_if_common(Instruction::SetGT, Type::IntTy, t, f);
    }
    void do_ifle(unsigned t, unsigned f) {
      do_iconst(0);
      do_if_common(Instruction::SetLE, Type::IntTy, t, f);
    }
    void do_if_icmpeq(unsigned t, unsigned f) {
      do_if_common(Instruction::SetEQ, Type::IntTy, t, f);
    }
    void do_if_icmpne(unsigned t, unsigned f) {
      do_if_common(Instruction::SetNE, Type::IntTy, t, f);
    }
    void do_if_icmplt(unsigned t, unsigned f) {
      do_if_common(Instruction::SetLT, Type::IntTy, t, f);
    }
    void do_if_icmpge(unsigned t, unsigned f) {
      do_if_common(Instruction::SetGE, Type::IntTy, t, f);
    }
    void do_if_icmpgt(unsigned t, unsigned f) {
      do_if_common(Instruction::SetGT, Type::IntTy, t, f);
    }
    void do_if_icmple(unsigned t, unsigned f) {
      do_if_common(Instruction::SetLE, Type::IntTy, t, f);
    }
    void do_if_acmpeq(unsigned t, unsigned f) {
      do_if_common(Instruction::SetEQ, resolver_->getObjectBaseRefType(), t, f);
    }
    void do_if_acmpne(unsigned t, unsigned f) {
      do_if_common(Instruction::SetNE, resolver_->getObjectBaseRefType(), t, f);
    }
    void do_ifnull(unsigned t, unsigned f) {
      do_aconst_null();
      do_if_common(Instruction::SetEQ, resolver_->getObjectBaseRefType(), t, f);
    }
    void do_ifnonnull(unsigned t, unsigned f) {
      do_aconst_null();
      do_if_common(Instruction::SetNE, resolver_->getObjectBaseRefType(), t, f);
    }

    void do_if_common(Instruction::BinaryOps cc, const Type* type,
                      unsigned t, unsigned f) {
      Value* v2 = pop(type);
      Value* v1 = pop(type);
      Value* c = new SetCondInst(cc, v1, v2, TMP, currentBB_);
      new BranchInst(bbBuilder_->getBasicBlock(t),
                     bbBuilder_->getBasicBlock(f),
                     c, currentBB_);
    }

    void do_goto(unsigned target) {
      new BranchInst(bbBuilder_->getBasicBlock(target), currentBB_);
    }

    void do_ireturn() { do_return_common(Type::IntTy); }
    void do_lreturn() { do_return_common(Type::LongTy); }
    void do_freturn() { do_return_common(Type::FloatTy); }
    void do_dreturn() { do_return_common(Type::DoubleTy); }
    void do_areturn() { do_return_common(resolver_->getObjectBaseRefType()); }

    void do_return_common(const Type* type) {
      Value* r = pop(type);
      const Type* retTy = currentBB_->getParent()->getReturnType();
      new ReturnInst(new CastInst(r, retTy, TMP, currentBB_), currentBB_);
    }

    void do_return() {
      new ReturnInst(NULL, currentBB_);
    }

    void do_jsr(unsigned target, unsigned retAddress) {
      // FIXME: this is currently a noop.
      push(llvm::Constant::getNullValue(Type::IntTy));
    }

    void do_ret(unsigned index) {
      // FIXME: this is currently a noop.
    }

    void do_tableswitch(unsigned defTarget, const SwitchCases& sw) {
      do_switch_common(defTarget, sw);
    }

    void do_lookupswitch(unsigned defTarget, const SwitchCases& sw) {
      do_switch_common(defTarget, sw);
    }

    void do_switch_common(unsigned defTarget, const SwitchCases& sw) {
      Value* v = pop(Type::IntTy);
      SwitchInst* in =
        new SwitchInst(v, bbBuilder_->getBasicBlock(defTarget), sw.size(),
                       currentBB_);
      for (unsigned i = 0, e = sw.size(); i != e; ++i)
        in->addCase(ConstantSInt::get(Type::IntTy, sw[i].first),
                    bbBuilder_->getBasicBlock(sw[i].second));
    }

    void do_getstatic(unsigned index) {
      Value* v = new LoadInst(getStaticField(index), TMP, currentBB_);
      push(v);
    }

    void do_putstatic(unsigned index) {
      Value* ptr = getStaticField(index);
      const Type* fieldTy = cast<PointerType>(ptr->getType())->getElementType();
      Value* v = pop(fieldTy);
      new StoreInst(v, ptr, currentBB_);
    }

    void do_getfield(unsigned index) {
      ConstantFieldRef* fieldRef =
        class_->getClassFile()->getConstantFieldRef(index);
      const std::string& name = fieldRef->getNameAndType()->getName()->str();
      Value* p = pop(resolver_->getObjectBaseRefType());
      Value* v = new LoadInst(getField(index, p), name, currentBB_);
      push(v);
    }

    void do_putfield(unsigned index) {
      ConstantFieldRef* fieldRef =
        class_->getClassFile()->getConstantFieldRef(index);
      const Type* type =
        resolver_->getType(fieldRef->getNameAndType()->getDescriptor()->str());
      Value* v = pop(type);
      Value* p = pop(resolver_->getObjectBaseRefType());
      Value* fp = getField(index, p);
      const Type* ft = cast<PointerType>(fp->getType())->getElementType();
      v = new CastInst(v, ft, TMP, currentBB_);
      new StoreInst(v, getField(index, p), currentBB_);
    }

    void makeCall(Value* fun, const std::vector<Value*> params) {
      const PointerType* funPtrTy = cast<PointerType>(fun->getType());
      const FunctionType* funTy =
        cast<FunctionType>(funPtrTy->getElementType());

      if (funTy->getReturnType() == Type::VoidTy)
        new CallInst(fun, params, "", currentBB_);
      else {
        Value* r = new CallInst(fun, params, TMP, currentBB_);
        push(r);
      }
    }

    std::vector<Value*> getParams(const FunctionType* funTy) {
      unsigned numParams = funTy->getNumParams();
      std::vector<Value*> params(numParams);
      while (numParams--)
        params[numParams] = pop(funTy->getParamType(numParams));

      return params;
    }

    const VTableInfo* getVTableInfoGeneric(const Class* clazz) {
      assert(!clazz->isPrimitive() &&
             "Cannot get VTableInfo for primitive class!");
      if (clazz->isArray()) {
        const Class* componentClass = clazz->getComponentClass();
        if (componentClass->isPrimitive())
          return &getPrimitiveArrayVTableInfo(componentClass->getType());
        else
          return &getObjectArrayVTableInfo();
      }
      else
        return &getVTableInfo(clazz);
    }

    void do_invokevirtual(unsigned index) {
      ConstantMethodRef* methodRef =
        class_->getClassFile()->getConstantMethodRef(index);
      ConstantNameAndType* nameAndType = methodRef->getNameAndType();

      const std::string& className = methodRef->getClass()->getName()->str();

      const Class* clazz = class_->getClass(methodRef->getClassIndex());
      const VTableInfo* vi = getVTableInfoGeneric(clazz);

      const std::string& methodDescr =
        nameAndType->getName()->str() +
        nameAndType->getDescriptor()->str();

      const FunctionType* funTy = cast<FunctionType>(
        resolver_->getType(nameAndType->getDescriptor()->str(), true));

      std::vector<Value*> params(getParams(funTy));

      Value* objRef = params.front();
      objRef = new CastInst(objRef, clazz->getType(), "this", currentBB_);
      Value* objBase =
        new CastInst(objRef, resolver_->getObjectBaseRefType(), TMP, currentBB_);
      Value* vtable = new CallInst(getVtable_, objBase, TMP, currentBB_);
      vtable = new CastInst(vtable, vi->vtable->getType(),
                            className + ".vtable", currentBB_);
      std::vector<Value*> indices(1, ConstantUInt::get(Type::UIntTy, 0));
      assert(vi->m2iMap.find(methodDescr) != vi->m2iMap.end() &&
             "could not find slot for virtual function!");
      unsigned vSlot = vi->m2iMap.find(methodDescr)->second;
      indices.push_back(ConstantUInt::get(Type::UIntTy, vSlot));
      Value* vfunPtr =
        new GetElementPtrInst(vtable, indices, TMP, currentBB_);
      Value* vfun = new LoadInst(vfunPtr, className + '/' + methodDescr,
                                 currentBB_);

      makeCall(vfun, params);
    }

    void do_invokespecial(unsigned index) {
      ConstantMethodRef* methodRef =
        class_->getClassFile()->getConstantMethodRef(index);
      ConstantNameAndType* nameAndType = methodRef->getNameAndType();

      const std::string& className = methodRef->getClass()->getName()->str();
      const std::string& methodName = nameAndType->getName()->str();
      const std::string& methodDescr =
        methodName + nameAndType->getDescriptor()->str();
      std::string funcName = className + '/' + methodDescr;
      const Class* clazz = class_->getClass(methodRef->getClassIndex());

      const FunctionType* funcTy = cast<FunctionType>(
        resolver_->getType(nameAndType->getDescriptor()->str(), true));
      Function* function = module_->getOrInsertFunction(funcName, funcTy);
      scheduleFunction(function);
      makeCall(function, getParams(funcTy));
    }

    void do_invokestatic(unsigned index) {
      ConstantMethodRef* methodRef =
        class_->getClassFile()->getConstantMethodRef(index);
      const Class* clazz = class_->getClass(methodRef->getClassIndex());
      emitStaticInitializers(clazz->getClassFile());
      Method* method = getMethod(methodRef);
      Function* function = getFunction(method);
      // Intercept java/lang/System/loadLibrary() calls and add
      // library deps to the module
      if (function->getName().find(
            "java/lang/System/loadLibrary(Ljava/lang/String;)V") == 0) {
        // FIXME: we should get the string and add this library to the
        // module.

        // If this function is not defined, define it now.
        if (function->empty())
          new ReturnInst(NULL, new BasicBlock("entry", function));
      }
      else
        scheduleFunction(function);
      makeCall(function, getParams(function->getFunctionType()));
    }

    void do_invokeinterface(unsigned index) {
      ConstantInterfaceMethodRef* methodRef =
        class_->getClassFile()->getConstantInterfaceMethodRef(index);
      ConstantNameAndType* nameAndType = methodRef->getNameAndType();

      const std::string& className = methodRef->getClass()->getName()->str();

      const Class* clazz = class_->getClass(methodRef->getClassIndex());
      const VTableInfo* vi = getVTableInfoGeneric(clazz);

      const std::string& methodDescr =
        nameAndType->getName()->str() +
        nameAndType->getDescriptor()->str();

      const FunctionType* funTy = cast<FunctionType>(
        resolver_->getType(nameAndType->getDescriptor()->str(), true));

      std::vector<Value*> params(getParams(funTy));

      Value* objRef = params.front();
      objRef = new CastInst(objRef, clazz->getType(), "this", currentBB_);
      Value* objBase =
        new CastInst(objRef, resolver_->getObjectBaseRefType(), TMP, currentBB_);
      Value* vtable = new CallInst(getVtable_, objBase, TMP, currentBB_);
      vtable = new CastInst(vtable, PointerType::get(VTableInfo::VTableTy),
                            TMP, currentBB_);
      // get the interfaces array of vtables
      std::vector<Value*> indices(2, ConstantUInt::get(Type::UIntTy, 0));
      indices.push_back(ConstantUInt::get(Type::UIntTy, 3));
      Value* interfaceVTables =
        new GetElementPtrInst(vtable, indices, TMP, currentBB_);
      interfaceVTables = new LoadInst(interfaceVTables, TMP, currentBB_);
      // Get the actual interface vtable.
      indices.clear();
      indices.push_back(ConstantUInt::get(Type::UIntTy,
                                          clazz->getInterfaceIndex()));
      Value* interfaceVTable =
        new GetElementPtrInst(interfaceVTables, indices, TMP, currentBB_);
      interfaceVTable =
        new LoadInst(interfaceVTable, className + ".vtable", currentBB_);
      interfaceVTable =
        new CastInst(interfaceVTable, vi->vtable->getType(), TMP, currentBB_);
      // Get the function pointer.
      assert(vi->m2iMap.find(methodDescr) != vi->m2iMap.end() &&
             "could not find slot for virtual function!");
      unsigned vSlot = vi->m2iMap.find(methodDescr)->second;
      indices.resize(2);
      indices[0] = ConstantUInt::get(Type::UIntTy, 0);
      indices[1] = ConstantUInt::get(Type::UIntTy, vSlot);
      Value* vfunPtr =
        new GetElementPtrInst(interfaceVTable, indices, TMP, currentBB_);
      Value* vfun = new LoadInst(vfunPtr, className + '/' + methodDescr,
                                 currentBB_);

      makeCall(vfun, params);
    }

    template<typename InsertionPointTy>
    Value* allocateObject(const Class& clazz,
                          const VTableInfo& vi,
                          InsertionPointTy* ip) {
      static std::vector<Value*> params(4);

      Value* objRef = new MallocInst(clazz.getStructType(), NULL, TMP, ip);
      params[0] =
        new CastInst(objRef, PointerType::get(Type::SByteTy), TMP, ip); // dest
      params[1] = ConstantUInt::get(Type::UByteTy, 0); // value
      params[2] = ConstantExpr::getSizeOf(clazz.getStructType()); // size
      params[3] = ConstantUInt::get(Type::UIntTy, 0); // alignment
      new CallInst(memset_, params, "", ip);

      // Install the vtable pointer.
      Value* objBase = new CastInst(objRef, resolver_->getObjectBaseRefType(), TMP, ip);
      Value* vtable = new CastInst(vi.vtable, VTableBaseRefTy, TMP, ip);
      new CallInst(setVtable_, objBase, vtable, "", ip);

      return objRef;
    }

    void do_new(unsigned index) {
      const Class* clazz = class_->getClass(index);
      emitStaticInitializers(clazz->getClassFile());
      const VTableInfo& vi = getVTableInfo(clazz);

      push(allocateObject(*clazz, vi, currentBB_));
    }

    template <typename InsertionPointTy>
    Value* getArrayLengthPtr(Value* arrayRef, InsertionPointTy* ip) const {
      std::vector<Value*> indices;
      indices.reserve(2);
      indices.push_back(ConstantUInt::get(Type::UIntTy, 0));
      indices.push_back(ConstantUInt::get(Type::UIntTy, 1));

      return new GetElementPtrInst(arrayRef, indices, TMP, ip);
    }

    template <typename InsertionPointTy>
    Value* getArrayObjectBasePtr(Value* arrayRef, InsertionPointTy* ip) const {
      std::vector<Value*> indices;
      indices.reserve(2);
      indices.push_back(ConstantUInt::get(Type::UIntTy, 0));
      indices.push_back(ConstantUInt::get(Type::UIntTy, 0));

      return new GetElementPtrInst(arrayRef, indices, TMP, ip);
    }

    template<typename InsertionPointTy>
    Value* allocateArray(const Class* clazz,
                         const VTableInfo* vi,
                         Value* count,
                         InsertionPointTy* ip) {
      static std::vector<Value*> params(4);

      assert(clazz->isArray() && "Not an array class!");
      const Class* componentClass = clazz->getComponentClass();
      const Type* elementTy = componentClass->getType();

      // The size of the element.
      llvm::Constant* elementSize =
        ConstantExpr::getCast(ConstantExpr::getSizeOf(elementTy), Type::UIntTy);

      // The size of the array part of the struct.
      Value* size = BinaryOperator::create(
        Instruction::Mul, count, elementSize, TMP, ip);
      // The size of the rest of the array object.
      llvm::Constant* arrayObjectSize =
        ConstantExpr::getCast(ConstantExpr::getSizeOf(clazz->getStructType()),
                              Type::UIntTy);

      // Add the array part plus the object part together.
      size = BinaryOperator::create(
        Instruction::Add, size, arrayObjectSize, TMP, ip);
      // Allocate memory for the object.
      Value* objRef = new MallocInst(Type::SByteTy, size, TMP, ip);
      params[0] = objRef; // dest
      params[1] = ConstantUInt::get(Type::UByteTy, 0); // value
      params[2] = new CastInst(size, Type::ULongTy, TMP, ip); // size
      params[3] = ConstantUInt::get(Type::UIntTy, 0); // alignment
      new CallInst(memset_, params, "", ip);

      // Cast back to array type.
      objRef = new CastInst(objRef, clazz->getType(), TMP, ip);

      // Store the size.
      Value* lengthPtr = getArrayLengthPtr(objRef, ip);
      new StoreInst(count, lengthPtr, ip);

      // Install the vtable pointer.
      Value* objBase = new CastInst(objRef, resolver_->getObjectBaseRefType(), TMP, ip);
      Value* vtable = new CastInst(vi->vtable, VTableBaseRefTy, TMP, ip);
      new CallInst(setVtable_, objBase, vtable, "", ip);

      return objRef;
    }

    void do_newarray(JType type) {
      Value* count = pop(Type::UIntTy);

      const Class* clazz = resolver_->getClass(type);
      const Class* arrayClass = resolver_->getArrayClass(clazz);
      const VTableInfo* vi = getVTableInfoGeneric(arrayClass);

      push(allocateArray(arrayClass, vi, count, currentBB_));
    }

    void do_anewarray(unsigned index) {
      Value* count = pop(Type::UIntTy);

      const Class* clazz = class_->getClass(index);
      const Class* arrayClass = resolver_->getArrayClass(clazz);
      const VTableInfo* vi = getVTableInfoGeneric(arrayClass);

      push(allocateArray(arrayClass, vi, count, currentBB_));
    }

    void do_arraylength() {
      const Class* clazz = resolver_->getClass("[Ljava/lang/Object;");
      Value* arrayRef = pop(clazz->getType());
      Value* lengthPtr = getArrayLengthPtr(arrayRef, currentBB_);
      Value* length = new LoadInst(lengthPtr, TMP, currentBB_);
      push(length);
    }

    void do_athrow() {
      Value* objRef = pop(resolver_->getObjectBaseRefType());
      new CallInst(throw_, objRef, "", currentBB_);
      new UnreachableInst(currentBB_);
    }

    void do_checkcast(unsigned index) {
      const Class* clazz = class_->getClass(index);
      const VTableInfo* vi = getVTableInfoGeneric(clazz);

      Value* objRef = pop(resolver_->getObjectBaseRefType());
      Value* vtable = new CastInst(vi->vtable,
                                   VTableBaseRefTy,
                                   TMP, currentBB_);
      Value* r = new CallInst(isInstanceOf_, objRef, vtable, TMP, currentBB_);

      Value* b = new SetCondInst(Instruction::SetEQ,
                                 r, ConstantSInt::get(Type::IntTy, 1),
                                 TMP, currentBB_);
      // FIXME: if b is false we must throw a ClassCast exception
      push(objRef);
    }

    void do_instanceof(unsigned index) {
      const Class* clazz = class_->getClass(index);
      const VTableInfo* vi = getVTableInfoGeneric(clazz);

      Value* objRef = pop(resolver_->getObjectBaseRefType());
      Value* vtable = new CastInst(vi->vtable, VTableBaseRefTy,
                                   TMP, currentBB_);
      Value* r = new CallInst(isInstanceOf_, objRef, vtable, TMP, currentBB_);
      push(r);
    }

    void do_monitorenter() {
      // FIXME: This is currently a noop.
      pop(resolver_->getObjectBaseRefType());
    }

    void do_monitorexit() {
      // FIXME: This is currently a noop.
      pop(resolver_->getObjectBaseRefType());
    }

    void do_multianewarray(unsigned index, unsigned dims) {
      assert(0 && "not implemented");
    }
  };

  StructType* Compiler::VTableInfo::VTableTy;
  StructType* Compiler::VTableInfo::TypeInfoTy;

} } } // namespace llvm::Java::

std::auto_ptr<Module> llvm::Java::compile(const std::string& className)
{
  DEBUG(std::cerr << "Compiling class: " << className << '\n');

  std::auto_ptr<Module> m(new Module(className));
  // Require the Java runtime.
  m->addLibrary("jrt");

  Compiler c(m.get());
  Function* main = c.compileMethod(className + "/main([Ljava/lang/String;)V");
  Function* javaMain = m->getOrInsertFunction
    ("llvm_java_main", Type::VoidTy,
     Type::IntTy, PointerType::get(PointerType::get(Type::SByteTy)), NULL);

  BasicBlock* bb = new BasicBlock("entry", javaMain);
  const FunctionType* mainTy = main->getFunctionType();
  new CallInst(main,
               // FIXME: Forward correct params from llvm_java_main
               llvm::Constant::getNullValue(mainTy->getParamType(0)),
               "",
               bb);
  new ReturnInst(NULL, bb);

  return m;
}

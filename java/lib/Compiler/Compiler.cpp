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

#include <llvm/Java/Bytecode.h>
#include <llvm/Java/BytecodeParser.h>
#include <llvm/Java/ClassFile.h>
#include <llvm/Java/Compiler.h>
#include <llvm/Constants.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Instructions.h>
#include <llvm/Value.h>
#include <llvm/Type.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/SetVector.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/Support/Debug.h>
#include <iostream>
#include <stack>
#include <vector>

#define LLVM_JAVA_OBJECT_BASE "struct.llvm_java_object_base"
#define LLVM_JAVA_OBJECT_HEADER "struct.llvm_java_object_header"
#define LLVM_JAVA_OBJECT_TYPEINFO "struct.llvm_java_object_typeinfo"
#define LLVM_JAVA_OBJECT_VTABLE "struct.llvm_java_object_vtable"

#define LLVM_JAVA_STATIC_INIT "llvm_java_static_init"

#define LLVM_JAVA_ISINSTANCEOF  "llvm_java_IsInstanceOf"
#define LLVM_JAVA_GETOBJECTCLASS "llvm_java_GetObjectClass"
#define LLVM_JAVA_THROW "llvm_java_Throw"

using namespace llvm;
using namespace llvm::Java;

namespace llvm { namespace Java { namespace {

  const std::string TMP("tmp");

  typedef std::stack<Value*, std::vector<Value*> > OperandStack;
  typedef std::vector<Value*> Locals;

  inline bool isTwoSlotType(const Type* t) {
    return t == Type::LongTy | t == Type::DoubleTy;
  }

  inline bool isTwoSlotValue(const Value* v) {
    return isTwoSlotType(v->getType());
  }

  inline bool isOneSlotType(const Type* t) {
    return !isTwoSlotType(t);
  }

  inline bool isOneSlotValue(const Value* v) {
    return isOneSlotType(v->getType());
  }

  class Bytecode2BasicBlockMapper
    : public BytecodeParser<Bytecode2BasicBlockMapper> {
    Function* function_;
    typedef std::vector<BasicBlock*> BC2BBMap;
    BC2BBMap bc2bbMap_;
    typedef std::map<BasicBlock*, BasicBlock*> FallThroughMap;
    FallThroughMap ftMap_;

    void createBasicBlockAt(unsigned bcI) {
      if (!bc2bbMap_[bcI])
        bc2bbMap_[bcI] = new BasicBlock("bc" + utostr(bcI), function_);
    }

  public:
    Bytecode2BasicBlockMapper(Function* f, CodeAttribute* c)
      : function_(f), bc2bbMap_(c->getCodeSize()) {
      BasicBlock* bb = new BasicBlock("entry", function_);

      parse(c->getCode(), c->getCodeSize());

      for (unsigned i = 0, e = bc2bbMap_.size(); i != e; ++i)
        if (BasicBlock* next = bc2bbMap_[i]) {
          ftMap_.insert(std::make_pair(bb, next));
          bb = next;
        }
        else
          bc2bbMap_[i] = bb;

      assert(function_->getEntryBlock().getName() == "entry");
    }

    BasicBlock* getBBAt(unsigned bcI) {
      assert(bc2bbMap_.size() > bcI && "Invalid bytecode index!");
      return bc2bbMap_[bcI];
    }

    BasicBlock* getFallThroughBranch(BasicBlock* bb) {
      assert(ftMap_.find(bb) != ftMap_.end() &&
             "Basic block is not in this mapper!");
      return ftMap_.find(bb)->second;
    }

    void do_ifeq(unsigned t, unsigned f) {
      createBasicBlockAt(t);
      createBasicBlockAt(f);
    }

    void do_ifne(unsigned t, unsigned f) {
      createBasicBlockAt(t);
      createBasicBlockAt(f);
    }

    void do_iflt(unsigned t, unsigned f) {
      createBasicBlockAt(t);
      createBasicBlockAt(f);
    }

    void do_ifge(unsigned t, unsigned f) {
      createBasicBlockAt(t);
      createBasicBlockAt(f);
    }

    void do_ifgt(unsigned t, unsigned f) {
      createBasicBlockAt(t);
      createBasicBlockAt(f);
    }

    void do_ifle(unsigned t, unsigned f) {
      createBasicBlockAt(t);
      createBasicBlockAt(f);
    }

    void do_if_icmpeq(unsigned t, unsigned f) {
      createBasicBlockAt(t);
      createBasicBlockAt(f);
    }

    void do_if_icmpne(unsigned t, unsigned f) {
      createBasicBlockAt(t);
      createBasicBlockAt(f);
    }

    void do_if_icmplt(unsigned t, unsigned f) {
      createBasicBlockAt(t);
      createBasicBlockAt(f);
    }

    void do_if_icmpgt(unsigned t, unsigned f) {
      createBasicBlockAt(t);
      createBasicBlockAt(f);
    }

    void do_if_icmpge(unsigned t, unsigned f) {
      createBasicBlockAt(t);
      createBasicBlockAt(f);
    }

    void do_if_icmple(unsigned t, unsigned f) {
      createBasicBlockAt(t);
      createBasicBlockAt(f);
    }

    void do_switch(unsigned defTarget, const SwitchCases& sw) {
      for (unsigned i = 0, e = sw.size(); i != e; ++i) {
        unsigned target = sw[i].second;
        createBasicBlockAt(target);
      }
      createBasicBlockAt(defTarget);
    }

    void do_ifnull(unsigned t, unsigned f) {
      createBasicBlockAt(t);
      createBasicBlockAt(f);
    }

    void do_ifnotnull(unsigned t, unsigned f) {
      createBasicBlockAt(t);
      createBasicBlockAt(f);
    }
  };

  class Compiler :
    public BytecodeParser<Compiler> {
    Module& module_;
    ClassFile* cf_;
    OperandStack opStack_;
    Locals locals_;
    std::auto_ptr<Bytecode2BasicBlockMapper> mapper_;
    BasicBlock* prologue_;
    BasicBlock* current_;

    typedef SetVector<Function*> FunctionSet;
    FunctionSet toCompileFunctions_;

    struct ClassInfo {
      ClassInfo() : type(NULL), interfaceIdx(0) { }
      Type* type;
      unsigned interfaceIdx;
      typedef std::map<std::string, unsigned> Field2IndexMap;
      Field2IndexMap f2iMap;

      static unsigned InterfaceCount;
      static Type* ObjectBaseTy;
    };
    typedef std::map<ClassFile*, ClassInfo> Class2ClassInfoMap;
    Class2ClassInfoMap c2ciMap_;

    struct VTableInfo {
      VTableInfo() : vtable(NULL) { }
      GlobalVariable* vtable;
      std::vector<llvm::Constant*> superVtables;
      typedef std::map<std::string, unsigned> Method2IndexMap;
      Method2IndexMap m2iMap;

      static StructType* VTableTy;
      static StructType* TypeInfoTy;
    };
    typedef std::map<ClassFile*, VTableInfo> Class2VTableInfoMap;
    Class2VTableInfoMap c2viMap_;

  public:
    Compiler(Module& m)
      : module_(m) {
    }

  private:
    llvm::Constant* getConstant(Constant* c) {
      if (dynamic_cast<ConstantString*>(c))
        // FIXME: should return a String object represeting this ConstantString
        return ConstantPointerNull::get(
          PointerType::get(
            getClassInfo(ClassFile::get("java/lang/String")).type));
      else if (ConstantInteger* i = dynamic_cast<ConstantInteger*>(c))
        return ConstantSInt::get(Type::IntTy, i->getValue());
      else if (ConstantFloat* f = dynamic_cast<ConstantFloat*>(c))
        return ConstantFP::get(Type::FloatTy, f->getValue());
      else if (ConstantLong* l = dynamic_cast<ConstantLong*>(c))
        return ConstantSInt::get(Type::LongTy, l->getValue());
      else if (ConstantDouble* d = dynamic_cast<ConstantDouble*>(c))
        return ConstantFP::get(Type::DoubleTy, d->getValue());
      else
        assert(0 && "Unknown llvm::Java::Constant!");
    }

    Type* getType(JType type) {
      switch (type) {
      case BOOLEAN: return Type::BoolTy;
      case CHAR: return Type::UShortTy;
      case FLOAT: return Type::FloatTy;
      case DOUBLE: return Type::DoubleTy;
      case BYTE: return Type::SByteTy;
      case SHORT: return Type::ShortTy;
      case INT: return Type::IntTy;
      case LONG: return Type::LongTy;
      default: assert(0 && "Invalid JType to Type conversion!");
      }

      return NULL;
    }

    /// Returns the type of the Java string descriptor. If the
    /// Type* self is not NULL then that type is used as the first
    /// type in function types
    Type* getType(ConstantUtf8* descr, Type* self = NULL) {
      unsigned i = 0;
      return getTypeHelper(descr->str(), i, self);
    }

    Type* getTypeHelper(const std::string& descr, unsigned& i, Type* self) {
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
      case 'L': {
        unsigned e = descr.find(';', i);
        std::string className = descr.substr(i, e - i);
        i = e + 1;
        return PointerType::get(getClassInfo(ClassFile::get(className)).type);
      }
      case '[':
        // FIXME: this should really be a new class
        // represeting the array of the following type
        return PointerType::get(
          ArrayType::get(getTypeHelper(descr, i, NULL), 0));
      case '(': {
        std::vector<const Type*> params;
        if (self)
          params.push_back(PointerType::get(self));
        while (descr[i] != ')')
          params.push_back(getTypeHelper(descr, i, NULL));
        return FunctionType::get(getTypeHelper(descr, ++i, NULL),params, false);
      }
        // FIXME: Throw something
      default:  return NULL;
      }
    }

    void initializeClassInfoMap() {
      DEBUG(std::cerr << "Building ClassInfo for: java/lang/Object\n");
      ClassFile* cf = ClassFile::get("java/lang/Object");
      ClassInfo& ci = c2ciMap_[cf];

      assert(!ci.type && ci.f2iMap.empty() &&
             "java/lang/Object ClassInfo should not be initialized!");
      ci.type = OpaqueType::get();

      std::vector<const Type*> elements;

      // because this is java/lang/Object, we add the opaque
      // llvm_java_object_base type first
      ClassInfo::ObjectBaseTy = OpaqueType::get();
      module_.addTypeName(LLVM_JAVA_OBJECT_BASE, ClassInfo::ObjectBaseTy);
      ci.f2iMap.insert(std::make_pair(LLVM_JAVA_OBJECT_BASE, elements.size()));
      elements.push_back(ClassInfo::ObjectBaseTy);

      const Fields& fields = cf->getFields();
      for (unsigned i = 0, e = fields.size(); i != e; ++i) {
        Field* field = fields[i];
        if (!field->isStatic()) {
          ci.f2iMap.insert(
            std::make_pair(field->getName()->str(), elements.size()));
          elements.push_back(getType(field->getDescriptor()));
        }
      }
      PATypeHolder holder = ci.type;
      cast<OpaqueType>(ci.type)->refineAbstractTypeTo(StructType::get(elements));
      ci.type = holder.get();
      DEBUG(std::cerr << "Adding java/lang/Object = "
            << *ci.type << " to type map\n");
      module_.addTypeName("java/lang/Object", ci.type);

      assert(ci.type && "ClassInfo not initialized properly!");
      emitStaticInitializers(cf);
      DEBUG(std::cerr << "Built ClassInfo for: java/lang/Object\n");
    }

    void initializeVTableInfoMap() {
      DEBUG(std::cerr << "Building VTableInfo for: java/lang/Object\n");
      ClassFile* cf = ClassFile::get("java/lang/Object");
      VTableInfo& vi = c2viMap_[cf];

      assert(!vi.vtable && vi.m2iMap.empty() &&
             "java/lang/Object VTableInfo should not be initialized!");

      Type* VTtype = OpaqueType::get();

      std::vector<const Type*> elements;
      std::vector<llvm::Constant*> init;

      // this is java/lang/Object so we must add a
      // llvm_java_object_typeinfo struct first

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

      // this is a static variable
      VTableInfo::TypeInfoTy = StructType::get(elements);
      module_.addTypeName(LLVM_JAVA_OBJECT_TYPEINFO, VTableInfo::TypeInfoTy);
      llvm::Constant* typeInfoInit =
        ConstantStruct::get(VTableInfo::TypeInfoTy, init);

      // now that we have both the type and initializer for the
      // llvm_java_object_typeinfo struct we can start adding the
      // function pointers
      elements.clear();
      init.clear();

      /// first add the typeinfo struct itself
      elements.push_back(typeInfoInit->getType());
      // add the typeinfo block for this class
      init.push_back(typeInfoInit);

      const Methods& methods = cf->getMethods();

      const ClassInfo& ci = getClassInfo(cf);

      // add member functions to the vtable
      for (unsigned i = 0, e = methods.size(); i != e; ++i) {
        Method* method = methods[i];
        // the contructor is the only non-static method that is not
        // dynamically dispatched so we skip it
        if (!method->isStatic() && method->getName()->str() != "<init>") {
          std::string methodDescr =
            method->getName()->str() +
            method->getDescriptor()->str();

          std::string funcName = "java/lang/Object/" + methodDescr;
          const FunctionType* funcTy = cast<FunctionType>(
            getType(method->getDescriptor(), ci.type));

          Function* vfun = module_.getOrInsertFunction(funcName, funcTy);
          toCompileFunctions_.insert(vfun);

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
      module_.addTypeName("java/lang/Object<vtable>", VTableInfo::VTableTy);

      vi.vtable = new GlobalVariable(VTableInfo::VTableTy,
                                     true, GlobalVariable::ExternalLinkage,
                                     ConstantStruct::get(init),
                                     "java/lang/Object<vtable>",
                                     &module_);
      DEBUG(std::cerr << "Built VTableInfo for: java/lang/Object\n");
    }

    void initializeTypeMaps() {
      initializeClassInfoMap();
      initializeVTableInfoMap();
    }

    const ClassInfo& getClassInfo(ClassFile* cf) {
      Class2ClassInfoMap::iterator it = c2ciMap_.lower_bound(cf);
      if (it != c2ciMap_.end() && it->first == cf)
        return it->second;

      const std::string& className = cf->getThisClass()->getName()->str();
      DEBUG(std::cerr << "Building ClassInfo for: " << className << '\n');
      ClassInfo& ci = c2ciMap_[cf];

      assert(!ci.type && ci.f2iMap.empty() &&
             "got already initialized ClassInfo!");

      // get the interface id
      if (cf->isInterface())
        ci.interfaceIdx = ClassInfo::InterfaceCount++;

      ci.type = OpaqueType::get();

      std::vector<const Type*> elements;
      ConstantClass* super = cf->getSuperClass();
      assert(super && "Class does not have superclass!");
      const ClassInfo& superCI =
        getClassInfo(ClassFile::get(super->getName()->str()));
      elements.push_back(superCI.type);

      const Fields& fields = cf->getFields();
      for (unsigned i = 0, e = fields.size(); i != e; ++i) {
        Field* field = fields[i];
        if (!field->isStatic()) {
          ci.f2iMap.insert(
            std::make_pair(field->getName()->str(), elements.size()));
          elements.push_back(getType(field->getDescriptor()));
        }
      }
      PATypeHolder holder = ci.type;
      cast<OpaqueType>(ci.type)->refineAbstractTypeTo(StructType::get(elements));
      ci.type = holder.get();

      assert(ci.type && "ClassInfo not initialized properly!");
      DEBUG(std::cerr << "Adding " << className << " = "
            << *ci.type << " to type map\n");
      module_.addTypeName(className, ci.type);
      emitStaticInitializers(cf);
      DEBUG(std::cerr << "Built ClassInfo for: " << className << '\n');
      return ci;
    }

    std::pair<unsigned,llvm::Constant*>
    buildSuperClassesVTables(ClassFile* cf, const VTableInfo& vi) const {
      ArrayType* vtablesArrayTy =
        ArrayType::get(PointerType::get(VTableInfo::VTableTy),
                       vi.superVtables.size());

      GlobalVariable* vtablesArray = new GlobalVariable(
        vtablesArrayTy,
        true,
        GlobalVariable::ExternalLinkage,
        ConstantArray::get(vtablesArrayTy, vi.superVtables),
        cf->getThisClass()->getName()->str() + "<superclassesvtables>",
        &module_);

      return std::make_pair(
        vi.superVtables.size(),
        ConstantExpr::getGetElementPtr(
          vtablesArray,
          std::vector<llvm::Constant*>(2, ConstantUInt::get(Type::UIntTy, 0))));
    }

    llvm::Constant* buildInterfaceVTable(ClassFile* cf, ClassFile* interface) {

      const VTableInfo& classVI = getVTableInfo(cf);
      const VTableInfo& interfaceVI = getVTableInfo(interface);
      const Methods& methods = interface->getMethods();

      // the size of the initializer will be 1 greater than the number
      // of methods for this interface (the first slot is the typeinfo
      // struct
      std::vector<llvm::Constant*> init(interfaceVI.m2iMap.size()+1, NULL);
      init[0] = llvm::Constant::getNullValue(VTableInfo::TypeInfoTy);

      for (VTableInfo::Method2IndexMap::const_iterator
             i = interfaceVI.m2iMap.begin(), e = interfaceVI.m2iMap.end();
           i != e; ++i) {
        std::vector<llvm::Constant*> indices;
        indices.reserve(2);
        indices.push_back(ConstantUInt::get(Type::UIntTy, 0));
        assert(classVI.m2iMap.find(i->first) != classVI.m2iMap.end() &&
               "Interface method not found in class definition!");
        unsigned classMethodIdx = classVI.m2iMap.find(i->first)->second;
        indices.push_back(ConstantUInt::get(Type::UIntTy, classMethodIdx));
        init[i->second] =
          ConstantExpr::getGetElementPtr(classVI.vtable, indices);
      }

      llvm::Constant* vtable = ConstantStruct::get(init);
      const std::string& globalName =
        cf->getThisClass()->getName()->str() + '+' +
        interface->getThisClass()->getName()->str() + "<vtable>";
      module_.addTypeName(globalName, vtable->getType());

      return new GlobalVariable(
        vtable->getType(),
        true,
        GlobalVariable::ExternalLinkage,
        vtable,
        globalName,
        &module_);
    }

    std::pair<int, llvm::Constant*>
    buildInterfacesVTables(ClassFile* cf, const VTableInfo& vi) {
      // if this is an interface then we are not implementing any
      // interfaces so the lastInterface field is our index and the
      // pointer to the array of interface vtables is an all-ones
      // value
      if (cf->isInterface())
        return std::make_pair(
          getClassInfo(cf).interfaceIdx,
          ConstantExpr::getCast(
            ConstantIntegral::getAllOnesValue(Type::LongTy),
            PointerType::get(PointerType::get(VTableInfo::VTableTy))));

      std::vector<llvm::Constant*> vtables;
      const Classes& interfaces = cf->getInterfaces();
      llvm::Constant* nullVTable =
        llvm::Constant::getNullValue(PointerType::get(VTableInfo::VTableTy));

      for (unsigned i = 0, e = interfaces.size(); i != e; ++i) {
        ClassFile* interface = ClassFile::get(interfaces[i]->getName()->str());
        assert(interface->isInterface() &&
               "Class in interfaces list is not an interface!");
        const ClassInfo& interfaceCI = getClassInfo(interface);
        if (interfaceCI.interfaceIdx >= vtables.size())
          vtables.resize(interfaceCI.interfaceIdx+1, nullVTable);
        vtables[interfaceCI.interfaceIdx] = buildInterfaceVTable(cf, interface);
      }

      ArrayType* interfacesArrayTy =
        ArrayType::get(PointerType::get(VTableInfo::VTableTy), vtables.size());

      const std::string& globalName =
        cf->getThisClass()->getName()->str() + "<interfacesvtables>";
      module_.addTypeName(globalName, interfacesArrayTy);

      GlobalVariable* interfacesArray = new GlobalVariable(
        interfacesArrayTy,
        true,
        GlobalVariable::ExternalLinkage,
        ConstantArray::get(interfacesArrayTy, vtables),
        globalName,
        &module_);

      return std::make_pair(
        int(vtables.size())-1,
        ConstantExpr::getGetElementPtr(
          interfacesArray,
          std::vector<llvm::Constant*>(2, ConstantUInt::get(Type::UIntTy, 0))));
    }

    llvm::Constant* buildClassTypeInfo(ClassFile* cf, const VTableInfo& vi) {
      std::vector<llvm::Constant*> typeInfoInit;

      unsigned depth;
      llvm::Constant* superClassesVTables;
      tie(depth, superClassesVTables) = buildSuperClassesVTables(cf, vi);

      // the depth (java/lang/Object has depth 0)
      typeInfoInit.push_back(ConstantSInt::get(Type::IntTy, depth));
      // the super classes' vtables
      typeInfoInit.push_back(superClassesVTables);

      int lastInterface;
      llvm::Constant* interfacesVTables;
      tie(lastInterface, interfacesVTables) = buildInterfacesVTables(cf, vi);

      // the last interface index or the interface index if this is an
      // interface
      typeInfoInit.push_back(ConstantSInt::get(Type::IntTy, lastInterface));
      // the interfaces' vtables
      typeInfoInit.push_back(interfacesVTables);

      return ConstantStruct::get(VTableInfo::TypeInfoTy, typeInfoInit);
    }

    const VTableInfo& getVTableInfo(ClassFile* cf) {
      Class2VTableInfoMap::iterator it = c2viMap_.lower_bound(cf);
      if (it != c2viMap_.end() && it->first == cf)
        return it->second;

      const std::string& className = cf->getThisClass()->getName()->str();
      DEBUG(std::cerr << "Building VTableInfo for: " << className << '\n');
      VTableInfo& vi = c2viMap_[cf];

      assert(!vi.vtable && vi.m2iMap.empty() &&
             "got already initialized VTableInfo!");

      ConstantClass* super = cf->getSuperClass();
      assert(super && "Class does not have superclass!");
      const VTableInfo& superVI =
        getVTableInfo(ClassFile::get(super->getName()->str()));

      // copy the super vtables array
      vi.superVtables.push_back(superVI.vtable);
      vi.superVtables.reserve(superVI.superVtables.size() + 1);
      std::copy(superVI.superVtables.begin(), superVI.superVtables.end(),
                std::back_inserter(vi.superVtables));

      // copy all the constants from the super class' vtable
      assert(superVI.vtable && "No vtable found for super class!");
      ConstantStruct* superInit =
        cast<ConstantStruct>(superVI.vtable->getInitializer());
      std::vector<llvm::Constant*> init(superInit->getNumOperands());
      // use a null typeinfo struct for now
      init[0] = llvm::Constant::getNullValue(VTableInfo::TypeInfoTy);
      // fill in the function pointers as they are in the super
      // class. overriden methods will be replaced later
      for (unsigned i = 0, e = superInit->getNumOperands(); i != e; ++i)
        init[i] = superInit->getOperand(i);
      vi.m2iMap = superVI.m2iMap;

      // add member functions to the vtable
      const Methods& methods = cf->getMethods();

      for (unsigned i = 0, e = methods.size(); i != e; ++i) {
        Method* method = methods[i];
        // the contructor is the only non-static method that is not
        // dynamically dispatched so we skip it
        if (!method->isStatic() && method->getName()->str() != "<init>") {
          const std::string& methodDescr =
            method->getName()->str() + method->getDescriptor()->str();

          std::string funcName = className + '/' + methodDescr;

          // if this is not an interface we will need to build up the
          const FunctionType* funcTy = cast<FunctionType>(
            getType(method->getDescriptor(), getClassInfo(cf).type));
          Function* vfun = module_.getOrInsertFunction(funcName, funcTy);
          toCompileFunctions_.insert(vfun);

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
      module_.addTypeName(globalName, vtable->getType());
      vi.vtable = new GlobalVariable(vtable->getType(),
                                     true,
                                     GlobalVariable::ExternalLinkage,
                                     vtable,
                                     globalName,
                                     &module_);

      // Now the vtable is complete, install the new typeinfo block
      // for this class: we install it last because we need the vtable
      // to exist in order to build it
      init[0] = buildClassTypeInfo(cf, vi);
      vi.vtable->setInitializer(ConstantStruct::get(init));

      DEBUG(std::cerr << "Built VTableInfo for: " << className << '\n');
      return vi;
    }

    Value* getOrCreateLocal(unsigned index, Type* type) {
      if (!locals_[index] ||
          cast<PointerType>(locals_[index]->getType())->getElementType() != type) {
        locals_[index] =
          new AllocaInst(type, NULL, "local" + utostr(index), prologue_);
      }

      return locals_[index];
    }

    GlobalVariable* getStaticField(unsigned index) {
      ConstantFieldRef* fieldRef = cf_->getConstantFieldRef(index);
      ConstantNameAndType* nameAndType = fieldRef->getNameAndType();

      // get ClassInfo for class owning the field - this will force
      // the globals to be initialized
      getClassInfo(ClassFile::get(fieldRef->getClass()->getName()->str()));

      std::string globalName =
        fieldRef->getClass()->getName()->str() + '/' +
        nameAndType->getName()->str();

      DEBUG(std::cerr << "Looking up global: " << globalName << '\n');
      GlobalVariable* global = module_.getGlobalVariable
        (globalName, getType(nameAndType->getDescriptor()));
      assert(global && "Got NULL global variable!");

      return global;
    }

    Value* getField(unsigned index, Value* ptr) {
      ConstantFieldRef* fieldRef = cf_->getConstantFieldRef(index);
      ConstantNameAndType* nameAndType = fieldRef->getNameAndType();
      ClassFile* cf = ClassFile::get(fieldRef->getClass()->getName()->str());
      return getField(cf, nameAndType->getName()->str(), ptr);
    }

    Value* getField(ClassFile* cf, const std::string& fieldName, Value* ptr) {
      // Cast ptr to correct type
      ptr = new CastInst(ptr, PointerType::get(getClassInfo(cf).type),
                         TMP, current_);

      // deref pointer
      std::vector<Value*> indices(1, ConstantUInt::get(Type::UIntTy, 0));
      while (true) {
        const ClassInfo& info = getClassInfo(cf);
        ClassInfo::Field2IndexMap::const_iterator it =
          info.f2iMap.find(fieldName);
        if (it == info.f2iMap.end()) {
          cf = ClassFile::get(cf->getSuperClass()->getName()->str());
          indices.push_back(ConstantUInt::get(Type::UIntTy, 0));
        }
        else {
          indices.push_back(ConstantUInt::get(Type::UIntTy, it->second));
          break;
        }
      }

      return new GetElementPtrInst(ptr, indices, TMP, current_);
    }

    Function* compileMethodOnly(const std::string& classMethodDesc) {
      Method* method = getMethod(classMethodDesc);
      cf_ = method->getParent();

      Function* function = getFunction(method);

      if (method->isNative()) {
        DEBUG(std::cerr << "Ignoring native method: ";
              std::cerr << classMethodDesc << '\n');
        return function;
      }
      else if (method->isAbstract()) {
        DEBUG(std::cerr << "Ignoring abstract method: ";
              std::cerr << classMethodDesc << '\n');
        return function;
      }

      DEBUG(std::cerr << "Compiling method: " << classMethodDesc << '\n');

      Java::CodeAttribute* codeAttr = method->getCodeAttribute();

      while (!opStack_.empty())
        opStack_.pop();

      locals_.clear();
      locals_.assign(codeAttr->getMaxLocals(), NULL);

      mapper_.reset(new Bytecode2BasicBlockMapper(function, codeAttr));

      prologue_ = new BasicBlock("prologue");
      unsigned index = 0;
      for (Function::aiterator
             a = function->abegin(), ae = function->aend(); a != ae; ++a) {
        // create a new local
        locals_[index] = new AllocaInst(
          a->getType(), NULL, "arg" + utostr(index), prologue_);
        // initialize the local with the contents of this argument
        new StoreInst(a, locals_[index], prologue_);
        index += isTwoSlotType(a->getType()) ? 2 : 1;
      }

      // make the prologue the entry block of the function with a
      // fallthrough branch to the original entry block
      function->getBasicBlockList().push_front(prologue_);
      current_ = prologue_->getNext();
      new BranchInst(current_, prologue_);

      parse(codeAttr->getCode(), codeAttr->getCodeSize());

      // function->dump();

      return function;
    }

    void emitStaticInitializers(const ClassFile* classfile) {
      const Method* method = classfile->getMethod("<clinit>()V");
      if (!method)
        return;

      std::string name = classfile->getThisClass()->getName()->str();
      name += '/';
      name += method->getName()->str();
      name += method->getDescriptor()->str();

      Function* hook = module_.getOrInsertFunction(LLVM_JAVA_STATIC_INIT,
                                                   Type::VoidTy, 0);
      Function* init = module_.getOrInsertFunction(name, Type::VoidTy, 0);

      // if this is the first time we scheduled this function
      // for compilation insert a call to it right before the
      // terminator of the only basic block in
      // llvm_java_static_init
      if (toCompileFunctions_.insert(init)) {
        assert(hook->front().getTerminator() &&
               LLVM_JAVA_STATIC_INIT " should have a terminator!");
        new CallInst(init, "", hook->front().getTerminator());
        // we also create the global variables of this class
        const Fields& fields = classfile->getFields();
        for (unsigned i = 0, e = fields.size(); i != e; ++i) {
          Field* field = fields[i];
          if (field->isStatic()) {
            llvm::Constant* init = NULL;
            if (ConstantValueAttribute* cv = field->getConstantValueAttribute())
              init = getConstant(cv->getValue());

            std::string globalName =
              classfile->getThisClass()->getName()->str() + '/' +
              field->getName()->str();
            DEBUG(std::cerr << "Adding global: " << globalName << '\n');
            new GlobalVariable(getType(field->getDescriptor()),
                               field->isFinal(),
                               (field->isPrivate() & bool(init) ?
                                GlobalVariable::InternalLinkage :
                                GlobalVariable::ExternalLinkage),
                               init,
                               globalName,
                               &module_);
          }
        }
      }
    }

    Function* getFunction(Method* method) {
      ClassFile* clazz = method->getParent();

      FunctionType* funcTy = cast<FunctionType>(
        getType(method->getDescriptor(),
                method->isStatic() ? NULL : getClassInfo(clazz).type));
      std::string funcName =
        clazz->getThisClass()->getName()->str() + '/' +
        method->getName()->str() + method->getDescriptor()->str();

      Function* function = module_.getOrInsertFunction(funcName, funcTy);
      function->setLinkage(method->isPrivate() ?
                           Function::InternalLinkage :
                           Function::ExternalLinkage);
      return function;
    }

    Method* getMethod(const std::string& classMethodDesc) {
      unsigned slash = classMethodDesc.rfind('/', classMethodDesc.find('('));
      std::string className = classMethodDesc.substr(0, slash);
      std::string methodNameAndDescr = classMethodDesc.substr(slash+1);

      ClassFile* classfile = ClassFile::get(className);
      emitStaticInitializers(classfile);
      Method* method = classfile->getMethod(methodNameAndDescr);

      if (!method)
        throw InvocationTargetException("Method " + methodNameAndDescr +
                                        " not found in class " + className);

      return method;
    }

  public:
    Function* compileMethod(const std::string& classMethodDesc) {
      // initialize the static initializer function
      Function* staticInit =
        module_.getOrInsertFunction(LLVM_JAVA_STATIC_INIT, Type::VoidTy, 0);
      BasicBlock* staticInitBB = new BasicBlock("entry", staticInit);
      new ReturnInst(NULL, staticInitBB);

      // initialize type maps and globals (vtables)
      initializeTypeMaps();

      // create the method requested
      Function* function = getFunction(getMethod(classMethodDesc));
      toCompileFunctions_.insert(function);
      // compile the transitive closure of methods called by this method
      for (unsigned i = 0; i != toCompileFunctions_.size(); ++i) {
        Function* f = toCompileFunctions_[i];
        compileMethodOnly(f->getName());
      }

      return function;
    }

    void pre_inst(unsigned bcI) {
      BasicBlock* previous = current_;
      current_ = mapper_->getBBAt(bcI);
      if (previous != current_ && !previous->getTerminator())
        new BranchInst(mapper_->getFallThroughBranch(previous), previous);
    }

    void do_aconst_null() {
      ClassFile* root = ClassFile::get("java/lang/Object");
      opStack_.push(llvm::Constant::getNullValue(
                      PointerType::get(getClassInfo(root).type)));
    }

    void do_iconst(int value) {
      opStack_.push(ConstantSInt::get(Type::IntTy, value));
    }

    void do_lconst(long long value) {
      opStack_.push(ConstantSInt::get(Type::LongTy, value));
    }

    void do_fconst(float value) {
      opStack_.push(ConstantFP::get(Type::FloatTy, value));
    }

    void do_dconst(double value) {
      opStack_.push(ConstantFP::get(Type::DoubleTy, value));
    }

    void do_ldc(unsigned index) {
      Constant* c = cf_->getConstant(index);
      assert(getConstant(c) && "Java constant not handled!");
      opStack_.push(getConstant(c));
    }

    void do_ldc2(unsigned index) {
      do_ldc(index);
    }

    void do_iload(unsigned index) { do_load_common(Type::IntTy, index); }
    void do_lload(unsigned index) { do_load_common(Type::LongTy, index); }
    void do_fload(unsigned index) { do_load_common(Type::FloatTy, index); }
    void do_dload(unsigned index) { do_load_common(Type::DoubleTy, index); }
    void do_aload(unsigned index) {
      ClassFile* root = ClassFile::get("java/lang/Object");
      do_load_common(PointerType::get(getClassInfo(root).type), index);
    }

    void do_load_common(Type* type, unsigned index) {
      opStack_.push(new LoadInst(getOrCreateLocal(index, type), TMP, current_));
    }

    void do_iaload() { do_aload_common(Type::IntTy); }
    void do_laload() { do_aload_common(Type::LongTy); }
    void do_faload() { do_aload_common(Type::FloatTy); }
    void do_daload() { do_aload_common(Type::DoubleTy); }
    void do_aaload() {
      ClassFile* root = ClassFile::get("java/lang/Object");
      do_aload_common(PointerType::get(getClassInfo(root).type));
    }
    void do_baload() { do_aload_common(Type::SByteTy); }
    void do_caload() { do_aload_common(Type::UShortTy); }
    void do_saload() { do_aload_common(Type::ShortTy); }

    void do_aload_common(Type* type) {
      assert(0 && "not implemented");
    }

    void do_istore(unsigned index) { do_store_common(Type::IntTy, index); }
    void do_lstore(unsigned index) { do_store_common(Type::LongTy, index); }
    void do_fstore(unsigned index) { do_store_common(Type::FloatTy, index); }
    void do_dstore(unsigned index) { do_store_common(Type::DoubleTy, index); }
    void do_astore(unsigned index) {
      ClassFile* root = ClassFile::get("java/lang/Object");
      do_store_common(PointerType::get(getClassInfo(root).type), index);
    }

    void do_store_common(Type* type, unsigned index) {
      Value* val = opStack_.top(); opStack_.pop();
      const Type* valTy = val->getType();
      Value* ptr = getOrCreateLocal(index, type);
      if (!valTy->isPrimitiveType() &&
          valTy != cast<PointerType>(ptr->getType())->getElementType())
        ptr = new CastInst(ptr, PointerType::get(valTy), TMP, current_);
      opStack_.push(new StoreInst(val, ptr, current_));
    }

    void do_iastore() { do_astore_common(Type::IntTy); }
    void do_lastore() { do_astore_common(Type::LongTy); }
    void do_fastore() { do_astore_common(Type::FloatTy); }
    void do_dastore() { do_astore_common(Type::DoubleTy); }
    void do_aastore() {
      ClassFile* root = ClassFile::get("java/lang/Object");
      do_astore_common(PointerType::get(getClassInfo(root).type));
    }
    void do_bastore() { do_astore_common(Type::SByteTy); }
    void do_castore() { do_astore_common(Type::UShortTy); }
    void do_sastore() { do_astore_common(Type::ShortTy); }

    void do_astore_common(Type* type) {
      assert(0 && "not implemented");
    }

    void do_pop() {
      opStack_.pop();
    }

    void do_pop2() {
      Value* v1 = opStack_.top(); opStack_.pop();
      if (isOneSlotValue(v1))
        opStack_.pop();
    }

    void do_dup() {
      opStack_.push(opStack_.top());
    }

    void do_dup_x1() {
      Value* v1 = opStack_.top(); opStack_.pop();
      Value* v2 = opStack_.top(); opStack_.pop();
      opStack_.push(v1);
      opStack_.push(v2);
      opStack_.push(v1);
    }

    void do_dup_x2() {
      Value* v1 = opStack_.top(); opStack_.pop();
      Value* v2 = opStack_.top(); opStack_.pop();
      if (isOneSlotValue(v2)) {
        Value* v3 = opStack_.top(); opStack_.pop();
        opStack_.push(v1);
        opStack_.push(v3);
        opStack_.push(v2);
        opStack_.push(v1);
      }
      else {
        opStack_.push(v1);
        opStack_.push(v2);
        opStack_.push(v1);
      }
    }

    void do_dup2() {
      Value* v1 = opStack_.top(); opStack_.pop();
      if (isOneSlotValue(v1)) {
        Value* v2 = opStack_.top(); opStack_.pop();
        opStack_.push(v2);
        opStack_.push(v1);
        opStack_.push(v2);
        opStack_.push(v1);
      }
      else {
        opStack_.push(v1);
        opStack_.push(v1);
      }
    }

    void do_dup2_x1() {
      Value* v1 = opStack_.top(); opStack_.pop();
      Value* v2 = opStack_.top(); opStack_.pop();
      if (isOneSlotValue(v1)) {
        Value* v3 = opStack_.top(); opStack_.pop();
        opStack_.push(v2);
        opStack_.push(v1);
        opStack_.push(v3);
        opStack_.push(v2);
        opStack_.push(v1);
      }
      else {
        opStack_.push(v1);
        opStack_.push(v2);
        opStack_.push(v1);
      }
    }

    void do_dup2_x2() {
      Value* v1 = opStack_.top(); opStack_.pop();
      Value* v2 = opStack_.top(); opStack_.pop();
      if (isOneSlotValue(v1)) {
        Value* v3 = opStack_.top(); opStack_.pop();
        if (isOneSlotValue(v3)) {
          Value* v4 = opStack_.top(); opStack_.pop();
          opStack_.push(v2);
          opStack_.push(v1);
          opStack_.push(v4);
          opStack_.push(v3);
          opStack_.push(v2);
          opStack_.push(v1);
        }
        else {
          opStack_.push(v2);
          opStack_.push(v1);
          opStack_.push(v3);
          opStack_.push(v2);
          opStack_.push(v1);
        }
      }
      else {
        if (isOneSlotValue(v2)) {
          Value* v3 = opStack_.top(); opStack_.pop();
          opStack_.push(v1);
          opStack_.push(v3);
          opStack_.push(v2);
          opStack_.push(v1);
        }
        else {
          opStack_.push(v1);
          opStack_.push(v2);
          opStack_.push(v1);
        }
      }
    }

    void do_swap() {
      Value* v1 = opStack_.top(); opStack_.pop();
      Value* v2 = opStack_.top(); opStack_.pop();
      opStack_.push(v1);
      opStack_.push(v2);
    }

    void do_iadd() { do_binary_op_common(Instruction::Add); }
    void do_ladd() { do_binary_op_common(Instruction::Add); }
    void do_fadd() { do_binary_op_common(Instruction::Add); }
    void do_dadd() { do_binary_op_common(Instruction::Add); }

    void do_isub() { do_binary_op_common(Instruction::Sub); }
    void do_lsub() { do_binary_op_common(Instruction::Sub); }
    void do_fsub() { do_binary_op_common(Instruction::Sub); }
    void do_dsub() { do_binary_op_common(Instruction::Sub); }

    void do_imul() { do_binary_op_common(Instruction::Mul); }
    void do_lmul() { do_binary_op_common(Instruction::Mul); }
    void do_fmul() { do_binary_op_common(Instruction::Mul); }
    void do_dmul() { do_binary_op_common(Instruction::Mul); }

    void do_idiv() { do_binary_op_common(Instruction::Div); }
    void do_ldiv() { do_binary_op_common(Instruction::Div); }
    void do_fdiv() { do_binary_op_common(Instruction::Div); }
    void do_ddiv() { do_binary_op_common(Instruction::Div); }

    void do_irem() { do_binary_op_common(Instruction::Rem); }
    void do_lrem() { do_binary_op_common(Instruction::Rem); }
    void do_frem() { do_binary_op_common(Instruction::Rem); }
    void do_drem() { do_binary_op_common(Instruction::Rem); }

    void do_ineg() { do_neg_common(); }
    void do_lneg() { do_neg_common(); }
    void do_fneg() { do_neg_common(); }
    void do_dneg() { do_neg_common(); }

    void do_neg_common() {
      Value* v1 = opStack_.top(); opStack_.pop();
      opStack_.push(BinaryOperator::createNeg(v1, TMP, current_));
    }

    void do_ishl() { do_shift_common(Instruction::Shl); }
    void do_lshl() { do_shift_common(Instruction::Shl); }
    void do_ishr() { do_shift_common(Instruction::Shr); }
    void do_lshr() { do_shift_common(Instruction::Shr); }

    void do_iushr() { do_shift_unsigned_common(); }
    void do_lushr() { do_shift_unsigned_common(); }

    void do_shift_unsigned_common() {
      // cast value to be shifted into its unsigned version
      do_swap();
      Value* value = opStack_.top(); opStack_.pop();
      value = new CastInst(value, value->getType()->getUnsignedVersion(),
                           TMP, current_);
      opStack_.push(value);
      do_swap();

      do_shift_common(Instruction::Shr);

      value = opStack_.top(); opStack_.pop();
      // cast shifted value back to its original signed version
      opStack_.push(new CastInst(value, value->getType()->getSignedVersion(),
                                 TMP, current_));
    }

    void do_shift_common(Instruction::OtherOps op) {
      Value* amount = opStack_.top(); opStack_.pop();
      Value* value = opStack_.top(); opStack_.pop();
      amount = new CastInst(amount, Type::UByteTy, TMP, current_);
      opStack_.push(new ShiftInst(op, value, amount, TMP, current_));
    }

    void do_iand() { do_binary_op_common(Instruction::And); }
    void do_land() { do_binary_op_common(Instruction::And); }
    void do_ior() { do_binary_op_common(Instruction::Or); }
    void do_lor() { do_binary_op_common(Instruction::Or); }
    void do_ixor() { do_binary_op_common(Instruction::Xor); }
    void do_lxor() { do_binary_op_common(Instruction::Xor); }

    void do_binary_op_common(Instruction::BinaryOps op) {
      Value* v2 = opStack_.top(); opStack_.pop();
      Value* v1 = opStack_.top(); opStack_.pop();
      opStack_.push(BinaryOperator::create(op, v1, v2, TMP,current_));
    }


    void do_iinc(unsigned index, int amount) {
      Value* v = new LoadInst(getOrCreateLocal(index, Type::IntTy),
                              TMP, current_);
      BinaryOperator::createAdd(v, ConstantSInt::get(Type::IntTy, amount),
                                TMP, current_);
      new StoreInst(v, getOrCreateLocal(index, Type::IntTy), current_);
    }

    void do_i2l() { do_cast_common(Type::LongTy); }
    void do_i2f() { do_cast_common(Type::FloatTy); }
    void do_i2d() { do_cast_common(Type::DoubleTy); }
    void do_l2i() { do_cast_common(Type::IntTy); }
    void do_l2f() { do_cast_common(Type::FloatTy); }
    void do_l2d() { do_cast_common(Type::DoubleTy); }
    void do_f2i() { do_cast_common(Type::IntTy); }
    void do_f2l() { do_cast_common(Type::LongTy); }
    void do_f2d() { do_cast_common(Type::DoubleTy); }
    void do_d2i() { do_cast_common(Type::IntTy); }
    void do_d2l() { do_cast_common(Type::LongTy); }
    void do_d2f() { do_cast_common(Type::FloatTy); }
    void do_i2b() { do_cast_common(Type::SByteTy); }
    void do_i2c() { do_cast_common(Type::UShortTy); }
    void do_i2s() { do_cast_common(Type::ShortTy); }

    void do_cast_common(Type* type) {
      Value* v1 = opStack_.top(); opStack_.pop();
      opStack_.push(new CastInst(v1, type, TMP, current_));
    }

    void do_lcmp() {
      Value* v2 = opStack_.top(); opStack_.pop();
      Value* v1 = opStack_.top(); opStack_.pop();
      Value* c = BinaryOperator::createSetGT(v1, v2, TMP, current_);
      Value* r = new SelectInst(c, ConstantSInt::get(Type::IntTy, 1),
                                ConstantSInt::get(Type::IntTy, 0), TMP,
                                current_);
      c = BinaryOperator::createSetLT(v1, v2, TMP, current_);
      r = new SelectInst(c, ConstantSInt::get(Type::IntTy, -1), r, TMP,
                         current_);
      opStack_.push(r);
    }

    void do_fcmpl() { do_cmp_common(-1); }
    void do_dcmpl() { do_cmp_common(-1); }
    void do_fcmpg() { do_cmp_common(1); }
    void do_dcmpg() { do_cmp_common(1); }

    void do_cmp_common(int valueIfUnordered) {
      Value* v2 = opStack_.top(); opStack_.pop();
      Value* v1 = opStack_.top(); opStack_.pop();
      Value* c = BinaryOperator::createSetGT(v1, v2, TMP, current_);
      Value* r = new SelectInst(c, ConstantSInt::get(Type::IntTy, 1),
                                ConstantSInt::get(Type::IntTy, 0), TMP,
                                current_);
      c = BinaryOperator::createSetLT(v1, v2, TMP, current_);
      r = new SelectInst(c, ConstantSInt::get(Type::IntTy, -1), r, TMP,
                         current_);
      c = new CallInst(module_.getOrInsertFunction
                       ("llvm.isunordered",
                        Type::BoolTy, v1->getType(), v2->getType(), 0),
                       v1, v2, TMP, current_);
      r = new SelectInst(c, ConstantSInt::get(Type::IntTy, valueIfUnordered),
                         r, TMP, current_);
      opStack_.push(r);
    }

    void do_ifeq(unsigned t, unsigned f) { do_if_common(Instruction::SetEQ, t, f); }
    void do_ifne(unsigned t, unsigned f) { do_if_common(Instruction::SetNE, t, f); }
    void do_iflt(unsigned t, unsigned f) { do_if_common(Instruction::SetLT, t, f); }
    void do_ifge(unsigned t, unsigned f) { do_if_common(Instruction::SetGE, t, f); }
    void do_ifgt(unsigned t, unsigned f) { do_if_common(Instruction::SetGT, t, f); }
    void do_ifle(unsigned t, unsigned f) { do_if_common(Instruction::SetLE, t, f); }

    void do_if_common(Instruction::BinaryOps cc, unsigned t, unsigned f) {
      Value* v1 = opStack_.top(); opStack_.pop();
      Value* v2 = llvm::Constant::getNullValue(Type::IntTy);
      Value* c = new SetCondInst(cc, v1, v2, TMP, current_);
      new BranchInst(mapper_->getBBAt(t), mapper_->getBBAt(f), c, current_);
    }

    void do_goto(unsigned target) {
      new BranchInst(mapper_->getBBAt(target), current_);
    }

    void do_jsr(unsigned target) {
      assert(0 && "not implemented");
    }

    void do_ret(unsigned index) {
      assert(0 && "not implemented");
    }

    void do_switch(unsigned defTarget, const SwitchCases& sw) {
      Value* v = opStack_.top(); opStack_.pop();
      SwitchInst* in = new SwitchInst(v, mapper_->getBBAt(defTarget), current_);
      for (unsigned i = 0, e = sw.size(); i != e; ++i)
        in->addCase(ConstantSInt::get(Type::IntTy, sw[i].first),
                    mapper_->getBBAt(sw[i].second));
    }

    void do_ireturn() { do_return_common(); }
    void do_lreturn() { do_return_common(); }
    void do_freturn() { do_return_common(); }
    void do_dreturn() { do_return_common(); }
    void do_areturn() { do_return_common(); }

    void do_return_common() {
      Value* v1 = opStack_.top(); opStack_.pop();
      new ReturnInst(v1, current_);
    }

    void do_return() {
      new ReturnInst(NULL, current_);
    }

    void do_getstatic(unsigned index) {
      Value* v = new LoadInst(getStaticField(index), TMP, current_);
      opStack_.push(v);
    }

    void do_putstatic(unsigned index) {
      Value* v = opStack_.top(); opStack_.pop();
      Value* ptr = getStaticField(index);
      const Type* fieldTy = cast<PointerType>(ptr->getType())->getElementType();
      if (v->getType() != fieldTy)
        v = new CastInst(v, fieldTy, TMP, current_);
      new StoreInst(v, ptr, current_);
    }

    void do_getfield(unsigned index) {
      Value* p = opStack_.top(); opStack_.pop();
      Value* v = new LoadInst(getField(index, p), TMP, current_);
      opStack_.push(v);
    }

    void do_putfield(unsigned index) {
      Value* v = opStack_.top(); opStack_.pop();
      Value* p = opStack_.top(); opStack_.pop();
      new StoreInst(v, getField(index, p), current_);
    }

    void makeCall(Value* fun, const std::vector<Value*> params) {
      const PointerType* funPtrTy = cast<PointerType>(fun->getType());
      const FunctionType* funTy =
        cast<FunctionType>(funPtrTy->getElementType());

      if (funTy->getReturnType() == Type::VoidTy)
        new CallInst(fun, params, "", current_);
      else {
        Value* r = new CallInst(fun, params, TMP, current_);
        opStack_.push(r);
      }
    }

    std::vector<Value*> getParams(FunctionType* funTy) {
      unsigned numParams = funTy->getNumParams();
      std::vector<Value*> params(numParams);
      while (numParams--) {
        Value* p = opStack_.top(); opStack_.pop();
        params[numParams] =
          p->getType() == funTy->getParamType(numParams) ?
          p :
          new CastInst(p, funTy->getParamType(numParams), TMP, current_);
      }

      return params;
    }

    void do_invokevirtual(unsigned index) {
      ConstantMethodRef* methodRef = cf_->getConstantMethodRef(index);
      ConstantNameAndType* nameAndType = methodRef->getNameAndType();

      ClassFile* cf = ClassFile::get(methodRef->getClass()->getName()->str());
      const ClassInfo& ci = getClassInfo(cf);
      const VTableInfo& vi = getVTableInfo(cf);

      const std::string& className = cf->getThisClass()->getName()->str();
      const std::string& methodDescr =
        nameAndType->getName()->str() +
        nameAndType->getDescriptor()->str();

      FunctionType* funTy =
        cast<FunctionType>(getType(nameAndType->getDescriptor(), ci.type));

      std::vector<Value*> params(getParams(funTy));

      Value* objRef = params.front();
      objRef = new CastInst(objRef, PointerType::get(ci.type),
                            "this", current_);
      Value* objBase = getField(cf, LLVM_JAVA_OBJECT_BASE, objRef);
      Function* f = module_.getOrInsertFunction(
        LLVM_JAVA_GETOBJECTCLASS, PointerType::get(VTableInfo::VTableTy),
        objBase->getType(), NULL);
      Value* vtable = new CallInst(f, objBase, TMP, current_);
      vtable = new CastInst(vtable, PointerType::get(vi.vtable->getType()),
                            TMP, current_);
      vtable = new LoadInst(vtable, className + "<vtable>", current_);
      std::vector<Value*> indices(1, ConstantUInt::get(Type::UIntTy, 0));
      assert(vi.m2iMap.find(methodDescr) != vi.m2iMap.end() &&
             "could not find slot for virtual function!");
      unsigned vSlot = vi.m2iMap.find(methodDescr)->second;
      indices.push_back(ConstantUInt::get(Type::UIntTy, vSlot));
      Value* vfunPtr =
        new GetElementPtrInst(vtable, indices, TMP, current_);
      Value* vfun = new LoadInst(vfunPtr, methodDescr, current_);

      makeCall(vfun, params);
    }

    void do_invokespecial(unsigned index) {
      ConstantMethodRef* methodRef = cf_->getConstantMethodRef(index);
      ConstantNameAndType* nameAndType = methodRef->getNameAndType();

      const std::string& className = methodRef->getClass()->getName()->str();
      const std::string& methodName = nameAndType->getName()->str();
      const std::string& methodDescr =
        methodName + nameAndType->getDescriptor()->str();
      std::string funcName = className + '/' + methodDescr;
      const ClassInfo& ci = getClassInfo(ClassFile::get(className));

      // constructor calls are statically bound
      if (methodName == "<init>") {
        FunctionType* funcTy =
          cast<FunctionType>(getType(nameAndType->getDescriptor(), ci.type));
        Function* function = module_.getOrInsertFunction(funcName, funcTy);
        toCompileFunctions_.insert(function);
        makeCall(function, getParams(funcTy));
      }
      // otherwise we call the superclass' implementation of the method
      else {
        assert(0 && "not implemented");
      }
    }

    void do_invokestatic(unsigned index) {
      ConstantMethodRef* methodRef = cf_->getConstantMethodRef(index);
      ConstantNameAndType* nameAndType = methodRef->getNameAndType();

      std::string funcName =
        methodRef->getClass()->getName()->str() + '/' +
        nameAndType->getName()->str() +
        nameAndType->getDescriptor()->str();

      FunctionType* funcTy =
        cast<FunctionType>(getType(nameAndType->getDescriptor()));
      Function* function = module_.getOrInsertFunction(funcName, funcTy);
      toCompileFunctions_.insert(function);
      makeCall(function, getParams(funcTy));
    }

    void do_invokeinterface(unsigned index) {
      ConstantInterfaceMethodRef* methodRef =
        cf_->getConstantInterfaceMethodRef(index);
      ConstantNameAndType* nameAndType = methodRef->getNameAndType();

      ClassFile* cf = ClassFile::get(methodRef->getClass()->getName()->str());
      const ClassInfo& ci = getClassInfo(cf);
      const VTableInfo& vi = getVTableInfo(cf);

      const std::string& className = cf->getThisClass()->getName()->str();
      const std::string& methodDescr =
        nameAndType->getName()->str() +
        nameAndType->getDescriptor()->str();

      FunctionType* funTy =
        cast<FunctionType>(getType(nameAndType->getDescriptor(), ci.type));

      std::vector<Value*> params(getParams(funTy));

      Value* objRef = params.front();
      objRef = new CastInst(objRef, PointerType::get(ci.type),
                            "this", current_);
      Value* objBase = getField(cf, LLVM_JAVA_OBJECT_BASE, objRef);
      Function* f = module_.getOrInsertFunction(
        LLVM_JAVA_GETOBJECTCLASS, PointerType::get(VTableInfo::VTableTy),
        objBase->getType(), NULL);
      Value* vtable = new CallInst(f, objBase, TMP, current_);
      // get the interfaces array of vtables
      std::vector<Value*> indices(2, ConstantUInt::get(Type::UIntTy, 0));
      indices.push_back(ConstantUInt::get(Type::UIntTy, 3));
      Value* interfaceVTables =
        new GetElementPtrInst(vtable, indices, TMP, current_);
      interfaceVTables = new LoadInst(interfaceVTables, TMP, current_);
      // get the actual interface vtable
      indices.clear();
      indices.push_back(ConstantUInt::get(Type::UIntTy, ci.interfaceIdx));
      Value* interfaceVTable =
        new GetElementPtrInst(interfaceVTables, indices, TMP, current_);
      interfaceVTable =
        new LoadInst(interfaceVTable, className + "<vtable>", current_);
      interfaceVTable =
        new CastInst(interfaceVTable, vi.vtable->getType(), TMP, current_);
      // get the function pointer
      indices.resize(1);
      assert(vi.m2iMap.find(methodDescr) != vi.m2iMap.end() &&
             "could not find slot for virtual function!");
      unsigned vSlot = vi.m2iMap.find(methodDescr)->second;
      indices.push_back(ConstantUInt::get(Type::UIntTy, vSlot));
      Value* vfunPtr =
        new GetElementPtrInst(interfaceVTable, indices, TMP, current_);
      Value* vfun = new LoadInst(vfunPtr, methodDescr, current_);

      makeCall(vfun, params);
    }

    void do_new(unsigned index) {
      ConstantClass* classRef = cf_->getConstantClass(index);
      ClassFile* cf = ClassFile::get(classRef->getName()->str());
      const ClassInfo& ci = getClassInfo(cf);
      const VTableInfo& vi = getVTableInfo(cf);

      Value* objRef = new MallocInst(ci.type,
                                     ConstantUInt::get(Type::UIntTy, 0),
                                     TMP, current_);
      Value* objBase = getField(cf, LLVM_JAVA_OBJECT_BASE, objRef);
      Function* f = module_.getOrInsertFunction(
        LLVM_JAVA_GETOBJECTCLASS, PointerType::get(VTableInfo::VTableTy),
        objBase->getType(), NULL);
      Value* vtable = new CallInst(f, objBase, TMP, current_);
      vtable = new CastInst(vtable, PointerType::get(vi.vtable->getType()),
                            TMP, current_);
      vtable = new StoreInst(vi.vtable, vtable, current_);
      opStack_.push(objRef);
    }

    void do_newarray(JType type) {
      assert(0 && "not implemented");
    }

    void do_anewarray(unsigned index) {
      assert(0 && "not implemented");
    }

    void do_arraylength() {
      assert(0 && "not implemented");
    }

    void do_athrow() {
      Value* objRef = opStack_.top(); opStack_.pop();
      objRef = new CastInst(objRef, PointerType::get(ClassInfo::ObjectBaseTy),
                            TMP, current_);
      Function* f = module_.getOrInsertFunction(
        LLVM_JAVA_THROW, Type::IntTy, objRef->getType(), NULL);
      new CallInst(f, objRef, TMP, current_);
    }

    void do_checkcast(unsigned index) {
      do_dup();
      do_instanceof(index);
      Value* r = opStack_.top(); opStack_.pop();
      Value* b = new SetCondInst(Instruction::SetEQ,
                                 r, ConstantSInt::get(Type::IntTy, 1),
                                 TMP, current_);
      // FIXME: if b is false we must throw a ClassCast exception
    }

    void do_instanceof(unsigned index) {
      ConstantClass* classRef = cf_->getConstantClass(index);
      ClassFile* cf = ClassFile::get(classRef->getName()->str());
      const VTableInfo& vi = getVTableInfo(cf);

      Value* objRef = opStack_.top(); opStack_.pop();
      Value* objBase = getField(cf, LLVM_JAVA_OBJECT_BASE, objRef);
      Function* f = module_.getOrInsertFunction(
        LLVM_JAVA_ISINSTANCEOF, Type::IntTy,
        objBase->getType(), PointerType::get(VTableInfo::VTableTy), NULL);
      Value* vtable = new CastInst(vi.vtable,
                                   PointerType::get(VTableInfo::VTableTy),
                                   TMP, current_);
      Value* r = new CallInst(f, objBase, vtable, TMP, current_);
      opStack_.push(r);
    }

    void do_monitorenter() {
      assert(0 && "not implemented");
    }

    void do_monitorexit() {
      assert(0 && "not implemented");
    }

    void do_multianewarray(unsigned index, unsigned dims) {
      assert(0 && "not implemented");
    }
  };

  unsigned Compiler::ClassInfo::InterfaceCount = 0;
  Type* Compiler::ClassInfo::ObjectBaseTy;
  StructType* Compiler::VTableInfo::VTableTy;
  StructType* Compiler::VTableInfo::TypeInfoTy;

} } } // namespace llvm::Java::

std::auto_ptr<Module> llvm::Java::compile(const std::string& className)
{
  DEBUG(std::cerr << "Compiling class: " << className << '\n');

  std::auto_ptr<Module> m(new Module(className));

  Compiler c(*m);
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

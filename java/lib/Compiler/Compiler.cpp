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

using namespace llvm;
using namespace llvm::Java;

namespace llvm { namespace Java { namespace {

  const std::string TMP("tmp");

  typedef std::vector<BasicBlock*> BC2BBMap;
  typedef std::stack<Value*, std::vector<Value*> > OperandStack;
  typedef std::vector<Value*> Locals;

  inline bool isTwoSlotValue(const Value* v) {
    return v->getType() == Type::LongTy | v->getType() == Type::DoubleTy;
  }

  inline bool isOneSlotValue(const Value* v) {
    return !isTwoSlotValue(v);
  }

  struct Bytecode2BasicBlockMapper
    : public BytecodeParser<Bytecode2BasicBlockMapper> {
  public:
    Bytecode2BasicBlockMapper(Function& f,
                              BC2BBMap& m,
                              CodeAttribute& c)
      : function_(f), bc2bbMap_(m), codeAttr_(c) { }

    void compute() {
      bc2bbMap_.clear();
      bc2bbMap_.assign(codeAttr_.getCodeSize(), NULL);

      BasicBlock* bb = new BasicBlock("entry", &function_);

      parse(codeAttr_.getCode(), codeAttr_.getCodeSize());

      for (unsigned i = 0; i < bc2bbMap_.size(); ++i) {
        if (bc2bbMap_[i])
          bb = bc2bbMap_[i];
        else
          bc2bbMap_[i] = bb;
      }

      assert(function_.getEntryBlock().getName() == "entry");
    }

    void do_if(unsigned bcI, JSetCC cc, JType type,
               unsigned t, unsigned f) {
      if (!bc2bbMap_[t])
        bc2bbMap_[t] = new BasicBlock("bc" + utostr(t), &function_);
      if (!bc2bbMap_[f])
        bc2bbMap_[f] = new BasicBlock("bc" + utostr(f), &function_);
    }

    void do_ifcmp(unsigned bcI, JSetCC cc,
                  unsigned t, unsigned f) {
      if (!bc2bbMap_[t])
        bc2bbMap_[t] = new BasicBlock("bc" + utostr(t), &function_);
      if (!bc2bbMap_[f])
        bc2bbMap_[f] = new BasicBlock("bc" + utostr(f), &function_);
    }

    void do_switch(unsigned bcI,
                   unsigned defTarget,
                   const SwitchCases& sw) {
      for (unsigned i = 0; i < sw.size(); ++i) {
        unsigned target = sw[i].second;
        if (!bc2bbMap_[target])
          bc2bbMap_[target] = new BasicBlock("bc" + utostr(target), &function_);
      }
      if (!bc2bbMap_[defTarget])
        bc2bbMap_[defTarget] =
          new BasicBlock("bc" + utostr(defTarget), &function_);
    }

  private:
    Function& function_;
    BC2BBMap& bc2bbMap_;
    const CodeAttribute& codeAttr_;
  };

  struct CompilerImpl :
    public BytecodeParser<CompilerImpl> {
  private:
    Module* module_;
    ClassFile* cf_;
    OperandStack opStack_;
    Locals locals_;
    BC2BBMap bc2bbMap_;
    BasicBlock* prologue_;

    typedef SetVector<Function*> FunctionSet;
    FunctionSet toCompileFunctions_;

    struct ClassInfo {
      ClassInfo() : type(NULL), interfaceIdx(0) { }
      Type* type;
      unsigned interfaceIdx;
      typedef std::map<std::string, unsigned> Field2IndexMap;
      Field2IndexMap f2iMap;

      static unsigned InterfaceCount;
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

  private:
    BasicBlock* getBBAt(unsigned bcI) { return bc2bbMap_[bcI]; }

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
      case REFERENCE:
        return PointerType::get(
          getClassInfo(ClassFile::get("java/lang/Object")).type);
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

    Instruction::BinaryOps getSetCC(JSetCC cc) {
      switch (cc) {
      case EQ: return Instruction::SetEQ;
      case NE: return Instruction::SetNE;
      case LT: return Instruction::SetLT;
      case GE: return Instruction::SetGE;
      case GT: return Instruction::SetGT;
      case LE: return Instruction::SetLE;
      default: assert(0 && "Invalid JSetCC to BinaryOps conversion!");
      }
      return static_cast<Instruction::BinaryOps>(-1);
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
      Type* base = OpaqueType::get();
      module_->addTypeName(LLVM_JAVA_OBJECT_BASE, base);
      ci.f2iMap.insert(std::make_pair(LLVM_JAVA_OBJECT_BASE, elements.size()));
      elements.push_back(base);

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
      module_->addTypeName("java/lang/Object", ci.type);

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
      elements.push_back(Type::UIntTy);
      init.push_back(llvm::ConstantUInt::get(elements[0], 0));
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
      module_->addTypeName(LLVM_JAVA_OBJECT_TYPEINFO, VTableInfo::TypeInfoTy);
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
            getType(method->getDescriptor(), getClassInfo(cf).type));

          Function* vfun = module_->getOrInsertFunction(funcName, funcTy);
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
      module_->addTypeName("java/lang/Object<vtable>", VTableInfo::VTableTy);

      vi.vtable = new GlobalVariable(VTableInfo::VTableTy,
                                     true, GlobalVariable::ExternalLinkage,
                                     ConstantStruct::get(init),
                                     "java/lang/Object<vtable>",
                                     module_);
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
      module_->addTypeName(className, ci.type);
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
        module_);

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
      module_->addTypeName(globalName, vtable->getType());

      return new GlobalVariable(
        vtable->getType(),
        true,
        GlobalVariable::ExternalLinkage,
        vtable,
        globalName,
        module_);
    }

    std::pair<int, llvm::Constant*>
    buildInterfacesVTables(ClassFile* cf, const VTableInfo& vi) {

      std::vector<llvm::Constant*> vtables;
      const Classes& interfaces = cf->getInterfaces();
      llvm::Constant* nullVTable =
        llvm::Constant::getNullValue(PointerType::get(VTableInfo::VTableTy));

      for (unsigned i = 0, e = interfaces.size(); i != e; ++i) {
        ClassFile* interface = ClassFile::get(interfaces[i]->getName()->str());
        assert(interface->isInterface() &&
               "Class in interfaces list is not an interface!");
        const ClassInfo& interfaceCI = getClassInfo(interface);
        vtables.resize(interfaceCI.interfaceIdx, nullVTable);
        vtables[interfaceCI.interfaceIdx] = buildInterfaceVTable(cf, interface);
      }

      ArrayType* interfacesArrayTy =
        ArrayType::get(PointerType::get(VTableInfo::VTableTy), vtables.size());

      const std::string& globalName =
        cf->getThisClass()->getName()->str() + "<interfacesvtables>";
      module_->addTypeName(globalName, interfacesArrayTy);

      GlobalVariable* interfacesArray = new GlobalVariable(
        interfacesArrayTy,
        true,
        GlobalVariable::ExternalLinkage,
        ConstantArray::get(interfacesArrayTy, vtables),
        globalName,
        module_);

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
      typeInfoInit.push_back(ConstantUInt::get(Type::UIntTy, depth));
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
          Function* vfun = module_->getOrInsertFunction(funcName, funcTy);
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
      module_->addTypeName(globalName, vtable->getType());
      vi.vtable = new GlobalVariable(vtable->getType(),
                                     true,
                                     GlobalVariable::ExternalLinkage,
                                     vtable,
                                     globalName,
                                     module_);

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
      GlobalVariable* global = module_->getGlobalVariable
        (globalName, getType(nameAndType->getDescriptor()));
      assert(global && "Got NULL global variable!");

      return global;
    }

    Value* getField(unsigned bcI, unsigned index, Value* ptr) {
      ConstantFieldRef* fieldRef = cf_->getConstantFieldRef(index);
      ConstantNameAndType* nameAndType = fieldRef->getNameAndType();
      ClassFile* cf = ClassFile::get(fieldRef->getClass()->getName()->str());
      return getField(bcI,
                      cf,
                      nameAndType->getName()->str(),
                      ptr);
    }

    Value* getField(unsigned bcI,
                    ClassFile* cf,
                    const std::string& fieldName,
                    Value* ptr) {
      // Cast ptr to correct type
      ptr = new CastInst(ptr, PointerType::get(getClassInfo(cf).type),
                         TMP, getBBAt(bcI));

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

      return new GetElementPtrInst(ptr, indices, TMP, getBBAt(bcI));
    }

    Function* compileMethodOnly(const std::string& classMethodDesc) {
      Method* method;
      tie(cf_, method) = findClassAndMethod(classMethodDesc);

      std::string name = cf_->getThisClass()->getName()->str();
      name += '/';
      name += method->getName()->str();
      name += method->getDescriptor()->str();

      Function* function = module_->getOrInsertFunction
        (name, cast<FunctionType>(getType(method->getDescriptor())));
      function->setLinkage(method->isPrivate() ?
                           Function::InternalLinkage :
                           Function::ExternalLinkage);

      if (method->isNative()) {
        DEBUG(std::cerr << "Ignoring native method: ";
              std::cerr << classMethodDesc << '\n');
        return function;
      }

      DEBUG(std::cerr << "Compiling method: " << classMethodDesc << '\n');

      Java::CodeAttribute* codeAttr = method->getCodeAttribute();

      while (!opStack_.empty())
        opStack_.pop();

      locals_.clear();
      locals_.assign(codeAttr->getMaxLocals(), NULL);

      Bytecode2BasicBlockMapper mapper(*function, bc2bbMap_, *codeAttr);
      mapper.compute();

      prologue_ = new BasicBlock("prologue");

      parse(codeAttr->getCode(), codeAttr->getCodeSize());

      // if the prologue is not empty, make it the entry block
      // of the function with entry as its only successor
      if (prologue_->empty())
        delete prologue_;
      else {
        function->getBasicBlockList().push_front(prologue_);
        new BranchInst(prologue_->getNext(), prologue_);
      }

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

      Function* hook = module_->getOrInsertFunction(LLVM_JAVA_STATIC_INIT,
                                                    Type::VoidTy, 0);
      Function* init = module_->getOrInsertFunction(name, Type::VoidTy, 0);

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
                               module_);
          }
        }
      }
    }

    std::pair<ClassFile*, Method*>
    findClassAndMethod(const std::string& classMethodDesc) {
      unsigned slash = classMethodDesc.rfind('/', classMethodDesc.find('('));
      std::string className = classMethodDesc.substr(0, slash);
      std::string methodNameAndDescr = classMethodDesc.substr(slash+1);

      ClassFile* classfile = ClassFile::get(className);
      emitStaticInitializers(classfile);
      Method* method = classfile->getMethod(methodNameAndDescr);

      if (!method)
        throw InvocationTargetException("Method " + methodNameAndDescr +
                                        " not found in class " + className);

      return std::make_pair(classfile, method);
    }

  public:
    Function* compileMethod(Module& module,
                            const std::string& classMethodDesc) {
      module_ = &module;

      // initialize the static initializer function
      Function* staticInit =
        module_->getOrInsertFunction(LLVM_JAVA_STATIC_INIT,
                                     Type::VoidTy, 0);
      BasicBlock* staticInitBB = new BasicBlock("entry", staticInit);
      new ReturnInst(NULL, staticInitBB);

      // initialize type maps and globals (vtables)
      initializeTypeMaps();

      // compile the method requested
      Function* function = compileMethodOnly(classMethodDesc);
      // compile all other methods called by this method recursively
      for (unsigned i = 0; i != toCompileFunctions_.size(); ++i) {
        Function* f = toCompileFunctions_[i];
//        compileMethodOnly(f->getName());
      }

      return function;
    }

    void do_aconst_null(unsigned bcI) {
      opStack_.push(llvm::Constant::getNullValue(getType(REFERENCE)));
    }

    void do_iconst(unsigned bcI, int value) {
      opStack_.push(ConstantSInt::get(Type::IntTy, value));
    }

    void do_lconst(unsigned bcI, long long value) {
      opStack_.push(ConstantSInt::get(Type::LongTy, value));
    }

    void do_fconst(unsigned bcI, float value) {
      opStack_.push(ConstantFP::get(Type::FloatTy, value));
    }

    void do_dconst(unsigned bcI, double value) {
      opStack_.push(ConstantFP::get(Type::DoubleTy, value));
    }

    void do_ldc(unsigned bcI, unsigned index) {
      Constant* c = cf_->getConstant(index);
      assert(getConstant(c) && "Java constant not handled!");
      opStack_.push(getConstant(c));
    }

    void do_load(unsigned bcI, JType type, unsigned index) {
      opStack_.push(new LoadInst(getOrCreateLocal(index, getType(type)),
                                 TMP, getBBAt(bcI)));
    }

    void do_aload(unsigned bcI, JType type) {
      assert(0 && "not implemented");
    }

    void do_store(unsigned bcI, JType type, unsigned index) {
      Value* val = opStack_.top(); opStack_.pop();
      const Type* valTy = val->getType();
      Value* ptr = getOrCreateLocal(index, getType(type));
      if (!valTy->isPrimitiveType() &&
          valTy != cast<PointerType>(ptr->getType())->getElementType())
        ptr = new CastInst(ptr, PointerType::get(valTy), TMP, getBBAt(bcI));
      opStack_.push(new StoreInst(val, ptr, getBBAt(bcI)));
    }

    void do_astore(unsigned bcI, JType type) {
      assert(0 && "not implemented");
    }

    void do_pop(unsigned bcI) {
      opStack_.pop();
    }

    void do_pop2(unsigned bcI) {
      Value* v1 = opStack_.top(); opStack_.pop();
      if (isOneSlotValue(v1))
        opStack_.pop();
    }

    void do_dup(unsigned bcI) {
      opStack_.push(opStack_.top());
    }

    void do_dup_x1(unsigned bcI) {
      Value* v1 = opStack_.top(); opStack_.pop();
      Value* v2 = opStack_.top(); opStack_.pop();
      opStack_.push(v1);
      opStack_.push(v2);
      opStack_.push(v1);
    }

    void do_dup_x2(unsigned bcI) {
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

    void do_dup2(unsigned bcI) {
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

    void do_dup2_x1(unsigned bcI) {
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

    void do_dup2_x2(unsigned bcI) {
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

    void do_swap(unsigned bcI) {
      Value* v1 = opStack_.top(); opStack_.pop();
      Value* v2 = opStack_.top(); opStack_.pop();
      opStack_.push(v1);
      opStack_.push(v2);
    }

    void do_add(unsigned bcI) {
      do_binary_op_common(bcI, Instruction::Add);
    }

    void do_sub(unsigned bcI) {
      do_binary_op_common(bcI, Instruction::Sub);
    }

    void do_mul(unsigned bcI) {
      do_binary_op_common(bcI, Instruction::Mul);
    }

    void do_div(unsigned bcI) {
      do_binary_op_common(bcI, Instruction::Div);
    }

    void do_rem(unsigned bcI) {
      do_binary_op_common(bcI, Instruction::Rem);
    }

    void do_neg(unsigned bcI) {
      Value* v1 = opStack_.top(); opStack_.pop();
      opStack_.push(BinaryOperator::createNeg(v1, TMP, getBBAt(bcI)));
    }

    void do_shl(unsigned bcI) {
      do_shift_common(bcI, Instruction::Shl);
    }

    void do_shr(unsigned bcI) {
      do_shift_common(bcI, Instruction::Shr);
    }

    void do_ushr(unsigned bcI) {
      // cast value to be shifted into its unsigned version
      do_swap(bcI);
      Value* value = opStack_.top(); opStack_.pop();
      value = new CastInst(value, value->getType()->getUnsignedVersion(),
                           TMP, getBBAt(bcI));
      opStack_.push(value);
      do_swap(bcI);

      do_shift_common(bcI, Instruction::Shr);

      value = opStack_.top(); opStack_.pop();
      // cast shifted value back to its original signed version
      opStack_.push(new CastInst(value, value->getType()->getSignedVersion(),
                                 TMP, getBBAt(bcI)));
    }

    void do_shift_common(unsigned bcI, Instruction::OtherOps op) {
      Value* amount = opStack_.top(); opStack_.pop();
      Value* value = opStack_.top(); opStack_.pop();
      amount = new CastInst(amount, Type::UByteTy, TMP, getBBAt(bcI));
      opStack_.push(new ShiftInst(op, value, amount, TMP, getBBAt(bcI)));
    }

    void do_and(unsigned bcI) {
      do_binary_op_common(bcI, Instruction::And);
    }

    void do_or(unsigned bcI) {
      do_binary_op_common(bcI, Instruction::Or);
    }

    void do_xor(unsigned bcI) {
      do_binary_op_common(bcI, Instruction::Xor);
    }

    void do_binary_op_common(unsigned bcI, Instruction::BinaryOps op) {
      Value* v2 = opStack_.top(); opStack_.pop();
      Value* v1 = opStack_.top(); opStack_.pop();
      opStack_.push(BinaryOperator::create(op, v1, v2, TMP,getBBAt(bcI)));
    }


    void do_iinc(unsigned bcI, unsigned index, int amount) {
      Value* v = new LoadInst(getOrCreateLocal(index, Type::IntTy),
                              TMP, getBBAt(bcI));
      BinaryOperator::createAdd(v, ConstantSInt::get(Type::IntTy, amount),
                                TMP, getBBAt(bcI));
      new StoreInst(v, getOrCreateLocal(index, Type::IntTy), getBBAt(bcI));
    }

    void do_convert(unsigned bcI, JType to) {
      Value* v1 = opStack_.top(); opStack_.pop();
      opStack_.push(new CastInst(v1, getType(to), TMP, getBBAt(bcI)));
    }

    void do_lcmp(unsigned bcI) {
      Value* v2 = opStack_.top(); opStack_.pop();
      Value* v1 = opStack_.top(); opStack_.pop();
      Value* c = BinaryOperator::createSetGT(v1, v2, TMP, getBBAt(bcI));
      Value* r = new SelectInst(c, ConstantSInt::get(Type::IntTy, 1),
                                ConstantSInt::get(Type::IntTy, 0), TMP,
                                getBBAt(bcI));
      c = BinaryOperator::createSetLT(v1, v2, TMP, getBBAt(bcI));
      r = new SelectInst(c, ConstantSInt::get(Type::IntTy, -1), r, TMP,
                         getBBAt(bcI));
      opStack_.push(r);
    }

    void do_cmpl(unsigned bcI) {
      do_cmp_common(bcI, -1);
    }

    void do_cmpg(unsigned bcI) {
      do_cmp_common(bcI, 1);
    }

    void do_cmp_common(unsigned bcI, int valueIfUnordered) {
      Value* v2 = opStack_.top(); opStack_.pop();
      Value* v1 = opStack_.top(); opStack_.pop();
      Value* c = BinaryOperator::createSetGT(v1, v2, TMP, getBBAt(bcI));
      Value* r = new SelectInst(c, ConstantSInt::get(Type::IntTy, 1),
                                ConstantSInt::get(Type::IntTy, 0), TMP,
                                getBBAt(bcI));
      c = BinaryOperator::createSetLT(v1, v2, TMP, getBBAt(bcI));
      r = new SelectInst(c, ConstantSInt::get(Type::IntTy, -1), r, TMP,
                         getBBAt(bcI));
      c = new CallInst(module_->getOrInsertFunction
                       ("llvm.isunordered",
                        Type::BoolTy, v1->getType(), v2->getType(), 0),
                       v1, v2, TMP, getBBAt(bcI));
      r = new SelectInst(c, ConstantSInt::get(Type::IntTy, valueIfUnordered),
                         r, TMP, getBBAt(bcI));
      opStack_.push(r);
    }

    void do_if(unsigned bcI, JSetCC cc, JType type,
               unsigned t, unsigned f) {
      Value* v1 = opStack_.top(); opStack_.pop();
      Value* v2 = llvm::Constant::getNullValue(v1->getType());
      Value* c = new SetCondInst(getSetCC(cc), v1, v2, TMP, getBBAt(bcI));
      new BranchInst(getBBAt(t), getBBAt(f), c, getBBAt(bcI));
    }

    void do_ifcmp(unsigned bcI, JSetCC cc,
                  unsigned t, unsigned f) {
      Value* v2 = opStack_.top(); opStack_.pop();
      Value* v1 = opStack_.top(); opStack_.pop();
      Value* c = new SetCondInst(getSetCC(cc), v1, v2, TMP, getBBAt(bcI));
      new BranchInst(getBBAt(t), getBBAt(f), c, getBBAt(bcI));
    }

    void do_goto(unsigned bcI, unsigned target) {
      new BranchInst(getBBAt(target), getBBAt(bcI));
    }

    void do_jsr(unsigned bcI, unsigned target) {
      assert(0 && "not implemented");
    }

    void do_ret(unsigned bcI, unsigned index) {
      assert(0 && "not implemented");
    }

    void do_switch(unsigned bcI,
                   unsigned defTarget,
                   const SwitchCases& sw) {
      Value* v1 = opStack_.top(); opStack_.pop();
      SwitchInst* in = new SwitchInst(v1, getBBAt(defTarget), getBBAt(bcI));
      for (unsigned i = 0; i < sw.size(); ++i)
        in->addCase(ConstantSInt::get(Type::IntTy, sw[i].first),
                    getBBAt(sw[i].second));
    }

    void do_return(unsigned bcI) {
      Value* v1 = opStack_.top(); opStack_.pop();
      new ReturnInst(v1, getBBAt(bcI));
    }

    void do_return_void(unsigned bcI) {
      new ReturnInst(NULL, getBBAt(bcI));
    }

    void do_getstatic(unsigned bcI, unsigned index) {
      Value* v = new LoadInst(getStaticField(index), TMP, getBBAt(bcI));
      opStack_.push(v);
    }

    void do_putstatic(unsigned bcI, unsigned index) {
      Value* v = opStack_.top(); opStack_.pop();
      Value* ptr = getStaticField(index);
      const Type* fieldTy = cast<PointerType>(ptr->getType())->getElementType();
      if (v->getType() != fieldTy)
        v = new CastInst(v, fieldTy, TMP, getBBAt(bcI));
      new StoreInst(v, ptr, getBBAt(bcI));
    }

    void do_getfield(unsigned bcI, unsigned index) {
      Value* p = opStack_.top(); opStack_.pop();
      Value* v = new LoadInst(getField(bcI, index, p), TMP, getBBAt(bcI));
      opStack_.push(v);
    }

    void do_putfield(unsigned bcI, unsigned index) {
      Value* v = opStack_.top(); opStack_.pop();
      Value* p = opStack_.top(); opStack_.pop();
      new StoreInst(v, getField(bcI, index, p), getBBAt(bcI));
    }

    void makeCall(Value* fun, BasicBlock* bb) {
      const PointerType* funPtrTy = cast<PointerType>(fun->getType());
      const FunctionType* funTy = cast<FunctionType>(funPtrTy->getElementType());
      std::vector<Value*> params(funTy->getNumParams(), NULL);
      for (unsigned i = 0, e = funTy->getNumParams(); i != e; ++i) {
        Value* p = opStack_.top(); opStack_.pop();
        const Type* paramTy = funTy->getParamType(i);
        params[i] =
          p->getType() == paramTy ? p : new CastInst(p, paramTy, TMP, bb);
      }

      if (funTy->getReturnType() == Type::VoidTy)
        new CallInst(fun, params, "", bb);
      else {
        Value* r = new CallInst(fun, params, TMP, bb);
        opStack_.push(r);
      }
    }

    void do_invokevirtual(unsigned bcI, unsigned index) {
      ConstantMethodRef* methodRef = cf_->getConstantMethodRef(index);
      ConstantNameAndType* nameAndType = methodRef->getNameAndType();

      ClassFile* cf = ClassFile::get(methodRef->getClass()->getName()->str());
      const ClassInfo& ci = getClassInfo(cf);
      const VTableInfo& vi = getVTableInfo(cf);

      const std::string& className = cf->getThisClass()->getName()->str();
      const std::string& methodDescr =
        nameAndType->getName()->str() +
        nameAndType->getDescriptor()->str();

      Value* objRef = opStack_.top(); // do not pop
      objRef = new CastInst(objRef, PointerType::get(ci.type),
                            "this", getBBAt(bcI));
      Value* objBase = getField(bcI, cf, LLVM_JAVA_OBJECT_BASE, objRef);
      Function* f = module_->getOrInsertFunction(
        LLVM_JAVA_GETOBJECTCLASS, PointerType::get(VTableInfo::VTableTy),
        objBase->getType(), NULL);
      Value* vtable = new CallInst(f, objBase, TMP, getBBAt(bcI));
      vtable = new CastInst(vtable, PointerType::get(vi.vtable->getType()),
                            TMP, getBBAt(bcI));
      vtable = new LoadInst(vtable, className + "<vtable>", getBBAt(bcI));
      std::vector<Value*> indices(1, ConstantUInt::get(Type::UIntTy, 0));
      assert(vi.m2iMap.find(methodDescr) != vi.m2iMap.end() &&
             "could not find slot for virtual function!");
      unsigned vSlot = vi.m2iMap.find(methodDescr)->second;
      indices.push_back(ConstantUInt::get(Type::UIntTy, vSlot));
      Value* vfunPtr =
        new GetElementPtrInst(vtable, indices, TMP, getBBAt(bcI));
      Value* vfun = new LoadInst(vfunPtr, methodDescr, getBBAt(bcI));

      makeCall(vfun, getBBAt(bcI));
    }

    void do_invokespecial(unsigned bcI, unsigned index) {
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
        FunctionType* funcType =
          cast<FunctionType>(getType(nameAndType->getDescriptor(), ci.type));
        Function* function = module_->getOrInsertFunction(funcName, funcType);
        toCompileFunctions_.insert(function);
        makeCall(function, getBBAt(bcI));
      }
      // otherwise we call the superclass' implementation of the method
      else {
        assert(0 && "not implemented");
      }
    }

    void do_invokestatic(unsigned bcI, unsigned index) {
      ConstantMethodRef* methodRef = cf_->getConstantMethodRef(index);
      ConstantNameAndType* nameAndType = methodRef->getNameAndType();

      std::string funcName =
        methodRef->getClass()->getName()->str() + '/' +
        nameAndType->getName()->str() +
        nameAndType->getDescriptor()->str();

      FunctionType* funcType =
        cast<FunctionType>(getType(nameAndType->getDescriptor()));
      Function* function = module_->getOrInsertFunction(funcName, funcType);
      toCompileFunctions_.insert(function);
      makeCall(function, getBBAt(bcI));
    }

    void do_invokeinterface(unsigned bcI, unsigned index) {
      ConstantMethodRef* methodRef = cf_->getConstantMethodRef(index);
      ConstantNameAndType* nameAndType = methodRef->getNameAndType();

      ClassFile* cf = ClassFile::get(methodRef->getClass()->getName()->str());
      const ClassInfo& ci = getClassInfo(cf);
      const VTableInfo& vi = getVTableInfo(cf);

      const std::string& className = cf->getThisClass()->getName()->str();
      const std::string& methodDescr =
        nameAndType->getName()->str() +
        nameAndType->getDescriptor()->str();

      Value* objRef = opStack_.top(); // do not pop
      objRef = new CastInst(objRef, PointerType::get(ci.type),
                            "this", getBBAt(bcI));
      Value* objBase = getField(bcI, cf, LLVM_JAVA_OBJECT_BASE, objRef);
      Function* f = module_->getOrInsertFunction(
        LLVM_JAVA_GETOBJECTCLASS, PointerType::get(VTableInfo::VTableTy),
        objBase->getType(), NULL);
      Value* vtable = new CallInst(f, objBase, TMP, getBBAt(bcI));
      // get the interfaces array of vtables
      std::vector<Value*> indices(2, ConstantUInt::get(Type::UIntTy, 0));
      indices.push_back(ConstantUInt::get(Type::UIntTy, 3));
      Value* interfaceVTables =
        new GetElementPtrInst(vtable, indices, TMP, getBBAt(bcI));
      interfaceVTables = new LoadInst(interfaceVTables, TMP, getBBAt(bcI));
      // get the actual interface vtable
      indices.resize(1);
      indices.push_back(ConstantUInt::get(Type::UIntTy, ci.interfaceIdx));
      Value* interfaceVTable =
        new GetElementPtrInst(vtable, indices, TMP, getBBAt(bcI));
      interfaceVTable =
        new CastInst(vtable, PointerType::get(VTableInfo::VTableTy),
                     TMP, getBBAt(bcI));
      interfaceVTable =
        new LoadInst(interfaceVTable, className + "<vtable>", getBBAt(bcI));
      // get the function pointer
      indices.resize(1);
      assert(vi.m2iMap.find(methodDescr) != vi.m2iMap.end() &&
             "could not find slot for virtual function!");
      unsigned vSlot = vi.m2iMap.find(methodDescr)->second;
      indices.push_back(ConstantUInt::get(Type::UIntTy, vSlot));
      Value* vfunPtr =
        new GetElementPtrInst(interfaceVTable, indices, TMP, getBBAt(bcI));
      Value* vfun = new LoadInst(vfunPtr, methodDescr, getBBAt(bcI));

      makeCall(vfun, getBBAt(bcI));
    }

    void do_new(unsigned bcI, unsigned index) {
      ConstantClass* classRef = cf_->getConstantClass(index);
      ClassFile* cf = ClassFile::get(classRef->getName()->str());
      const ClassInfo& ci = getClassInfo(cf);
      const VTableInfo& vi = getVTableInfo(cf);

      Value* objRef = new MallocInst(ci.type,
                                     ConstantUInt::get(Type::UIntTy, 0),
                                     TMP, getBBAt(bcI));
      Value* vtable = getField(bcI, cf, LLVM_JAVA_OBJECT_BASE, objRef);
      vtable = new CastInst(vtable, PointerType::get(vi.vtable->getType()),
                            TMP, getBBAt(bcI));
      vtable = new StoreInst(vi.vtable, vtable, getBBAt(bcI));
      opStack_.push(objRef);
    }

    void do_newarray(unsigned bcI, JType type) {
      assert(0 && "not implemented");
    }

    void do_anewarray(unsigned bcI, unsigned index) {
      assert(0 && "not implemented");
    }

    void do_arraylength(unsigned bcI) {
      assert(0 && "not implemented");
    }

    void do_athrow(unsigned bcI) {
      assert(0 && "not implemented");
    }

    void do_checkcast(unsigned bcI, unsigned index) {
      do_dup(bcI);
      do_instanceof(bcI, index);
      Value* r = opStack_.top(); opStack_.pop();
      Value* b = new SetCondInst(Instruction::SetEQ,
                                 r, ConstantSInt::get(Type::IntTy, 1),
                                 TMP, getBBAt(bcI));
      // FIXME: if b is false we must throw a ClassCast exception
    }

    void do_instanceof(unsigned bcI, unsigned index) {
      ConstantClass* classRef = cf_->getConstantClass(index);
      ClassFile* cf = ClassFile::get(classRef->getName()->str());
      const VTableInfo& vi = getVTableInfo(cf);

      Value* objRef = opStack_.top(); opStack_.pop();
      Value* objBase = getField(bcI, cf, LLVM_JAVA_OBJECT_BASE, objRef);
      Function* f = module_->getOrInsertFunction(
        LLVM_JAVA_ISINSTANCEOF, Type::IntTy,
        objBase->getType(), PointerType::get(VTableInfo::VTableTy), NULL);
      Value* r = new CallInst(f, objBase, vi.vtable, TMP, getBBAt(bcI));
      opStack_.push(r);
    }

    void do_monitorenter(unsigned bcI) {
      assert(0 && "not implemented");
    }

    void do_monitorexit(unsigned bcI) {
      assert(0 && "not implemented");
    }

    void do_multianewarray(unsigned bcI,
                           unsigned index,
                           unsigned dims) {
      assert(0 && "not implemented");
    }
  };

  unsigned CompilerImpl::ClassInfo::InterfaceCount = 0;
  StructType* CompilerImpl::VTableInfo::VTableTy;
  StructType* CompilerImpl::VTableInfo::TypeInfoTy;

} } } // namespace llvm::Java::

Compiler::Compiler()
  : compilerImpl_(new CompilerImpl())
{

}

Compiler::~Compiler()
{
  delete compilerImpl_;
}

void Compiler::compile(Module& m, const std::string& className)
{
  DEBUG(std::cerr << "Compiling class: " << className << '\n');

  Function* main =
    compilerImpl_->compileMethod(m, className + "/main([Ljava/lang/String;)V");
  Function* javaMain = m.getOrInsertFunction
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
}

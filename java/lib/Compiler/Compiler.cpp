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
#include <llvm/Java/Bytecode.h>
#include <llvm/Java/BytecodeParser.h>
#include <llvm/Java/ClassFile.h>
#include <llvm/Constants.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Instructions.h>
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

#define LLVM_JAVA_ISINSTANCEOF  "llvm_java_IsInstanceOf"
#define LLVM_JAVA_GETOBJECTCLASS "llvm_java_GetObjectClass"
#define LLVM_JAVA_SETOBJECTCLASS "llvm_java_SetObjectClass"
#define LLVM_JAVA_THROW "llvm_java_Throw"

using namespace llvm;
using namespace llvm::Java;

namespace llvm { namespace Java { namespace {

  const std::string TMP("tmp");

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

  class Compiler : public BytecodeParser<Compiler> {
    Module& module_;
    ClassFile* cf_;
    std::auto_ptr<BasicBlockBuilder> bbBuilder_;
    std::list<BasicBlock*> bbWorkList_;
    typedef std::map<BasicBlock*, std::pair<Locals,OperandStack> > BBInfoMap;
    BBInfoMap bbInfoMap_;
    BasicBlock* currentBB_;
    Locals* currentLocals_;
    OperandStack* currentOpStack_;

    typedef SetVector<Function*> FunctionSet;
    FunctionSet toCompileFunctions_;

    /// This class containts the LLVM type that a class maps to and
    /// the max interface index of the interfaces this class
    /// implements or the interface index of this interface if this
    /// represents an interface. It also contains a map from fields to
    /// struct indices for this class (used to index into the class
    /// object).
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

    /// This class contains the vtable of a class, a vector with the
    /// vtables of its super classes (with the class higher in the
    /// hierarchy first). It also contains a map from methods to
    /// struct indices for this class (used to index into the vtable).
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
    Class2VTableInfoMap ac2viMap_;

  public:
    Compiler(Module& m)
      : module_(m) {
    }

  private:
    /// Given a llvm::Java::Constant returns a llvm::Constant.
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

    /// Given a JType returns the appropriate llvm::Type.
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
        if (descr[i] == '[') {
          do { ++i; } while (descr[i] == '[');
          getTypeHelper(descr, i, NULL);
          return PointerType::get(getObjectArrayInfo().type);
        }
        else if (descr[i] == 'L') {
          getTypeHelper(descr, i, NULL);
          return PointerType::get(getObjectArrayInfo().type);
        }
        else {
           return PointerType::get(
             getPrimitiveArrayInfo(getTypeHelper(descr, i, NULL)).type);
        }
        break;
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

    /// Initializes the class info map; in other words it adds the
    /// class info of java.lang.Object.
    void initializeClassInfoMap() {
      DEBUG(std::cerr << "Building ClassInfo for: java/lang/Object\n");
      ClassFile* cf = ClassFile::get("java/lang/Object");
      ClassInfo& ci = c2ciMap_[cf];

      assert(!ci.type && ci.f2iMap.empty() &&
	     "java/lang/Object ClassInfo should not be initialized!");
      ci.type = OpaqueType::get();

      std::vector<const Type*> elements;

      // Because this is java/lang/Object, we add the opaque
      // llvm_java_object_base type first.
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

    /// Initializes the VTableInfo map; in other words it adds the
    /// VTableInfo for java.lang.Object.
    void initializeVTableInfoMap() {
      DEBUG(std::cerr << "Building VTableInfo for: java/lang/Object\n");
      ClassFile* cf = ClassFile::get("java/lang/Object");
      VTableInfo& vi = c2viMap_[cf];

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

      // This is a static variable.
      VTableInfo::TypeInfoTy = StructType::get(elements);
      module_.addTypeName(LLVM_JAVA_OBJECT_TYPEINFO, VTableInfo::TypeInfoTy);
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

      const Methods& methods = cf->getMethods();

      const ClassInfo& ci = getClassInfo(cf);

      // Add member functions to the vtable.
      for (unsigned i = 0, e = methods.size(); i != e; ++i) {
	Method* method = methods[i];
	// The contructor is the only non-static method that is not
	// dynamically dispatched so we skip it.
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

    /// Returns the ClassInfo object associated with this classfile.
    const ClassInfo& getClassInfo(ClassFile* cf) {
      Class2ClassInfoMap::iterator it = c2ciMap_.lower_bound(cf);
      if (it != c2ciMap_.end() && it->first == cf)
	return it->second;

      const std::string& className = cf->getThisClass()->getName()->str();
      DEBUG(std::cerr << "Building ClassInfo for: " << className << '\n');
      ClassInfo& ci = c2ciMap_[cf];

      assert(!ci.type && ci.f2iMap.empty() &&
	     "got already initialized ClassInfo!");

      // Get the interface id.
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

    /// Creates a ClassInfo object for an array of the specified
    /// element type.
    ClassInfo buildArrayClassInfo(Type* elementTy) {
      ClassInfo arrayInfo;

      std::vector<const Type*> elements;
      elements.reserve(3);
      elements.push_back(getClassInfo(ClassFile::get("java/lang/Object")).type);
      elements.push_back(Type::UIntTy);
      arrayInfo.f2iMap.insert(std::make_pair("<length>", elements.size()));
      elements.push_back(ArrayType::get(elementTy, 0));
      arrayInfo.f2iMap.insert(std::make_pair("<data>", elements.size()));

      arrayInfo.type = StructType::get(elements);

      return arrayInfo;
    }

    const ClassInfo& getPrimitiveArrayInfo(Type* type) {
      if (Type::BoolTy == type) return getPrimitiveArrayInfo(BOOLEAN);
      else if (Type::UShortTy == type) return getPrimitiveArrayInfo(CHAR);
      else if (Type::FloatTy == type) return getPrimitiveArrayInfo(FLOAT);
      else if (Type::DoubleTy == type) return getPrimitiveArrayInfo(DOUBLE);
      else if (Type::SByteTy == type) return getPrimitiveArrayInfo(BYTE);
      else if (Type::ShortTy == type) return getPrimitiveArrayInfo(SHORT);
      else if (Type::IntTy == type) return getPrimitiveArrayInfo(INT);
      else if (Type::LongTy == type) return getPrimitiveArrayInfo(LONG);
      else abort();
    }

    /// Returns the ClassInfo object associated with an array of the
    /// specified element type.
    const ClassInfo& getPrimitiveArrayInfo(JType type) {
      switch (type) {
      case BOOLEAN: {
	static ClassInfo arrayInfo = buildArrayClassInfo(Type::BoolTy);
	return arrayInfo;
      }
      case CHAR: {
	static ClassInfo arrayInfo = buildArrayClassInfo(Type::UShortTy);
	return arrayInfo;
      }
      case FLOAT: {
	static ClassInfo arrayInfo = buildArrayClassInfo(Type::FloatTy);
	return arrayInfo;
      }
      case DOUBLE: {
	static ClassInfo arrayInfo = buildArrayClassInfo(Type::DoubleTy);
	return arrayInfo;
      }
      case BYTE: {
	static ClassInfo arrayInfo = buildArrayClassInfo(Type::SByteTy);
	return arrayInfo;
      }
      case SHORT: {
	static ClassInfo arrayInfo = buildArrayClassInfo(Type::ShortTy);
	return arrayInfo;
      }
      case INT: {
	static ClassInfo arrayInfo = buildArrayClassInfo(Type::IntTy);
	return arrayInfo;
      }
      case LONG: {
	static ClassInfo arrayInfo = buildArrayClassInfo(Type::LongTy);
	return arrayInfo;
      }
      }
    }

    /// Returns the ClassInfo object associated with an array of the
    /// specified element type.
    const ClassInfo& getObjectArrayInfo() {
      static ClassInfo arrayInfo = buildArrayClassInfo(
	PointerType::get(
          getClassInfo(ClassFile::get("java/lang/Object")).type));
      return arrayInfo;
    }

    /// Builds the super classes' vtable array for this classfile and
    /// its corresponding VTable. The most generic class goes first in
    /// the array.
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

    /// Builds an interface VTable for the specified <class,interface>
    /// pair.
    llvm::Constant* buildInterfaceVTable(ClassFile* cf, ClassFile* interface) {

      const VTableInfo& classVI = getVTableInfo(cf);
      const VTableInfo& interfaceVI = getVTableInfo(interface);
      const Methods& methods = interface->getMethods();

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

    /// Builds the interfaces vtable array for this classfile and its
    /// corresponding VTableInfo. If this classfile is an interface we
    /// return a pointer to 0xFFFFFFFF.
    std::pair<int, llvm::Constant*>
    buildInterfacesVTables(ClassFile* cf, const VTableInfo& vi) {
      // If this is an interface then we are not implementing any
      // interfaces so the lastInterface field is our index and the
      // pointer to the array of interface vtables is an all-ones
      // value.
      if (cf->isInterface())
	return std::make_pair(
	  getClassInfo(cf).interfaceIdx,
	  ConstantExpr::getCast(
	    ConstantIntegral::getAllOnesValue(Type::LongTy),
	    PointerType::get(PointerType::get(VTableInfo::VTableTy))));

      // Otherwise we must fill in the interfaces vtables array. For
      // each implemented vtable we insert a pointer to the
      // <class,interface> vtable for this class. Note that we only
      // fill in up to the highest index of the implemented
      // interfaces.
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

    /// Given the classfile and its corresponding VTableInfo,
    /// construct the typeinfo constant for it.
    llvm::Constant* buildClassTypeInfo(ClassFile* cf, const VTableInfo& vi) {
      std::vector<llvm::Constant*> typeInfoInit;

      unsigned depth;
      llvm::Constant* superClassesVTables;
      tie(depth, superClassesVTables) = buildSuperClassesVTables(cf, vi);

      // The depth (java/lang/Object has depth 0).
      typeInfoInit.push_back(ConstantSInt::get(Type::IntTy, depth));
      // The super classes' vtables.
      typeInfoInit.push_back(superClassesVTables);

      int lastInterface;
      llvm::Constant* interfacesVTables;
      tie(lastInterface, interfacesVTables) = buildInterfacesVTables(cf, vi);

      // The last interface index or the interface index if this is an
      // interface.
      typeInfoInit.push_back(ConstantSInt::get(Type::IntTy, lastInterface));
      // The interfaces' vtables.
      typeInfoInit.push_back(interfacesVTables);

      return ConstantStruct::get(VTableInfo::TypeInfoTy, typeInfoInit);
    }

    /// Returns the VTableInfo associated with this classfile.
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

      // Copy the super vtables array.
      vi.superVtables.reserve(superVI.superVtables.size() + 1);
      vi.superVtables.push_back(superVI.vtable);
      std::copy(superVI.superVtables.begin(), superVI.superVtables.end(),
		std::back_inserter(vi.superVtables));

      // Copy all the constants from the super class' vtable.
      assert(superVI.vtable && "No vtable found for super class!");
      ConstantStruct* superInit =
	cast<ConstantStruct>(superVI.vtable->getInitializer());
      std::vector<llvm::Constant*> init(superInit->getNumOperands());
      // Use a null typeinfo struct for now.
      init[0] = llvm::Constant::getNullValue(VTableInfo::TypeInfoTy);
      // Fill in the function pointers as they are in the super
      // class. Overriden methods will be replaced later.
      for (unsigned i = 1, e = superInit->getNumOperands(); i != e; ++i)
	init[i] = superInit->getOperand(i);
      vi.m2iMap = superVI.m2iMap;

      // Add member functions to the vtable.
      const Methods& methods = cf->getMethods();

      for (unsigned i = 0, e = methods.size(); i != e; ++i) {
	Method* method = methods[i];
	// The contructor is the only non-static method that is not
	// dynamically dispatched so we skip it.
	if (!method->isStatic() && method->getName()->str() != "<init>") {
	  const std::string& methodDescr =
	    method->getName()->str() + method->getDescriptor()->str();

	  std::string funcName = className + '/' + methodDescr;

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
      // to exist in order to build it.
      init[0] = buildClassTypeInfo(cf, vi);
      vi.vtable->setInitializer(ConstantStruct::get(init));

      DEBUG(std::cerr << "Built VTableInfo for: " << className << '\n');
      return vi;
    }

    VTableInfo buildArrayVTableInfo(Type* elementTy) {
      assert(elementTy->isPrimitiveType() &&
	     "This should not be called for arrays of non-primitive types");

      VTableInfo vi;
      const VTableInfo& superVI =
	getVTableInfo(ClassFile::get("java/lang/Object"));

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
	elementTy->getDescription() + "<vtable>";

      llvm::Constant* vtable = ConstantStruct::get(init);
      module_.addTypeName(globalName, vtable->getType());
      vi.vtable = new GlobalVariable(vtable->getType(),
				     true,
				     GlobalVariable::ExternalLinkage,
				     vtable,
				     globalName,
				     &module_);

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
	elementTy->getDescription() + "<superclassesvtables>",
	&module_);

      typeInfoInit.push_back(
	ConstantExpr::getGetElementPtr(
	  vtablesArray,
	  std::vector<llvm::Constant*>(2, ConstantUInt::get(Type::UIntTy, 0))));
      typeInfoInit.push_back(ConstantSInt::get(Type::IntTy, 0));
      typeInfoInit.push_back(
	llvm::Constant::getNullValue(
	  PointerType::get(PointerType::get(VTableInfo::VTableTy))));

      init[0] = ConstantStruct::get(VTableInfo::TypeInfoTy, typeInfoInit);
      vi.vtable->setInitializer(ConstantStruct::get(init));

      return vi;
    }

    const VTableInfo& getPrimitiveArrayVTableInfo(Type* type) {
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
	static VTableInfo arrayInfo = buildArrayVTableInfo(Type::BoolTy);
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
    }

    /// Initializes the VTableInfo map for object arrays; in other
    /// words it adds the VTableInfo for java.lang.Object[].
    void initializeObjectArrayVTableInfoMap() {
      DEBUG(std::cerr << "Building VTableInfo for: java/lang/Object[]\n");
      ClassFile* cf = ClassFile::get("java/lang/Object");
      VTableInfo& vi = ac2viMap_[cf];
      assert(!vi.vtable && vi.m2iMap.empty() &&
	     "java/lang/Object[] VTableInfo should not be initialized!");

      const VTableInfo& javaLangObjectVI =
        getVTableInfo(ClassFile::get("java/lang/Object"));
      vi.superVtables.reserve(1);
      vi.superVtables.push_back(javaLangObjectVI.vtable);

      std::vector<llvm::Constant*> init;

      // This is java/lang/Object[] so we must add a
      // llvm_java_object_typeinfo struct first.

      // depth
      init.push_back(llvm::ConstantSInt::get(Type::IntTy, 1));
      // superclasses vtable pointers
      ArrayType* vtablesArrayTy =
	ArrayType::get(PointerType::get(VTableInfo::VTableTy), 1);

      GlobalVariable* vtablesArray = new GlobalVariable(
	vtablesArrayTy,
	true,
	GlobalVariable::ExternalLinkage,
	ConstantArray::get(vtablesArrayTy, vi.superVtables),
	"java/lang/Object[]<superclassesvtables>",
	&module_);
      init.push_back(ConstantExpr::getGetElementPtr(
                       vtablesArray,
                       std::vector<llvm::Constant*>(2, ConstantUInt::get(Type::UIntTy, 0))));

      // last interface index
      init.push_back(llvm::ConstantSInt::get(Type::IntTy, -1));
      // interfaces vtable pointers
      init.push_back(
        llvm::Constant::getNullValue(
          PointerType::get(PointerType::get(VTableInfo::VTableTy))));

      llvm::Constant* typeInfoInit =
	ConstantStruct::get(VTableInfo::TypeInfoTy, init);

      // Now that we have both the type and initializer for the
      // llvm_java_object_typeinfo struct we can start adding the
      // function pointers.
      ConstantStruct* superInit =
	cast<ConstantStruct>(javaLangObjectVI.vtable->getInitializer());

      init.clear();
      init.resize(superInit->getNumOperands());
      // Add the typeinfo block for this class.
      init[0] = typeInfoInit;

      // Fill in the function pointers as they are in
      // java/lang/Object. There are no overriden methods.
      for (unsigned i = 1, e = superInit->getNumOperands(); i != e; ++i)
	init[i] = superInit->getOperand(i);
      vi.m2iMap = javaLangObjectVI.m2iMap;

      llvm::Constant* vtable = ConstantStruct::get(init);
      module_.addTypeName("java/lang/Object[]<vtable>", vtable->getType());

      vi.vtable = new GlobalVariable(VTableInfo::VTableTy,
				     true, GlobalVariable::ExternalLinkage,
                                     vtable,
				     "java/lang/Object[]<vtable>",
				     &module_);
      DEBUG(std::cerr << "Built VTableInfo for: java/lang/Object[]\n");
    }

    const VTableInfo& getObjectArrayVTableInfo(ClassFile* cf) {
      Class2VTableInfoMap::iterator it = ac2viMap_.lower_bound(cf);
      if (it != ac2viMap_.end() && it->first == cf)
        return it->second;

      const std::string& className = cf->getThisClass()->getName()->str();
      DEBUG(std::cerr << "Building VTableInfo for: " << className << "[]\n");
      VTableInfo& vi = ac2viMap_[cf];

      assert(!vi.vtable && vi.m2iMap.empty() &&
	     "got already initialized VTableInfo!");

      ConstantClass* super = cf->getSuperClass();
      assert(super && "Class does not have superclass!");
      const VTableInfo& superVI =
	getVTableInfo(ClassFile::get(super->getName()->str()));

      // Copy the super vtables array.
      vi.superVtables.reserve(superVI.superVtables.size() + 1);
      vi.superVtables.push_back(superVI.vtable);
      std::copy(superVI.superVtables.begin(), superVI.superVtables.end(),
		std::back_inserter(vi.superVtables));

      // Copy all the constants from the super class' vtable.
      assert(superVI.vtable && "No vtable found for super class!");
      ConstantStruct* superInit =
	cast<ConstantStruct>(superVI.vtable->getInitializer());
      std::vector<llvm::Constant*> init(superInit->getNumOperands());
      // Use a null typeinfo struct for now.
      init[0] = llvm::Constant::getNullValue(VTableInfo::TypeInfoTy);
      // Fill in the function pointers as they are in the super
      // class. There are no overriden methods.
      for (unsigned i = 0, e = superInit->getNumOperands(); i != e; ++i)
	init[i] = superInit->getOperand(i);
      vi.m2iMap = superVI.m2iMap;

#ifndef NDEBUG
      for (unsigned i = 0, e = init.size(); i != e; ++i)
	assert(init[i] && "No elements in the initializer should be NULL!");
#endif

      const std::string& globalName = className + "[]<vtable>";

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
      // to exist in order to build it.
      std::vector<llvm::Constant*> typeInfoInit;
      typeInfoInit.reserve(4);
      // depth
      typeInfoInit.push_back(
        llvm::ConstantSInt::get(Type::IntTy, vi.superVtables.size()));
      // superclasses vtable pointers
      ArrayType* vtablesArrayTy =
	ArrayType::get(PointerType::get(VTableInfo::VTableTy),
                       vi.superVtables.size());

      GlobalVariable* vtablesArray = new GlobalVariable(
	vtablesArrayTy,
	true,
	GlobalVariable::ExternalLinkage,
	ConstantArray::get(vtablesArrayTy, vi.superVtables),
	className + "[]<superclassesvtables>",
	&module_);

      typeInfoInit.push_back(ConstantExpr::getGetElementPtr(
                               vtablesArray,
                               std::vector<llvm::Constant*>(2, ConstantUInt::get(Type::UIntTy, 0))));
      // last interface index
      typeInfoInit.push_back(llvm::ConstantSInt::get(Type::IntTy, -1));
      // interfaces vtable pointers
      typeInfoInit.push_back(
        llvm::Constant::getNullValue(
          PointerType::get(PointerType::get(VTableInfo::VTableTy))));

      init[0] = ConstantStruct::get(VTableInfo::TypeInfoTy, typeInfoInit);
      vi.vtable->setInitializer(ConstantStruct::get(init));

      DEBUG(std::cerr << "Built VTableInfo for: " << className << "[]\n");
      return vi;

    }

    /// Emits the necessary code to get a pointer to a static field of
    /// an object.
    GlobalVariable* getStaticField(unsigned index) {
      ConstantFieldRef* fieldRef = cf_->getConstantFieldRef(index);
      ConstantNameAndType* nameAndType = fieldRef->getNameAndType();

      // Get ClassInfo for class owning the field - this will force
      // the globals to be initialized.
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

    /// Emits the necessary code to get a field from the passed
    /// pointer to an object.
    Value* getField(unsigned index, Value* ptr) {
      ConstantFieldRef* fieldRef = cf_->getConstantFieldRef(index);
      ConstantNameAndType* nameAndType = fieldRef->getNameAndType();
      ClassFile* cf = ClassFile::get(fieldRef->getClass()->getName()->str());
      return getField(cf, nameAndType->getName()->str(), ptr);
    }

    /// Emits the necessary code to get a field from the passed
    /// pointer to an object.
    Value* getField(ClassFile* cf, const std::string& fieldName, Value* ptr) {
      // Cast ptr to correct type.
      ptr = new CastInst(ptr, PointerType::get(getClassInfo(cf).type),
			 TMP, currentBB_);

      // Deref pointer.
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

      return new GetElementPtrInst(ptr, indices, TMP, currentBB_);
    }

    /// Compiles the passed method only (it does not compile any
    /// callers or methods of objects it creates).
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

      bbInfoMap_.clear();
      bbBuilder_.reset(new BasicBlockBuilder(function, codeAttr));

      // Put arguments into locals.
      Locals locals(codeAttr->getMaxLocals());

      unsigned index = 0;
      for (Function::aiterator
	     a = function->abegin(), ae = function->aend(); a != ae; ++a) {
	locals.store(index, a, &function->getEntryBlock());
	index += isTwoSlotType(a->getType()) ? 2 : 1;
      }
      // For the entry block the operand stack is empty and the locals
      // contain the arguments to the function.
      bbInfoMap_.insert(std::make_pair(&function->getEntryBlock(),
				       std::make_pair(locals, OperandStack())));

      // Insert the entry block to the work list.
      bbWorkList_.push_back(&function->getEntryBlock());

      // Process the work list until we compile the whole function.
      while (!bbWorkList_.empty()) {
	currentBB_ = bbWorkList_.front();
	bbWorkList_.pop_front();

	BBInfoMap::iterator bbInfo = bbInfoMap_.find(currentBB_);
	assert(bbInfo != bbInfoMap_.end() &&
	       "Unknown entry operand stack and locals for basic block in "
	       "work list!");

	currentLocals_ = &bbInfo->second.first;
	currentOpStack_ = &bbInfo->second.second;

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
	// entry operand stack and locals, do so, and add it to the
	// work list. If a successor already has an entry operand
	// stack and locals we assume the computation was correct and
	// do not add it to the work list.
	for (succ_iterator
	       SI = succ_begin(currentBB_), SE = succ_end(currentBB_);
	     SI != SE; ++SI) {
	  BasicBlock* Succ = *SI;
	  BBInfoMap::iterator bbSuccInfo = bbInfoMap_.lower_bound(Succ);
	  if (bbSuccInfo == bbInfoMap_.end() || bbSuccInfo->first != Succ) {
	    bbInfoMap_.insert(bbSuccInfo,
			      std::make_pair(Succ,
					     std::make_pair(*currentLocals_,
							    *currentOpStack_)));
	    bbWorkList_.push_back(Succ);
	  }
	}
      }

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
        // Create the global variables of this class.
        const Fields& fields = classfile->getFields();
        for (unsigned i = 0, e = fields.size(); i != e; ++i) {
          Field* field = fields[i];
          if (field->isStatic()) {
            Type* globalTy = getType(field->getDescriptor());
            llvm::Constant* init = NULL;
            if (ConstantValueAttribute* cv = field->getConstantValueAttribute())
              init =
                ConstantExpr::getCast(getConstant(cv->getValue()), globalTy);

            std::string globalName =
              classfile->getThisClass()->getName()->str() + '/' +
              field->getName()->str();
            DEBUG(std::cerr << "Adding global: " << globalName << '\n');
            new GlobalVariable(globalTy,
                               field->isFinal(),
                               (field->isPrivate() & bool(init) ?
                                GlobalVariable::InternalLinkage :
                                GlobalVariable::ExternalLinkage),
                               init,
                               globalName,
                               &module_);
          }
        }

        // Call its class initialization method if it exists.
        if (const Method* method = classfile->getMethod("<clinit>()V")) {
          std::string name = classfile->getThisClass()->getName()->str();
          name += '/';
          name += method->getName()->str();
          name += method->getDescriptor()->str();

          Function* hook = module_.getOrInsertFunction(LLVM_JAVA_STATIC_INIT,
                                                       Type::VoidTy, 0);
          Function* init = module_.getOrInsertFunction(name, Type::VoidTy, 0);

          // Insert a call to it right before the terminator of the only
          // basic block in llvm_java_static_init.
          bool inserted =  toCompileFunctions_.insert(init);
          assert(inserted && "Class initialization method already called!");
          assert(hook->front().getTerminator() &&
                 LLVM_JAVA_STATIC_INIT " should have a terminator!");
          new CallInst(init, "", hook->front().getTerminator());
        }
      }
    }

    /// Returns the llvm::Function corresponding to the specified
    /// llvm::Java::Method.
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

    /// Returns the llvm::Java::Method given a <class,method>
    /// descriptor.
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
    /// Compiles the specified method given a <class,method>
    /// descriptor and the transitive closure of all methods
    /// (possibly) called by it.
    Function* compileMethod(const std::string& classMethodDesc) {
      // Initialize the static initializer function.
      Function* staticInit =
	module_.getOrInsertFunction(LLVM_JAVA_STATIC_INIT, Type::VoidTy, 0);
      BasicBlock* staticInitBB = new BasicBlock("entry", staticInit);
      new ReturnInst(NULL, staticInitBB);

      // Initialize type maps and vtable globals.
      initializeClassInfoMap();
      initializeVTableInfoMap();
      initializeObjectArrayVTableInfoMap();

      // Create the method requested.
      Function* function = getFunction(getMethod(classMethodDesc));
      toCompileFunctions_.insert(function);
      // Compile the transitive closure of methods called by this method.
      for (unsigned i = 0; i != toCompileFunctions_.size(); ++i) {
	Function* f = toCompileFunctions_[i];
	compileMethodOnly(f->getName());
      }

      return function;
    }

    void do_aconst_null() {
      ClassFile* root = ClassFile::get("java/lang/Object");
      currentOpStack_->push(llvm::Constant::getNullValue(
			      PointerType::get(getClassInfo(root).type)),
			    currentBB_);
    }

    void do_iconst(int value) {
      currentOpStack_->push(ConstantSInt::get(Type::IntTy, value), currentBB_);
    }

    void do_lconst(long long value) {
      currentOpStack_->push(ConstantSInt::get(Type::LongTy, value), currentBB_);
    }

    void do_fconst(float value) {
      currentOpStack_->push(ConstantFP::get(Type::FloatTy, value), currentBB_);
    }

    void do_dconst(double value) {
      currentOpStack_->push(ConstantFP::get(Type::DoubleTy, value), currentBB_);
    }

    void do_ldc(unsigned index) {
      Constant* c = cf_->getConstant(index);
      assert(getConstant(c) && "Java constant not handled!");
      currentOpStack_->push(getConstant(c), currentBB_);
    }

    void do_ldc2(unsigned index) {
      do_ldc(index);
    }

    void do_iload(unsigned index) { do_load_common(index); }
    void do_lload(unsigned index) { do_load_common(index); }
    void do_fload(unsigned index) { do_load_common(index); }
    void do_dload(unsigned index) { do_load_common(index); }
    void do_aload(unsigned index) { do_load_common(index); }

    void do_load_common(unsigned index) {
      Value* val = currentLocals_->load(index, currentBB_);
      currentOpStack_->push(val, currentBB_);
    }

    void do_iaload() { do_aload_common(); }
    void do_laload() { do_aload_common(); }
    void do_faload() { do_aload_common(); }
    void do_daload() { do_aload_common(); }
    void do_aaload() {
      do_aload_common();
      do_cast_common(
        PointerType::get(
          getClassInfo(ClassFile::get("java/lang/Object")).type));
    }
    void do_baload() { do_aload_common(); do_cast_common(Type::IntTy); }
    void do_caload() { do_aload_common(); do_cast_common(Type::IntTy); }
    void do_saload() { do_aload_common(); do_cast_common(Type::IntTy); }

    void do_aload_common() {
      Value* index = currentOpStack_->pop(currentBB_);
      Value* arrayRef = currentOpStack_->pop(currentBB_);

      std::vector<Value*> indices;
      indices.reserve(3);
      indices.push_back(ConstantUInt::get(Type::UIntTy, 0));
      indices.push_back(ConstantUInt::get(Type::UIntTy, 2));
      indices.push_back(index);
      Value* elementPtr =
	new GetElementPtrInst(arrayRef, indices, TMP, currentBB_);
      Value* result = new LoadInst(elementPtr, TMP, currentBB_);
      currentOpStack_->push(result, currentBB_);
    }

    void do_istore(unsigned index) { do_store_common(index); }
    void do_lstore(unsigned index) { do_store_common(index); }
    void do_fstore(unsigned index) { do_store_common(index); }
    void do_dstore(unsigned index) { do_store_common(index); }
    void do_astore(unsigned index) { do_store_common(index); }

    void do_store_common(unsigned index) {
      Value* val = currentOpStack_->pop(currentBB_);
      currentLocals_->store(index, val, currentBB_);
    }

    void do_iastore() { do_astore_common(); }
    void do_lastore() { do_astore_common(); }
    void do_fastore() { do_astore_common(); }
    void do_dastore() { do_astore_common(); }
    void do_aastore() {
      do_cast_common(
        PointerType::get(
          getClassInfo(ClassFile::get("java/lang/Object")).type));
      do_astore_common();
    }
    void do_bastore() { do_cast_common(Type::SByteTy); do_astore_common(); }
    void do_castore() { do_cast_common(Type::UShortTy); do_astore_common(); }
    void do_sastore() { do_cast_common(Type::ShortTy); do_astore_common(); }

    void do_astore_common() {
      Value* value = currentOpStack_->pop(currentBB_);
      Value* index = currentOpStack_->pop(currentBB_);
      Value* arrayRef = currentOpStack_->pop(currentBB_);

      arrayRef->dump();

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
      currentOpStack_->pop(currentBB_);
    }

    void do_pop2() {
      Value* v1 = currentOpStack_->pop(currentBB_);
      if (isOneSlotValue(v1))
	currentOpStack_->pop(currentBB_);
    }

    void do_dup() {
      Value* val = currentOpStack_->pop(currentBB_);
      currentOpStack_->push(val, currentBB_);
      currentOpStack_->push(val, currentBB_);
    }

    void do_dup_x1() {
      Value* v1 = currentOpStack_->pop(currentBB_);
      Value* v2 = currentOpStack_->pop(currentBB_);
      currentOpStack_->push(v1, currentBB_);
      currentOpStack_->push(v2, currentBB_);
      currentOpStack_->push(v1, currentBB_);
    }

    void do_dup_x2() {
      Value* v1 = currentOpStack_->pop(currentBB_);
      Value* v2 = currentOpStack_->pop(currentBB_);
      if (isOneSlotValue(v2)) {
	Value* v3 = currentOpStack_->pop(currentBB_);
	currentOpStack_->push(v1, currentBB_);
	currentOpStack_->push(v3, currentBB_);
	currentOpStack_->push(v2, currentBB_);
	currentOpStack_->push(v1, currentBB_);
      }
      else {
	currentOpStack_->push(v1, currentBB_);
	currentOpStack_->push(v2, currentBB_);
	currentOpStack_->push(v1, currentBB_);
      }
    }

    void do_dup2() {
      Value* v1 = currentOpStack_->pop(currentBB_);
      if (isOneSlotValue(v1)) {
	Value* v2 = currentOpStack_->pop(currentBB_);
	currentOpStack_->push(v2, currentBB_);
	currentOpStack_->push(v1, currentBB_);
	currentOpStack_->push(v2, currentBB_);
	currentOpStack_->push(v1, currentBB_);
      }
      else {
	currentOpStack_->push(v1, currentBB_);
	currentOpStack_->push(v1, currentBB_);
      }
    }

    void do_dup2_x1() {
      Value* v1 = currentOpStack_->pop(currentBB_);
      Value* v2 = currentOpStack_->pop(currentBB_);
      if (isOneSlotValue(v1)) {
	Value* v3 = currentOpStack_->pop(currentBB_);
	currentOpStack_->push(v2, currentBB_);
	currentOpStack_->push(v1, currentBB_);
	currentOpStack_->push(v3, currentBB_);
	currentOpStack_->push(v2, currentBB_);
	currentOpStack_->push(v1, currentBB_);
      }
      else {
	currentOpStack_->push(v1, currentBB_);
	currentOpStack_->push(v2, currentBB_);
	currentOpStack_->push(v1, currentBB_);
      }
    }

    void do_dup2_x2() {
      Value* v1 = currentOpStack_->pop(currentBB_);
      Value* v2 = currentOpStack_->pop(currentBB_);
      if (isOneSlotValue(v1)) {
	Value* v3 = currentOpStack_->pop(currentBB_);
	if (isOneSlotValue(v3)) {
	  Value* v4 = currentOpStack_->pop(currentBB_);
	  currentOpStack_->push(v2, currentBB_);
	  currentOpStack_->push(v1, currentBB_);
	  currentOpStack_->push(v4, currentBB_);
	  currentOpStack_->push(v3, currentBB_);
	  currentOpStack_->push(v2, currentBB_);
	  currentOpStack_->push(v1, currentBB_);
	}
	else {
	  currentOpStack_->push(v2, currentBB_);
	  currentOpStack_->push(v1, currentBB_);
	  currentOpStack_->push(v3, currentBB_);
	  currentOpStack_->push(v2, currentBB_);
	  currentOpStack_->push(v1, currentBB_);
	}
      }
      else {
	if (isOneSlotValue(v2)) {
	  Value* v3 = currentOpStack_->pop(currentBB_);
	  currentOpStack_->push(v1, currentBB_);
	  currentOpStack_->push(v3, currentBB_);
	  currentOpStack_->push(v2, currentBB_);
	  currentOpStack_->push(v1, currentBB_);
	}
	else {
	  currentOpStack_->push(v1, currentBB_);
	  currentOpStack_->push(v2, currentBB_);
	  currentOpStack_->push(v1, currentBB_);
	}
      }
    }

    void do_swap() {
      Value* v1 = currentOpStack_->pop(currentBB_);
      Value* v2 = currentOpStack_->pop(currentBB_);
      currentOpStack_->push(v1, currentBB_);
      currentOpStack_->push(v2, currentBB_);
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
      Value* v1 = currentOpStack_->pop(currentBB_);
      Value* r = BinaryOperator::createNeg(v1, TMP, currentBB_);
      currentOpStack_->push(r, currentBB_);
    }

    void do_ishl() { do_shift_common(Instruction::Shl); }
    void do_lshl() { do_shift_common(Instruction::Shl); }
    void do_ishr() { do_shift_common(Instruction::Shr); }
    void do_lshr() { do_shift_common(Instruction::Shr); }

    void do_iushr() { do_shift_unsigned_common(); }
    void do_lushr() { do_shift_unsigned_common(); }

    void do_shift_unsigned_common() {
      // Cast value to be shifted into its unsigned version.
      do_swap();
      Value* v = currentOpStack_->pop(currentBB_);
      v = new CastInst(v, v->getType()->getUnsignedVersion(), TMP, currentBB_);
      currentOpStack_->push(v, currentBB_);
      do_swap();

      do_shift_common(Instruction::Shr);

      v = currentOpStack_->pop(currentBB_);
      // Cast shifted value back to its original signed version.
      v = new CastInst(v, v->getType()->getSignedVersion(), TMP, currentBB_);
      currentOpStack_->push(v, currentBB_);
    }

    void do_shift_common(Instruction::OtherOps op) {
      Value* a = currentOpStack_->pop(currentBB_);
      Value* v = currentOpStack_->pop(currentBB_);
      a = new CastInst(a, Type::UByteTy, TMP, currentBB_);
      Value* r = new ShiftInst(op, v, a, TMP, currentBB_);
      currentOpStack_->push(r, currentBB_);
    }

    void do_iand() { do_binary_op_common(Instruction::And); }
    void do_land() { do_binary_op_common(Instruction::And); }
    void do_ior() { do_binary_op_common(Instruction::Or); }
    void do_lor() { do_binary_op_common(Instruction::Or); }
    void do_ixor() { do_binary_op_common(Instruction::Xor); }
    void do_lxor() { do_binary_op_common(Instruction::Xor); }

    void do_binary_op_common(Instruction::BinaryOps op) {
      Value* v2 = currentOpStack_->pop(currentBB_);
      Value* v1 = currentOpStack_->pop(currentBB_);
      Value* r = BinaryOperator::create(op, v1, v2, TMP, currentBB_);
      currentOpStack_->push(r, currentBB_);
    }

    void do_iinc(unsigned index, int amount) {
      Value* v = currentLocals_->load(index, currentBB_);
      Value* a = ConstantSInt::get(Type::IntTy, amount);
      BinaryOperator::createAdd(v, a, TMP, currentBB_);
      currentLocals_->store(index, v, currentBB_);
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
    void do_i2b() { do_truncate_common(Type::SByteTy); }
    void do_i2c() { do_truncate_common(Type::UShortTy); }
    void do_i2s() { do_truncate_common(Type::ShortTy); }

    void do_cast_common(Type* type) {
      Value* v1 = currentOpStack_->pop(currentBB_);
      v1 = new CastInst(v1, type, TMP, currentBB_);
      currentOpStack_->push(v1, currentBB_);
    }

    void do_truncate_common(Type* type) {
      Value* v1 = currentOpStack_->pop(currentBB_);
      v1 = new CastInst(v1, type, TMP, currentBB_);
      v1 = new CastInst(v1, Type::IntTy, TMP, currentBB_);
      currentOpStack_->push(v1, currentBB_);
    }

    void do_lcmp() {
      Value* v2 = currentOpStack_->pop(currentBB_);
      Value* v1 = currentOpStack_->pop(currentBB_);
      Value* c = BinaryOperator::createSetGT(v1, v2, TMP, currentBB_);
      Value* r = new SelectInst(c, ConstantSInt::get(Type::IntTy, 1),
				ConstantSInt::get(Type::IntTy, 0), TMP,
				currentBB_);
      c = BinaryOperator::createSetLT(v1, v2, TMP, currentBB_);
      r = new SelectInst(c, ConstantSInt::get(Type::IntTy, -1), r, TMP,
			 currentBB_);
      currentOpStack_->push(r, currentBB_);
    }

    void do_fcmpl() { do_cmp_common(-1); }
    void do_dcmpl() { do_cmp_common(-1); }
    void do_fcmpg() { do_cmp_common(1); }
    void do_dcmpg() { do_cmp_common(1); }

    void do_cmp_common(int valueIfUnordered) {
      Value* v2 = currentOpStack_->pop(currentBB_);
      Value* v1 = currentOpStack_->pop(currentBB_);
      Value* c = BinaryOperator::createSetGT(v1, v2, TMP, currentBB_);
      Value* r = new SelectInst(c, ConstantSInt::get(Type::IntTy, 1),
				ConstantSInt::get(Type::IntTy, 0), TMP,
				currentBB_);
      c = BinaryOperator::createSetLT(v1, v2, TMP, currentBB_);
      r = new SelectInst(c, ConstantSInt::get(Type::IntTy, -1), r, TMP,
			 currentBB_);
      c = new CallInst(module_.getOrInsertFunction
		       ("llvm.isunordered",
			Type::BoolTy, v1->getType(), v2->getType(), 0),
		       v1, v2, TMP, currentBB_);
      r = new SelectInst(c, ConstantSInt::get(Type::IntTy, valueIfUnordered),
			 r, TMP, currentBB_);
      currentOpStack_->push(r, currentBB_);
    }

    void do_ifeq(unsigned t, unsigned f) {
      do_iconst(0);
      do_if_common(Instruction::SetEQ, t, f);
    }
    void do_ifne(unsigned t, unsigned f) {
      do_iconst(0);
      do_if_common(Instruction::SetNE, t, f);
    }
    void do_iflt(unsigned t, unsigned f) {
      do_iconst(0);
      do_if_common(Instruction::SetLT, t, f);
    }
    void do_ifge(unsigned t, unsigned f) {
      do_iconst(0);
      do_if_common(Instruction::SetGE, t, f);
    }
    void do_ifgt(unsigned t, unsigned f) {
      do_iconst(0);
      do_if_common(Instruction::SetGT, t, f);
    }
    void do_ifle(unsigned t, unsigned f) {
      do_iconst(0);
      do_if_common(Instruction::SetLE, t, f);
    }
    void do_if_icmpeq(unsigned t, unsigned f) {
      do_if_common(Instruction::SetEQ, t, f);
    }
    void do_if_icmpne(unsigned t, unsigned f) {
      do_if_common(Instruction::SetNE, t, f);
    }
    void do_if_icmplt(unsigned t, unsigned f) {
      do_if_common(Instruction::SetLT, t, f);
    }
    void do_if_icmpge(unsigned t, unsigned f) {
      do_if_common(Instruction::SetGE, t, f);
    }
    void do_if_icmpgt(unsigned t, unsigned f) {
      do_if_common(Instruction::SetGT, t, f);
    }
    void do_if_icmple(unsigned t, unsigned f) {
      do_if_common(Instruction::SetLE, t, f);
    }
    void do_if_acmpeq(unsigned t, unsigned f) {
      do_if_common(Instruction::SetEQ, t, f);
    }
    void do_if_acmpne(unsigned t, unsigned f) {
      do_if_common(Instruction::SetNE, t, f);
    }
    void do_ifnull(unsigned t, unsigned f) {
      do_aconst_null();
      do_if_common(Instruction::SetEQ, t, f);
    }
    void do_ifnonnull(unsigned t, unsigned f) {
      do_aconst_null();
      do_if_common(Instruction::SetEQ, t, f);
    }

    void do_if_common(Instruction::BinaryOps cc, unsigned t, unsigned f) {
      Value* v2 = currentOpStack_->pop(currentBB_);
      Value* v1 = currentOpStack_->pop(currentBB_);
      if (v1->getType() != v2->getType())
	v1 = new CastInst(v1, v2->getType(), TMP, currentBB_);
      Value* c = new SetCondInst(cc, v1, v2, TMP, currentBB_);
      new BranchInst(bbBuilder_->getBasicBlock(t),
		     bbBuilder_->getBasicBlock(f),
		     c, currentBB_);
    }

    void do_goto(unsigned target) {
      new BranchInst(bbBuilder_->getBasicBlock(target), currentBB_);
    }

    void do_ireturn() { do_return_common(); }
    void do_lreturn() { do_return_common(); }
    void do_freturn() { do_return_common(); }
    void do_dreturn() { do_return_common(); }
    void do_areturn() { do_return_common(); }

    void do_return_common() {
      Value* r = currentOpStack_->pop(currentBB_);
      new ReturnInst(r, currentBB_);
    }

    void do_return() {
      new ReturnInst(NULL, currentBB_);
    }

    void do_jsr(unsigned target) {
      assert(0 && "not implemented");
    }

    void do_ret(unsigned index) {
      assert(0 && "not implemented");
    }

    void do_switch(unsigned defTarget, const SwitchCases& sw) {
      Value* v = currentOpStack_->pop(currentBB_);
      SwitchInst* in =
	new SwitchInst(v, bbBuilder_->getBasicBlock(defTarget), currentBB_);
      for (unsigned i = 0, e = sw.size(); i != e; ++i)
	in->addCase(ConstantSInt::get(Type::IntTy, sw[i].first),
		    bbBuilder_->getBasicBlock(sw[i].second));
    }

    void do_getstatic(unsigned index) {
      Value* v = new LoadInst(getStaticField(index), TMP, currentBB_);
      currentOpStack_->push(v, currentBB_);
    }

    void do_putstatic(unsigned index) {
      Value* v = currentOpStack_->pop(currentBB_);
      Value* ptr = getStaticField(index);
      const Type* fieldTy = cast<PointerType>(ptr->getType())->getElementType();
      if (v->getType() != fieldTy)
	v = new CastInst(v, fieldTy, TMP, currentBB_);
      new StoreInst(v, ptr, currentBB_);
    }

    void do_getfield(unsigned index) {
      Value* p = currentOpStack_->pop(currentBB_);
      Value* v = new LoadInst(getField(index, p), TMP, currentBB_);
      currentOpStack_->push(v, currentBB_);
    }

    void do_putfield(unsigned index) {
      Value* v = currentOpStack_->pop(currentBB_);
      Value* p = currentOpStack_->pop(currentBB_);
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
	currentOpStack_->push(r, currentBB_);
      }
    }

    std::vector<Value*> getParams(FunctionType* funTy) {
      unsigned numParams = funTy->getNumParams();
      std::vector<Value*> params(numParams);
      while (numParams--) {
	Value* p = currentOpStack_->pop(currentBB_);
	params[numParams] =
	  p->getType() == funTy->getParamType(numParams) ?
	  p :
	  new CastInst(p, funTy->getParamType(numParams), TMP, currentBB_);
      }

      return params;
    }

    std::pair<const ClassInfo*, const VTableInfo*>
    getInfo(const std::string& className) {
      const ClassInfo* ci = NULL;
      const VTableInfo* vi = NULL;

      if (className[0] == '[') {
        if (className[1] == '[' || className[1] == 'L') {
          vi = &getObjectArrayVTableInfo(ClassFile::get("java/lang/Object"));
          ci = &getObjectArrayInfo();
        }
        else switch (className[1]) {
        case 'B':
          vi = &getPrimitiveArrayVTableInfo(Type::SByteTy);
          ci = &getPrimitiveArrayInfo(Type::SByteTy);
          break;
        case 'C':
          vi = &getPrimitiveArrayVTableInfo(Type::UShortTy);
          ci = &getPrimitiveArrayInfo(Type::UShortTy);
          break;
        case 'D':
          vi = &getPrimitiveArrayVTableInfo(Type::DoubleTy);
          ci = &getPrimitiveArrayInfo(Type::DoubleTy);
          break;
        case 'F':
          vi = &getPrimitiveArrayVTableInfo(Type::FloatTy);
          ci = &getPrimitiveArrayInfo(Type::FloatTy);
          break;
        case 'I':
          vi = &getPrimitiveArrayVTableInfo(Type::IntTy);
          ci = &getPrimitiveArrayInfo(Type::IntTy);
          break;
        case 'J':
          vi = &getPrimitiveArrayVTableInfo(Type::LongTy);
          ci = &getPrimitiveArrayInfo(Type::LongTy);
          break;
        case 'S':
          vi = &getPrimitiveArrayVTableInfo(Type::ShortTy);
          ci = &getPrimitiveArrayInfo(Type::ShortTy);
          break;
        case 'Z':
          vi = &getPrimitiveArrayVTableInfo(Type::BoolTy);
          ci = &getPrimitiveArrayInfo(Type::BoolTy);
          break;
        }
      }
      else {
        ClassFile* cf = ClassFile::get(className);
        vi = &getVTableInfo(cf);
        ci = &getClassInfo(cf);
      }

      return std::make_pair(ci, vi);
    }

    void do_invokevirtual(unsigned index) {
      ConstantMethodRef* methodRef = cf_->getConstantMethodRef(index);
      ConstantNameAndType* nameAndType = methodRef->getNameAndType();

      const std::string& className = methodRef->getClass()->getName()->str();

      const ClassInfo* ci = NULL;
      const VTableInfo* vi = NULL;
      tie(ci, vi) = getInfo(className);

      const std::string& methodDescr =
	nameAndType->getName()->str() +
	nameAndType->getDescriptor()->str();

      FunctionType* funTy =
	cast<FunctionType>(getType(nameAndType->getDescriptor(), ci->type));

      std::vector<Value*> params(getParams(funTy));

      Value* objRef = params.front();
      objRef = new CastInst(objRef, PointerType::get(ci->type),
			    "this", currentBB_);
      Value* objBase =
        new CastInst(objRef, ClassInfo::ObjectBaseTy, TMP, currentBB_);
      Function* f = module_.getOrInsertFunction(
	LLVM_JAVA_GETOBJECTCLASS, PointerType::get(VTableInfo::VTableTy),
	objBase->getType(), NULL);
      Value* vtable = new CallInst(f, objBase, TMP, currentBB_);
      vtable = new CastInst(vtable, PointerType::get(vi->vtable->getType()),
			    TMP, currentBB_);
      vtable = new LoadInst(vtable, className + "<vtable>", currentBB_);
      std::vector<Value*> indices(1, ConstantUInt::get(Type::UIntTy, 0));
      assert(vi->m2iMap.find(methodDescr) != vi->m2iMap.end() &&
	     "could not find slot for virtual function!");
      unsigned vSlot = vi->m2iMap.find(methodDescr)->second;
      indices.push_back(ConstantUInt::get(Type::UIntTy, vSlot));
      Value* vfunPtr =
	new GetElementPtrInst(vtable, indices, TMP, currentBB_);
      Value* vfun = new LoadInst(vfunPtr, methodDescr, currentBB_);

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

      FunctionType* funcTy =
	cast<FunctionType>(getType(nameAndType->getDescriptor(), ci.type));
      Function* function = module_.getOrInsertFunction(funcName, funcTy);
      toCompileFunctions_.insert(function);
      makeCall(function, getParams(funcTy));
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

      const std::string& className = methodRef->getClass()->getName()->str();

      const ClassInfo* ci = NULL;
      const VTableInfo* vi = NULL;
      tie(ci, vi) = getInfo(className);

      const std::string& methodDescr =
	nameAndType->getName()->str() +
	nameAndType->getDescriptor()->str();

      FunctionType* funTy =
	cast<FunctionType>(getType(nameAndType->getDescriptor(), ci->type));

      std::vector<Value*> params(getParams(funTy));

      Value* objRef = params.front();
      objRef = new CastInst(objRef, PointerType::get(ci->type),
			    "this", currentBB_);
      Value* objBase =
        new CastInst(objRef, ClassInfo::ObjectBaseTy, TMP, currentBB_);
      Function* f = module_.getOrInsertFunction(
	LLVM_JAVA_GETOBJECTCLASS, PointerType::get(VTableInfo::VTableTy),
	objBase->getType(), NULL);
      Value* vtable = new CallInst(f, objBase, TMP, currentBB_);
      // get the interfaces array of vtables
      std::vector<Value*> indices(2, ConstantUInt::get(Type::UIntTy, 0));
      indices.push_back(ConstantUInt::get(Type::UIntTy, 3));
      Value* interfaceVTables =
	new GetElementPtrInst(vtable, indices, TMP, currentBB_);
      interfaceVTables = new LoadInst(interfaceVTables, TMP, currentBB_);
      // Get the actual interface vtable.
      indices.clear();
      indices.push_back(ConstantUInt::get(Type::UIntTy, ci->interfaceIdx));
      Value* interfaceVTable =
	new GetElementPtrInst(interfaceVTables, indices, TMP, currentBB_);
      interfaceVTable =
	new LoadInst(interfaceVTable, className + "<vtable>", currentBB_);
      interfaceVTable =
	new CastInst(interfaceVTable, vi->vtable->getType(), TMP, currentBB_);
      // Get the function pointer.
      indices.resize(1);
      assert(vi->m2iMap.find(methodDescr) != vi->m2iMap.end() &&
	     "could not find slot for virtual function!");
      unsigned vSlot = vi->m2iMap.find(methodDescr)->second;
      indices.push_back(ConstantUInt::get(Type::UIntTy, vSlot));
      Value* vfunPtr =
	new GetElementPtrInst(interfaceVTable, indices, TMP, currentBB_);
      Value* vfun = new LoadInst(vfunPtr, methodDescr, currentBB_);

      makeCall(vfun, params);
    }

    void do_new(unsigned index) {
      ConstantClass* classRef = cf_->getConstantClass(index);
      ClassFile* cf = ClassFile::get(classRef->getName()->str());
      const ClassInfo& ci = getClassInfo(cf);
      const VTableInfo& vi = getVTableInfo(cf);

      Value* objRef = new MallocInst(ci.type,
				     ConstantUInt::get(Type::UIntTy, 0),
				     TMP, currentBB_);
      Value* objBase = getField(cf, LLVM_JAVA_OBJECT_BASE, objRef);
      Value* vtable = new CastInst(vi.vtable,
				   PointerType::get(VTableInfo::VTableTy),
				   TMP, currentBB_);
      Function* f = module_.getOrInsertFunction(
	LLVM_JAVA_SETOBJECTCLASS, Type::VoidTy,
	objBase->getType(), PointerType::get(VTableInfo::VTableTy), NULL);
      new CallInst(f, objBase, vtable, "", currentBB_);
      currentOpStack_->push(objRef, currentBB_);
    }

    Value* getArrayLengthPtr(Value* arrayRef) const {
      std::vector<Value*> indices;
      indices.reserve(2);
      indices.push_back(ConstantUInt::get(Type::UIntTy, 0));
      indices.push_back(ConstantUInt::get(Type::UIntTy, 1));

      return new GetElementPtrInst(arrayRef, indices, TMP, currentBB_);
    }

    Value* getArrayObjectBasePtr(Value* arrayRef) const {
      std::vector<Value*> indices;
      indices.reserve(2);
      indices.push_back(ConstantUInt::get(Type::UIntTy, 0));
      indices.push_back(ConstantUInt::get(Type::UIntTy, 0));

      return new GetElementPtrInst(arrayRef, indices, TMP, currentBB_);
    }

    void do_newarray(JType type) {
      Value* count = currentOpStack_->pop(currentBB_);
      count = new CastInst(count, Type::UIntTy, TMP, currentBB_);

      const ClassInfo& ci = getPrimitiveArrayInfo(type);
      const VTableInfo& vi = getPrimitiveArrayVTableInfo(type);

      do_newarray_common(ci, getType(type), vi, count);
    }

    void do_anewarray(unsigned index) {
      Value* count = currentOpStack_->pop(currentBB_);
      count = new CastInst(count, Type::UIntTy, TMP, currentBB_);

      ConstantClass* classRef = cf_->getConstantClass(index);
      ClassFile* cf = ClassFile::get(classRef->getName()->str());
      const ClassInfo& ci = getObjectArrayInfo();
      const ClassInfo& ei = getClassInfo(cf);
      const VTableInfo& vi = getObjectArrayVTableInfo(cf);

      do_newarray_common(ci, PointerType::get(ei.type), vi, count);
    }

    void do_newarray_common(const ClassInfo& ci,
                            Type* elementTy,
                            const VTableInfo& vi,
                            Value* count) {
      // The size of the array part of the struct.
      Value* size = BinaryOperator::create(
	Instruction::Mul, count, ConstantExpr::getSizeOf(elementTy),
	TMP, currentBB_);
      // Plus the size of the rest of the struct.
      size = BinaryOperator::create(
	Instruction::Add, size, ConstantExpr::getSizeOf(ci.type),
	TMP, currentBB_);
      // Allocate memory for the object.
      Value* objRef = new MallocInst(Type::SByteTy, size, TMP, currentBB_);
      objRef = new CastInst(objRef, PointerType::get(ci.type), TMP, currentBB_);

      // Store the size.
      Value* lengthPtr = getArrayLengthPtr(objRef);
      new StoreInst(count, lengthPtr, currentBB_);
      // Install the vtable pointer.
      Value* objBase = getArrayObjectBasePtr(objRef);
      Value* vtable = new CastInst(vi.vtable,
				   PointerType::get(VTableInfo::VTableTy),
				   TMP, currentBB_);
      Function* f = module_.getOrInsertFunction(
	LLVM_JAVA_SETOBJECTCLASS, Type::VoidTy,
	objBase->getType(), PointerType::get(VTableInfo::VTableTy), NULL);
      new CallInst(f, objBase, vtable, "", currentBB_);
      currentOpStack_->push(objRef, currentBB_);
    }

    void do_arraylength() {
      Value* arrayRef = currentOpStack_->pop(currentBB_);
      const ClassInfo& ci = getObjectArrayInfo();
      arrayRef =
	new CastInst(arrayRef, PointerType::get(ci.type), TMP, currentBB_);
      Value* lengthPtr = getArrayLengthPtr(arrayRef);
      Value* length = new LoadInst(lengthPtr, TMP, currentBB_);
      length = new CastInst(length, Type::IntTy, TMP, currentBB_);
      currentOpStack_->push(length, currentBB_);
    }

    void do_athrow() {
      Value* objRef = currentOpStack_->pop(currentBB_);
      objRef = new CastInst(objRef, PointerType::get(ClassInfo::ObjectBaseTy),
			    TMP, currentBB_);
      Function* f = module_.getOrInsertFunction(
	LLVM_JAVA_THROW, Type::IntTy, objRef->getType(), NULL);
      new CallInst(f, objRef, TMP, currentBB_);
      new UnreachableInst(currentBB_);
    }

    void do_checkcast(unsigned index) {
      ConstantClass* classRef = cf_->getConstantClass(index);

      const ClassInfo* ci = NULL;
      const VTableInfo* vi = NULL;
      tie(ci, vi) = getInfo(classRef->getName()->str());

      Value* objRef = currentOpStack_->pop(currentBB_);
      Value* objBase =
        new CastInst(objRef, ClassInfo::ObjectBaseTy, TMP, currentBB_);
      Function* f = module_.getOrInsertFunction(
        LLVM_JAVA_ISINSTANCEOF, Type::IntTy,
        objBase->getType(), PointerType::get(VTableInfo::VTableTy), NULL);
      Value* vtable = new CastInst(vi->vtable,
                                   PointerType::get(VTableInfo::VTableTy),
                                   TMP, currentBB_);
      Value* r = new CallInst(f, objBase, vtable, TMP, currentBB_);

      Value* b = new SetCondInst(Instruction::SetEQ,
                                 r, ConstantSInt::get(Type::IntTy, 1),
                                 TMP, currentBB_);
      // FIXME: if b is false we must throw a ClassCast exception
      Value* objCast =
        new CastInst(objRef, PointerType::get(ci->type), TMP, currentBB_);
      currentOpStack_->push(objCast, currentBB_);
    }

    void do_instanceof(unsigned index) {
      ConstantClass* classRef = cf_->getConstantClass(index);

      const ClassInfo* ci = NULL;
      const VTableInfo* vi = NULL;
      tie(ci, vi) = getInfo(classRef->getName()->str());

      Value* objRef = currentOpStack_->pop(currentBB_);
      Value* objBase =
        new CastInst(objRef, ClassInfo::ObjectBaseTy, TMP, currentBB_);
      Function* f = module_.getOrInsertFunction(
	LLVM_JAVA_ISINSTANCEOF, Type::IntTy,
	objBase->getType(), PointerType::get(VTableInfo::VTableTy), NULL);
      Value* vtable = new CastInst(vi->vtable,
				   PointerType::get(VTableInfo::VTableTy),
				   TMP, currentBB_);
      Value* r = new CallInst(f, objBase, vtable, TMP, currentBB_);
      currentOpStack_->push(r, currentBB_);
    }

    void do_monitorenter() {
      // assert(0 && "not implemented");
    }

    void do_monitorexit() {
      // assert(0 && "not implemented");
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

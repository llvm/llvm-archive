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

using namespace llvm;
using namespace llvm::Java;

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
    const VMClass* class_;
    std::auto_ptr<BasicBlockBuilder> bbBuilder_;
    std::list<BasicBlock*> bbWorkList_;
    typedef std::map<BasicBlock*, unsigned> OpStackDepthMap;
    OpStackDepthMap opStackDepthMap_;
    typedef std::map<std::string, GlobalVariable*> StringMap;
    StringMap stringMap_;
    BasicBlock* currentBB_;
    Locals locals_;
    OperandStack opStack_;
    Function *getClassRecord_, *setClassRecord_, *throw_, *isInstanceOf_,
      *memcpy_, *memset_;
    std::vector<llvm::Constant*> classInitializers_;

    SetVector<const VMMethod*> toCompileMethods_;

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
      const Type* classRecordPtrType = resolver_->getClassRecordPtrType();
      getClassRecord_ = module_->getOrInsertFunction(
        "llvm_java_get_class_record", classRecordPtrType,
        resolver_->getObjectBaseType(), NULL);
      setClassRecord_ = module_->getOrInsertFunction(
        "llvm_java_set_class_record", Type::VoidTy,
        resolver_->getObjectBaseType(), classRecordPtrType, NULL);
      throw_ = module_->getOrInsertFunction(
        "llvm_java_throw", Type::IntTy,
        resolver_->getObjectBaseType(), NULL);
      isInstanceOf_ = module_->getOrInsertFunction(
        "llvm_java_is_instance_of", Type::IntTy,
        resolver_->getObjectBaseType(), classRecordPtrType, NULL);
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
    bool scheduleMethod(const VMMethod* method) {
      if (toCompileMethods_.insert(method)) {
        DEBUG(std::cerr << "Scheduling function: "
              << method->getFunction()->getName() << " for compilation\n");
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
      Value* arrayRef = allocateArray(resolver_->getClass("[B"), count, ip);
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
      const VMClass* clazz = resolver_->getClass("java/lang/String");
      emitClassInitializers(clazz);

      // Install the class record.
      Value* objBase =
        new CastInst(globalString, resolver_->getObjectBaseType(), TMP, ip);
      const Type* classRecordPtrType = resolver_->getClassRecordPtrType();
      Value* classRecord =
        new CastInst(clazz->getClassRecord(), classRecordPtrType, TMP, ip);
      new CallInst(setClassRecord_, objBase, classRecord, "", ip);

      // Initialize it: call java/lang/String/<init>(byte[],int)
      const VMMethod* method = clazz->getMethod("<init>([BI)V");
      scheduleMethod(method);

      params.reserve(3);
      params.clear();
      params.push_back(objBase);
      params.push_back(
        new CastInst(arrayRef, resolver_->getObjectBaseType(), TMP, ip));
      params.push_back(ConstantSInt::get(Type::IntTy, 0));
      new CallInst(method->getFunction(), params, "", ip);
    }

    /// Returns the type of the Java string descriptor for JNI.
    const Type* getJNIType(const std::string& descr) {
      unsigned i = 0;
      return getJNITypeHelper(descr, i);
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
        return resolver_->getObjectBaseType();
      }
      case '[':
        // Skip '['s.
        if (descr[i] == '[')
          do { ++i; } while (descr[i] == '[');
        // Consume the element type
        getJNITypeHelper(descr, i);
        return resolver_->getObjectBaseType();
      case '(': {
        std::vector<const Type*> params;
        // JNIEnv*
        params.push_back(JNIEnvPtr_->getType());
        params.push_back(resolver_->getObjectBaseType());
        while (descr[i] != ')')
          params.push_back(getJNITypeHelper(descr, i));
        return FunctionType::get(getJNITypeHelper(descr, ++i), params, false);
      }
        // FIXME: Throw something
      default:  assert(0 && "Cannot parse type descriptor!");
      }
      return 0; // not reached
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
    void compileMethodOnly(const VMMethod* method) {
      class_ = method->getParent();

      Function* function = method->getFunction();
      if (!function->empty()) {
        DEBUG(std::cerr << "Function: " << function->getName()
              << " is already compiled!\n");
        return;
      }

      const std::string& className =
        class_->getClassFile()->getThisClass()->getName()->str();

      if (method->isNative()) {
        DEBUG(std::cerr << "Adding stub for natively implemented method: "
              << function->getName() << '\n');
        const FunctionType* jniFuncTy =
          cast<FunctionType>(getJNIType(method->getDescriptor()));

        std::string funcName =
          "Java_" +
          getMangledString(className) + '_' +
          getMangledString(method->getName());
        if (class_->getClassFile()->isNativeMethodOverloaded(*method->getMethod())) {
          // We need to add two underscores and a mangled argument signature
          funcName += "__";
          const std::string descr = method->getDescriptor();
          funcName += getMangledString(
            std::string(descr.begin() + descr.find('(') + 1,
                        descr.begin() + descr.find(')')));
        }

        Function* jniFunction =
          module_->getOrInsertFunction(funcName, jniFuncTy);

        BasicBlock* bb = new BasicBlock("entry", function);
        std::vector<Value*> params;
        params.push_back(JNIEnvPtr_);
        if (method->isStatic())
          params.push_back(llvm::Constant::getNullValue(resolver_->getObjectBaseType()));
        for (Function::arg_iterator A = function->arg_begin(),
               E = function->arg_end(); A != E; ++A) {
          params.push_back(
            new CastInst(A, jniFuncTy->getParamType(params.size()), TMP, bb));
        }
        Value* result = new CallInst(jniFunction, params, "", bb);
        if (result->getType() != Type::VoidTy)
          result = new CastInst(result, function->getReturnType(), TMP,bb);
        new ReturnInst(result, bb);

        return;
      }

      assert (!method->isAbstract() && "Trying to compile an abstract method!");

      // HACK: skip most of the class libraries.
      const std::string& funcName = function->getName();
      if ((funcName.find("java/") == 0 &&
           funcName.find("java/lang/Object") != 0 &&
           (funcName.find("java/lang/Throwable") != 0 ||
            funcName.find("java/lang/Throwable$StaticData/<cl") == 0) &&
           funcName.find("java/lang/Exception") != 0 &&
           funcName.find("java/lang/IllegalArgumentException") != 0 &&
           funcName.find("java/lang/IllegalStateException") != 0 &&
           funcName.find("java/lang/IndexOutOfBoundsException") != 0 &&
           funcName.find("java/lang/RuntimeException") != 0 &&
           funcName.find("java/lang/Math") != 0 &&
           funcName.find("java/lang/Number") != 0 &&
           funcName.find("java/lang/Byte") != 0 &&
           funcName.find("java/lang/Float") != 0 &&
           funcName.find("java/lang/Integer") != 0 &&
           funcName.find("java/lang/Long") != 0 &&
           funcName.find("java/lang/Short") != 0 &&
           (funcName.find("java/lang/String") != 0 ||
            funcName.find("java/lang/String/<cl") == 0) &&
           funcName.find("java/lang/StringBuffer") != 0 &&
           funcName.find("java/lang/System") != 0 &&
           funcName.find("java/lang/VMSystem") != 0 &&
           (funcName.find("java/util/") != 0 ||
            funcName.find("java/util/Locale/<cl") == 0 ||
            funcName.find("java/util/ResourceBundle/<cl") == 0 ||
            funcName.find("java/util/Calendar/<cl") == 0)) ||
          (funcName.find("gnu/") == 0)) {
        DEBUG(std::cerr << "Skipping compilation of method: "
              << funcName << '\n');
        return;
      }

      DEBUG(std::cerr << "Compiling method: " << funcName << '\n');

      Java::CodeAttribute* codeAttr = method->getMethod()->getCodeAttribute();

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

      DEBUG(std::cerr << "Finished compilation of method: "<< funcName << '\n');
      // DEBUG(function->dump());
    }

    /// Emits static initializers for this class if not done already.
    void emitClassInitializers(const VMClass* clazz) {
      static SetVector<const VMClass*> toInitClasses;

      // If this is a primitive class we are done.
      if (clazz->isPrimitive())
        return;

      // If this class is already initialized, we are done.
      if (!toInitClasses.insert(clazz))
        return;

      // If this class has a super class, initialize that first.
      if (const VMClass* superClass = clazz->getSuperClass())
        emitClassInitializers(superClass);

      // If this class is an array, initialize its component class now.
      if (const VMClass* componentClass = clazz->getComponentClass())
        emitClassInitializers(componentClass);

      // Schedule all its dynamically bound non abstract methods for
      // compilation.
      for (unsigned i = 0, e = clazz->getNumDynamicallyBoundMethods();
           i != e; ++i) {
        const VMMethod* method = clazz->getDynamicallyBoundMethod(i);
        if (!method->isAbstract())
          scheduleMethod(method);
      }

      
      // If this class has a constant pool (was loaded from a
      // classfile), create constant strings for it.
      if (const ClassFile* classfile = clazz->getClassFile()) {
        Function* stringConstructors = module_->getOrInsertFunction(
          clazz->getName() + "<strinit>",
          FunctionType::get(Type::VoidTy, std::vector<const Type*>(), false));
        Instruction* I =
          new ReturnInst(NULL, new BasicBlock("entry", stringConstructors));
        for (unsigned i = 0, e = classfile->getNumConstants(); i != e; ++i)
          if (ConstantString* s = dynamic_cast<ConstantString*>(classfile->getConstant(i)))
            initializeString(clazz->getConstant(i), s->getValue()->str(), I);

        // Insert string constructors method in class initializers array.
        classInitializers_.push_back(stringConstructors);

        // Call its class initialization method if it exists.
        if (const VMMethod* method = clazz->getMethod("<clinit>()V")) {
          classInitializers_.push_back(method->getFunction());
          bool inserted =  scheduleMethod(method);
          assert(inserted && "Class initialization method already called!");
        }
      }
    }

  public:
    /// Compiles the specified method given a <class,method>
    /// descriptor and the transitive closure of all methods
    /// (possibly) called by it.
    const VMMethod* compileMethod(const std::string& className,
                                  const std::string& methodDesc) {
      // Load the class.
      const VMClass* clazz = resolver_->getClass(className);
      emitClassInitializers(clazz);

      // Find the method.
      const VMMethod* method = clazz->getMethod(methodDesc);
      scheduleMethod(method);
      // Compile the transitive closure of methods called by this method.
      for (unsigned i = 0; i != toCompileMethods_.size(); ++i) {
        const VMMethod* m = toCompileMethods_[i];
        compileMethodOnly(m);
        DEBUG(std::cerr << i+1 << '/' << toCompileMethods_.size()
              << " functions compiled\n");
      }

      // Null terminate the static initializers array and add the
      // global to the module.
      Type* classInitializerType = PointerType::get(
          FunctionType::get(Type::VoidTy, std::vector<const Type*>(), false));
      classInitializers_.push_back(
        llvm::Constant::getNullValue(classInitializerType));

      ArrayType* classInitializersType =
        ArrayType::get(classInitializerType, classInitializers_.size());
      new GlobalVariable(classInitializersType,
                         true,
                         GlobalVariable::ExternalLinkage,
                         ConstantArray::get(classInitializersType,
                                            classInitializers_),
                         "llvm_java_class_initializers",
                         module_);


      return method;
    }

    void do_aconst_null() {
      push(llvm::Constant::getNullValue(resolver_->getObjectBaseType()));
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
    void do_aload(unsigned index) { do_load_common(index, resolver_->getObjectBaseType()); }

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
      const VMClass* arrayClass = resolver_->getClass(className);
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
    void do_astore(unsigned index) { do_store_common(index, resolver_->getObjectBaseType()); }

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
      const VMClass* arrayClass = resolver_->getClass(className);
      assert(arrayClass->isArray() && "Not an array class!");
      const VMClass* componentClass = arrayClass->getComponentClass();
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
      do_if_common(Instruction::SetEQ, resolver_->getObjectBaseType(), t, f);
    }
    void do_if_acmpne(unsigned t, unsigned f) {
      do_if_common(Instruction::SetNE, resolver_->getObjectBaseType(), t, f);
    }
    void do_ifnull(unsigned t, unsigned f) {
      do_aconst_null();
      do_if_common(Instruction::SetEQ, resolver_->getObjectBaseType(), t, f);
    }
    void do_ifnonnull(unsigned t, unsigned f) {
      do_aconst_null();
      do_if_common(Instruction::SetNE, resolver_->getObjectBaseType(), t, f);
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
    void do_areturn() { do_return_common(resolver_->getObjectBaseType()); }

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
      const VMField* field = class_->getField(index);
      emitClassInitializers(field->getParent());

      Value* v = new LoadInst(field->getGlobal(), TMP, currentBB_);
      push(v);
    }

    void do_putstatic(unsigned index) {
      const VMField* field = class_->getField(index);
      emitClassInitializers(field->getParent());

      Value* v = pop(field->getClass()->getType());
      new StoreInst(v, field->getGlobal(), currentBB_);
    }

    void do_getfield(unsigned index) {
      const VMField* field = class_->getField(index);

      Value* p = pop(field->getParent()->getType());
      std::vector<Value*> indices(2);
      indices[0] = ConstantUInt::get(Type::UIntTy, 0);
      indices[1] = ConstantUInt::get(Type::UIntTy, field->getMemberIndex());
      Value* fieldPtr =
        new GetElementPtrInst(p, indices, field->getName()+'*', currentBB_);
      Value* v = new LoadInst(fieldPtr, field->getName(), currentBB_);
      push(v);
    }

    void do_putfield(unsigned index) {
      const VMField* field = class_->getField(index);

      Value* v = pop(field->getClass()->getType());
      Value* p = pop(field->getParent()->getType());
      std::vector<Value*> indices(2);
      indices[0] = ConstantUInt::get(Type::UIntTy, 0);
      indices[1] = ConstantUInt::get(Type::UIntTy, field->getMemberIndex());
      Value* fieldPtr =
        new GetElementPtrInst(p, indices, field->getName()+'*', currentBB_);
      new StoreInst(v, fieldPtr, currentBB_);
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

    void do_invokevirtual(unsigned index) {
      const VMMethod* method = class_->getMethod(index);
      const VMClass* clazz = method->getParent();

      Function* function = method->getFunction();
      std::vector<Value*> params(getParams(function->getFunctionType()));

      Value* objRef = params.front();
      objRef = new CastInst(objRef, clazz->getType(), "this", currentBB_);
      Value* objBase =
        new CastInst(objRef, resolver_->getObjectBaseType(), TMP, currentBB_);
      Value* classRecord =
        new CallInst(getClassRecord_, objBase, TMP, currentBB_);
      classRecord = new CastInst(classRecord,
                                 clazz->getClassRecord()->getType(),
                                 clazz->getName() + ".classRecord", currentBB_);
      std::vector<Value*> indices(1, ConstantUInt::get(Type::UIntTy, 0));
      assert(method->getMethodIndex() != -1 &&
             "Method index not found for dynamically bound method!");
      indices.push_back(
        ConstantUInt::get(Type::UIntTy, method->getMethodIndex()+1));
      Value* funPtr =
        new GetElementPtrInst(classRecord, indices, TMP, currentBB_);
      Value* fun = new LoadInst(funPtr, function->getName(), currentBB_);

      makeCall(fun, params);
    }

    void do_invokespecial(unsigned index) {
      const VMMethod* method = class_->getMethod(index);
      scheduleMethod(method);
      Function* function = method->getFunction();
      makeCall(function, getParams(function->getFunctionType()));
    }

    void do_invokestatic(unsigned index) {
      const VMMethod* method = class_->getMethod(index);
      emitClassInitializers(method->getParent());
      Function* function = method->getFunction();
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
        scheduleMethod(method);
      makeCall(function, getParams(function->getFunctionType()));
    }

    void do_invokeinterface(unsigned index) {
      const VMMethod* method = class_->getMethod(index);
      const VMClass* clazz = method->getParent();
      assert(clazz->isInterface() && "Class must be an interface!");

      Function* function = method->getFunction();
      std::vector<Value*> params(getParams(function->getFunctionType()));

      Value* objRef = params.front();
      objRef = new CastInst(objRef, clazz->getType(), "this", currentBB_);
      Value* objBase =
        new CastInst(objRef, resolver_->getObjectBaseType(), TMP, currentBB_);
      Value* classRecord =
        new CallInst(getClassRecord_, objBase, TMP, currentBB_);
      classRecord = new CastInst(classRecord,
                                 resolver_->getClassRecordPtrType(),
                                 TMP, currentBB_);
      // get the interfaces array of class records
      std::vector<Value*> indices(2, ConstantUInt::get(Type::UIntTy, 0));
      indices.push_back(ConstantUInt::get(Type::UIntTy, 4));
      Value* interfaceClassRecords =
        new GetElementPtrInst(classRecord, indices, TMP, currentBB_);
      interfaceClassRecords =
        new LoadInst(interfaceClassRecords, TMP, currentBB_);
      // Get the actual interface class record.
      indices.clear();
      indices.push_back(
        ConstantUInt::get(Type::UIntTy, clazz->getInterfaceIndex()));
      Value* interfaceClassRecord =
        new GetElementPtrInst(interfaceClassRecords, indices, TMP, currentBB_);
      interfaceClassRecord =
        new LoadInst(interfaceClassRecord,
                     clazz->getName() + ".classRecord", currentBB_);
      interfaceClassRecord =
        new CastInst(interfaceClassRecord,
                     clazz->getClassRecord()->getType(), TMP, currentBB_);
      // Get the function pointer.
      assert(method->getMethodIndex() != -1 &&
             "Method index not found for dynamically bound method!");
      indices.resize(2);
      indices[0] = ConstantUInt::get(Type::UIntTy, 0);
      indices[1] = ConstantUInt::get(Type::UIntTy, method->getMethodIndex()+1);
      Value* funPtr =
        new GetElementPtrInst(interfaceClassRecord, indices, TMP, currentBB_);
      Value* fun = new LoadInst(funPtr, function->getName(), currentBB_);

      makeCall(fun, params);
    }

    template<typename InsertionPointTy>
    Value* allocateObject(const VMClass& clazz, InsertionPointTy* ip) {
      static std::vector<Value*> params(4);

      Value* objRef = new MallocInst(clazz.getLayoutType(), NULL, TMP, ip);
      params[0] =
        new CastInst(objRef, PointerType::get(Type::SByteTy), TMP, ip); // dest
      params[1] = ConstantUInt::get(Type::UByteTy, 0); // value
      params[2] = ConstantExpr::getSizeOf(clazz.getLayoutType()); // size
      params[3] = ConstantUInt::get(Type::UIntTy, 0); // alignment
      new CallInst(memset_, params, "", ip);

      // Install the class record.
      Value* objBase =
        new CastInst(objRef, resolver_->getObjectBaseType(), TMP, ip);
      const Type* classRecordPtrType = resolver_->getClassRecordPtrType();
      Value* classRecord =
        new CastInst(clazz.getClassRecord(), classRecordPtrType, TMP, ip);
      new CallInst(setClassRecord_, objBase, classRecord, "", ip);

      return objRef;
    }

    void do_new(unsigned index) {
      const VMClass* clazz = class_->getClass(index);
      emitClassInitializers(clazz);
      push(allocateObject(*clazz, currentBB_));
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
    Value* allocateArray(const VMClass* clazz,
                         Value* count,
                         InsertionPointTy* ip) {
      static std::vector<Value*> params(4);

      assert(clazz->isArray() && "Not an array class!");
      const VMClass* componentClass = clazz->getComponentClass();
      const Type* elementTy = componentClass->getType();

      // The size of the element.
      llvm::Constant* elementSize =
        ConstantExpr::getCast(ConstantExpr::getSizeOf(elementTy), Type::UIntTy);

      // The size of the array part of the struct.
      Value* size = BinaryOperator::create(
        Instruction::Mul, count, elementSize, TMP, ip);
      // The size of the rest of the array object.
      llvm::Constant* arrayObjectSize =
        ConstantExpr::getCast(ConstantExpr::getSizeOf(clazz->getLayoutType()),
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

      // Install the class record.
      Value* objBase = new CastInst(objRef, resolver_->getObjectBaseType(), TMP, ip);
      const Type* classRecordPtrType = resolver_->getClassRecordPtrType();
      Value* classRecord =
        new CastInst(clazz->getClassRecord(), classRecordPtrType, TMP, ip);
      new CallInst(setClassRecord_, objBase, classRecord, "", ip);

      return objRef;
    }

    void do_newarray(JType type) {
      Value* count = pop(Type::UIntTy);

      const VMClass* clazz = resolver_->getClass(type);
      const VMClass* arrayClass = resolver_->getArrayClass(clazz);
      emitClassInitializers(arrayClass);

      push(allocateArray(arrayClass, count, currentBB_));
    }

    void do_anewarray(unsigned index) {
      Value* count = pop(Type::UIntTy);

      const VMClass* clazz = class_->getClass(index);
      const VMClass* arrayClass = resolver_->getArrayClass(clazz);
      emitClassInitializers(arrayClass);

      push(allocateArray(arrayClass, count, currentBB_));
    }

    void do_arraylength() {
      const VMClass* clazz = resolver_->getClass("[Ljava/lang/Object;");
      Value* arrayRef = pop(clazz->getType());
      Value* lengthPtr = getArrayLengthPtr(arrayRef, currentBB_);
      Value* length = new LoadInst(lengthPtr, TMP, currentBB_);
      push(length);
    }

    void do_athrow() {
      Value* objRef = pop(resolver_->getObjectBaseType());
      new CallInst(throw_, objRef, "", currentBB_);
      new UnreachableInst(currentBB_);
    }

    void do_checkcast(unsigned index) {
      const VMClass* clazz = class_->getClass(index);

      Value* objRef = pop(resolver_->getObjectBaseType());
      const Type* classRecordPtrType = resolver_->getClassRecordPtrType();
      Value* classRecord = new CastInst(clazz->getClassRecord(),
                                        classRecordPtrType, TMP, currentBB_);
      Value* r =
        new CallInst(isInstanceOf_, objRef, classRecord, TMP, currentBB_);

      Value* b = new SetCondInst(Instruction::SetEQ,
                                 r, ConstantSInt::get(Type::IntTy, 1),
                                 TMP, currentBB_);
      // FIXME: if b is false we must throw a ClassCast exception
      push(objRef);
    }

    void do_instanceof(unsigned index) {
      const VMClass* clazz = class_->getClass(index);

      Value* objRef = pop(resolver_->getObjectBaseType());
      const Type* classRecordPtrType = resolver_->getClassRecordPtrType();
      Value* classRecord = new CastInst(clazz->getClassRecord(),
                                        classRecordPtrType, TMP, currentBB_);
      Value* r =
        new CallInst(isInstanceOf_, objRef, classRecord, TMP, currentBB_);
      push(r);
    }

    void do_monitorenter() {
      // FIXME: This is currently a noop.
      pop(resolver_->getObjectBaseType());
    }

    void do_monitorexit() {
      // FIXME: This is currently a noop.
      pop(resolver_->getObjectBaseType());
    }

    void do_multianewarray(unsigned index, unsigned dims) {
      assert(0 && "not implemented");
    }
  };

} } } // namespace llvm::Java::

std::auto_ptr<Module> llvm::Java::compile(const std::string& className)
{
  DEBUG(std::cerr << "Compiling class: " << className << '\n');

  std::auto_ptr<Module> m(new Module(className));
  // Require the Java runtime.
  m->addLibrary("jrt");

  Compiler c(m.get());
  const VMMethod* main =
    c.compileMethod(className, "main([Ljava/lang/String;)V");

  Function* javaMain = m->getOrInsertFunction
    ("llvm_java_main", Type::VoidTy,
     Type::IntTy, PointerType::get(PointerType::get(Type::SByteTy)), NULL);

  BasicBlock* bb = new BasicBlock("entry", javaMain);
  const FunctionType* mainTy = main->getFunction()->getFunctionType();
  new CallInst(main->getFunction(),
               // FIXME: Forward correct params from llvm_java_main
               llvm::Constant::getNullValue(mainTy->getParamType(0)),
               "",
               bb);
  new ReturnInst(NULL, bb);

  return m;
}

//===-- ClassFile.h - ClassFile support library -----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the ClassFile and subordinate
// classes, which represent a parsed class file.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_JAVA_CLASSFILE_H
#define LLVM_JAVA_CLASSFILE_H

#include <llvm/System/Path.h>

#include <iosfwd>
#include <stdexcept>
#include <vector>

#include <llvm/Support/DataTypes.h>

namespace llvm { namespace Java {

  // Forward declarations
  class Attribute;
  class ConstantValueAttribute;
  class CodeAttribute;
  class ExceptionsAttribute;
  class Constant;
  class ConstantClass;
  class ConstantFieldRef;
  class ConstantInterfaceMethodRef;
  class ConstantMemberRef;
  class ConstantMethodRef;
  class ConstantNameAndType;
  class ConstantUtf8;
  class ClassFile;
  class Field;
  class Method;

  enum AccessFlag {
    ACC_PUBLIC       = 0x0001,
    ACC_PRIVATE      = 0x0002,
    ACC_PROTECTED    = 0x0004,
    ACC_STATIC       = 0x0008,
    ACC_FINAL        = 0x0010,
    ACC_SUPER        = 0x0020,
    ACC_SYNCHRONIZED = 0x0020,
    ACC_VOLATILE     = 0x0040,
    ACC_TRANSIENT    = 0x0080,
    ACC_NATIVE       = 0x0100,
    ACC_INTERFACE    = 0x0200,
    ACC_ABSTRACT     = 0x0400,
    ACC_STRICT       = 0x0800,
  };

  typedef std::vector<Constant*> ConstantPool;
  typedef std::vector<Field*> Fields;
  typedef std::vector<Method*> Methods;
  typedef std::vector<Attribute*> Attributes;

  Attribute* getAttribute(const Attributes& attrs,
                          const std::string& name);

  class ClassFile {
    static const ClassFile* readClassFile(std::istream& is);
    static std::vector<llvm::sys::Path> getClassPath();
    static sys::Path getFileForClass(const std::string& classname);

  public:
    static const ClassFile* get(const std::string& classname);

    ~ClassFile();

    uint16_t getMinorVersion() const { return minorV_; }
    uint16_t getMajorVersion() const { return majorV_; }

    unsigned getNumConstants() const { return cPool_.size(); }
    Constant* getConstant(unsigned index) const { return cPool_[index]; }
    ConstantClass* getConstantClass(unsigned index) const;
    ConstantMemberRef* getConstantMemberRef(unsigned index) const;
    ConstantFieldRef* getConstantFieldRef(unsigned index) const;
    ConstantMethodRef* getConstantMethodRef(unsigned index) const;
    ConstantInterfaceMethodRef*
    getConstantInterfaceMethodRef(unsigned index) const;
    ConstantNameAndType* getConstantNameAndType(unsigned index) const;
    ConstantUtf8* getConstantUtf8(unsigned index) const;

    bool isPublic() const { return accessFlags_ & ACC_PUBLIC; }
    bool isFinal() const { return accessFlags_ & ACC_FINAL; }
    bool isSuper() const { return accessFlags_ & ACC_SUPER; }
    bool isInterface() const { return accessFlags_ & ACC_INTERFACE; }
    bool isAbstract() const { return accessFlags_ & ACC_ABSTRACT; }

    unsigned getThisClassIndex() const { return thisClassIdx_; }
    ConstantClass* getThisClass() const {
      return getConstantClass(thisClassIdx_);
    }
    unsigned getSuperClassIndex() const { return superClassIdx_; }
    ConstantClass* getSuperClass() const {
      return superClassIdx_ ? getConstantClass(superClassIdx_) : NULL;
    }

    unsigned getNumInterfaces() const { return interfaces_.size(); }
    unsigned getInterfaceIndex(unsigned i) const { return interfaces_[i]; }
    ConstantClass* getInterface(unsigned i) const {
      return getConstantClass(getInterfaceIndex(i));
    }

    const Fields& getFields() const { return fields_; }

    const Methods& getMethods() const { return methods_; }

    const Attributes& getAttributes() const { return attributes_; }

    bool isNativeMethodOverloaded(const Method& method) const;

    std::ostream& dump(std::ostream& os) const;

  private:
    uint16_t majorV_;
    uint16_t minorV_;
    ConstantPool cPool_;
    uint16_t accessFlags_;
    uint16_t thisClassIdx_;
    uint16_t superClassIdx_;
    std::vector<uint16_t> interfaces_;
    Fields fields_;
    Methods methods_;
    Attributes attributes_;

    ClassFile(std::istream& is);
  };

  inline std::ostream& operator<<(std::ostream& os, const ClassFile& c) {
    return c.dump(os);
  }

  class Constant {
  protected:
    const ClassFile* parent_;

    Constant(const ClassFile* cf)
      : parent_(cf) { }

  public:
    enum Tag {
      CLASS = 7,
      FIELD_REF = 9,
      METHOD_REF = 10,
      INTERFACE_METHOD_REF = 11,
      STRING = 8,
      INTEGER = 3,
      FLOAT = 4,
      LONG = 5,
      DOUBLE = 6,
      NAME_AND_TYPE = 12,
      UTF8 = 1
    };

    static Constant* readConstant(const ClassFile* cf, std::istream& is);

    virtual bool isSingleSlot() { return true; }
    bool isDoubleSlot() { return !isSingleSlot(); }
    const ClassFile* getParent() { return parent_; }

    virtual ~Constant();

    virtual std::ostream& dump(std::ostream& os) const = 0;
  };

  inline std::ostream& operator<<(std::ostream& os, const Constant& c) {
    return c.dump(os);
  }

  class ConstantClass : public Constant {
    uint16_t nameIdx_;
  public:
    ConstantClass(const ClassFile* cf, std::istream& is);
    unsigned getNameIndex() const { return nameIdx_; }
    ConstantUtf8* getName() const {
      return parent_->getConstantUtf8(nameIdx_);
    }
    std::ostream& dump(std::ostream& os) const;
  };

  class ConstantMemberRef : public Constant {
  protected:
    uint16_t classIdx_;
    uint16_t nameAndTypeIdx_;
    ConstantMemberRef(const ClassFile* cf, std::istream& is);

  public:
    unsigned getClassIndex() const { return classIdx_; }
    ConstantClass* getClass() const {
      return parent_->getConstantClass(classIdx_);
    }
    unsigned getNameAndTypeIndex() const { return nameAndTypeIdx_; }
    ConstantNameAndType* getNameAndType() const {
      return parent_->getConstantNameAndType(nameAndTypeIdx_);
    }
  };

  class ConstantFieldRef : public ConstantMemberRef {
  public:
    ConstantFieldRef(const ClassFile* cf, std::istream& is)
      : ConstantMemberRef(cf, is) { }
    std::ostream& dump(std::ostream& os) const;
  };

  class ConstantMethodRef : public ConstantMemberRef {
  public:
    ConstantMethodRef(const ClassFile* cf, std::istream& is)
      : ConstantMemberRef(cf, is) { }
    std::ostream& dump(std::ostream& os) const;
  };

  class ConstantInterfaceMethodRef : public ConstantMemberRef {
  public:
    ConstantInterfaceMethodRef(const ClassFile* cf, std::istream& is)
      : ConstantMemberRef(cf, is) { }
    std::ostream& dump(std::ostream& os) const;
  };

  class ConstantString : public Constant {
    uint16_t stringIdx_;
  public:
    ConstantString(const ClassFile* cf, std::istream& is);
    unsigned getStringIndex() const { return stringIdx_; }
    ConstantUtf8* getValue() const {
      return parent_->getConstantUtf8(stringIdx_);
    }
    std::ostream& dump(std::ostream& os) const;
  };

  class ConstantInteger : public Constant {
    int32_t value_;
  public:
    ConstantInteger(const ClassFile* cf, std::istream& is);
    int32_t getValue() const { return value_; }
    std::ostream& dump(std::ostream& os) const;
  };

  class ConstantFloat : public Constant {
    float value_;
  public:
    ConstantFloat(const ClassFile* cf, std::istream& is);
    float getValue() const { return value_; }
    std::ostream& dump(std::ostream& os) const;
  };

  class ConstantLong : public Constant {
    int64_t value_;
  public:
    ConstantLong(const ClassFile* cf, std::istream& is);
    virtual bool isSingleSlot() { return false; }
    int64_t getValue() const { return value_; }
    std::ostream& dump(std::ostream& os) const;
  };

  class ConstantDouble : public Constant {
    double value_;
  public:
    ConstantDouble(const ClassFile* cf, std::istream& is);
    virtual bool isSingleSlot() { return false; }
    double getValue() const { return value_; }
    std::ostream& dump(std::ostream& os) const;
  };

  class ConstantNameAndType : public Constant {
    uint16_t nameIdx_;
    uint16_t descriptorIdx_;
  public:
    ConstantNameAndType(const ClassFile* cf, std::istream& is);
    unsigned getNameIndex() const { return nameIdx_; }
    ConstantUtf8* getName() const {
      return parent_->getConstantUtf8(nameIdx_);
    }
    unsigned getDescriptorIndex() const { return descriptorIdx_; }
    ConstantUtf8* getDescriptor() const {
      return parent_->getConstantUtf8(descriptorIdx_);
    }
    std::ostream& dump(std::ostream& os) const;
  };

  class ConstantUtf8 : public Constant {
    std::string utf8_;
  public:
    ConstantUtf8(const ClassFile* cf, std::istream& is);
    const std::string& str() const { return utf8_; }

    std::ostream& dump(std::ostream& os) const;
  };

  class Member {
  protected:
    const ClassFile* parent_;
    uint16_t accessFlags_;
    uint16_t nameIdx_;
    uint16_t descriptorIdx_;
    Attributes attributes_;

    Member(const ClassFile* parent, std::istream& is);
    ~Member();

public:
    bool isPublic() const { return accessFlags_ & ACC_PUBLIC; }
    bool isPrivate() const { return accessFlags_ & ACC_PRIVATE; }
    bool isProtected() const { return accessFlags_ & ACC_PROTECTED; }
    bool isStatic() const { return accessFlags_ & ACC_STATIC; }
    bool isFinal() const { return accessFlags_ & ACC_FINAL; }

    const ClassFile* getParent() const { return parent_; }
    unsigned getNameIndex() const { return nameIdx_; }
    ConstantUtf8* getName() const { return parent_->getConstantUtf8(nameIdx_); }
    unsigned getDescriptorIndex() const { return descriptorIdx_; }
    ConstantUtf8* getDescriptor() const {
      return parent_->getConstantUtf8(descriptorIdx_);
    }
    const Attributes& getAttributes() const { return attributes_; }
  };

  class Field : public Member {
  private:
    Field(const ClassFile* parent, std::istream& is);

  public:
    static Field* readField(const ClassFile* parent, std::istream& is) {
      return new Field(parent, is);
    }

    ~Field();

    bool isVolatile() const { return accessFlags_ & ACC_VOLATILE; }
    bool isTransient() const { return accessFlags_ & ACC_TRANSIENT; }

    ConstantValueAttribute* getConstantValueAttribute() const;

    std::ostream& dump(std::ostream& os) const;
  };

  inline std::ostream& operator<<(std::ostream& os, const Field& f) {
    return f.dump(os);
  }

  class Method : public Member {
    Method(const ClassFile* parent, std::istream& is);

  public:
    static Method* readMethod(const ClassFile* parent, std::istream& is) {
      return new Method(parent, is);
    }

    ~Method();

    bool isSynchronized() const { return accessFlags_ & ACC_SYNCHRONIZED; }
    bool isNative() const { return accessFlags_ & ACC_NATIVE; }
    bool isAbstract() const { return accessFlags_ & ACC_ABSTRACT; }
    bool isStrict() const { return accessFlags_ & ACC_STRICT; }

    CodeAttribute* getCodeAttribute() const;
    ExceptionsAttribute* getExceptionsAttribute() const;

    std::ostream& dump(std::ostream& os) const;
  };

  inline std::ostream& operator<<(std::ostream& os, const Method& m) {
    return m.dump(os);
  }

  class Attribute {
  protected:
    const ClassFile* parent_;
    uint16_t nameIdx_;

    Attribute(const ClassFile* cf, uint16_t nameIdx, std::istream& is);

  public:
    static Attribute* readAttribute(const ClassFile* cf, std::istream& is);

    virtual ~Attribute();

    unsigned getNameIndex() const { return nameIdx_; }
    ConstantUtf8* getName() const { return parent_->getConstantUtf8(nameIdx_); }

    virtual std::ostream& dump(std::ostream& os) const;

    static const std::string CONSTANT_VALUE;
    static const std::string CODE;
    static const std::string EXCEPTIONS;
    static const std::string INNER_CLASSES;
    static const std::string SYNTHETIC;
    static const std::string SOURCE_FILE;
    static const std::string LINE_NUMBER_TABLE;
    static const std::string LOCAL_VARIABLE_TABLE;
    static const std::string DEPRECATED;
  };

  inline std::ostream& operator<<(std::ostream& os, const Attribute& a) {
    return a.dump(os);
  }

  class ConstantValueAttribute : public Attribute {
    uint16_t valueIdx_;
  public:
    ConstantValueAttribute(const ClassFile* cf,
                           uint16_t nameIdx,
                           std::istream& is);

    unsigned getValueIndex() const { return valueIdx_; }
    Constant* getValue() const { return parent_->getConstant(valueIdx_); }

    std::ostream& dump(std::ostream& os) const;
  };

  class CodeAttribute : public Attribute {
  public:
    class Exception {
      const ClassFile* parent_;
      uint16_t startPc_;
      uint16_t endPc_;
      uint16_t handlerPc_;
      uint16_t catchTypeIdx_;

    public:
      Exception(const ClassFile* cf, std::istream& is);

      uint16_t getStartPc() const { return startPc_; }
      uint16_t getEndPc() const { return endPc_; }
      uint16_t getHandlerPc() const { return handlerPc_; }
      uint16_t getCatchTypeIndex() const { return catchTypeIdx_; }
      ConstantClass* getCatchType() const {
        return catchTypeIdx_ ? NULL : parent_->getConstantClass(catchTypeIdx_);
      }

      std::ostream& dump(std::ostream& os) const;
    };

    typedef std::vector<Exception*> Exceptions;

  private:
    uint16_t maxStack_;
    uint16_t maxLocals_;
    uint32_t codeSize_;
    uint8_t* code_;
    Exceptions exceptions_;
    Attributes attributes_;

  public:
    CodeAttribute(const ClassFile* cf, uint16_t nameIdx, std::istream& is);
    ~CodeAttribute();
    uint16_t getMaxStack() const { return maxStack_; }
    uint16_t getMaxLocals() const { return maxLocals_; }
    const uint8_t* getCode() const { return code_; }
    uint32_t getCodeSize() const { return codeSize_; }
    const Exceptions& getExceptions() const { return exceptions_; }
    const Attributes& getAttributes() const { return attributes_; }

    std::ostream& dump(std::ostream& os) const;
  };

  inline std::ostream& operator<<(std::ostream& os,
                                  const CodeAttribute::Exception& e) {
    return e.dump(os);
  }

  class ExceptionsAttribute : public Attribute {
  private:
    uint16_t nameIdx_;
    std::vector<uint16_t> exceptions_;

  public:
    ExceptionsAttribute(const ClassFile* cf,
                        uint16_t nameIdx,
                        std::istream& is);

    unsigned getNumExceptions() const { return exceptions_.size(); }
    unsigned getExceptionIndex(unsigned i) const { return exceptions_[i]; }
    ConstantClass* getException(unsigned i) const {
      return parent_->getConstantClass(getExceptionIndex(i));
    }

    std::ostream& dump(std::ostream& os) const;
  };

  class ClassFileParseError : public std::exception {
    std::string msg_;
  public:
    explicit ClassFileParseError(const std::string& msg) : msg_(msg) { }
    virtual ~ClassFileParseError() throw();
    virtual const char* what() const throw() { return msg_.c_str(); }
  };

  class ClassFileSemanticError : public std::exception {
    std::string msg_;
  public:
    explicit ClassFileSemanticError(const std::string& msg) : msg_(msg) { }
    virtual ~ClassFileSemanticError() throw();
    virtual const char* what() const throw() { return msg_.c_str(); }
  };

  class ClassNotFoundException : public std::exception {
    std::string msg_;
  public:
    explicit ClassNotFoundException(const std::string& msg) : msg_(msg) { }
    virtual ~ClassNotFoundException() throw();
    virtual const char* what() const throw() { return msg_.c_str(); }
  };

  class InvocationTargetException : public std::exception {
    std::string msg_;
  public:
    explicit InvocationTargetException(const std::string& msg) : msg_(msg) { }
    virtual ~InvocationTargetException() throw();
    virtual const char* what() const throw() { return msg_.c_str(); }
  };

} } // namespace llvm::Java

#endif//LLVM_JAVA_CLASSFILE_H

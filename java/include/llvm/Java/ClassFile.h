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

#include <iosfwd>
#include <map>
#include <stdexcept>
#include <vector>

#include <stdint.h>

namespace llvm { namespace Java {

    // Forward declarations
    class Attribute;
    class CodeAttribute;
    class Constant;
    class ConstantClass;
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
    typedef std::vector<ConstantClass*> Classes;
    typedef std::vector<Field*> Fields;
    typedef std::vector<Method*> Methods;
    typedef std::vector<Attribute*> Attributes;

    const Attribute* getAttribute(const Attributes& attrs,
                                  const std::string& name);
    const CodeAttribute* getCodeAttribute(const Attributes& attrs);

    class ClassFile {
        static const ClassFile* readClassFile(std::istream& is);
        static std::vector<std::string> getClassPath();
        static std::string getFileForClass(const std::string& classname);

        typedef std::map<std::string, Method*> Name2MethodMap;

    public:
        static const ClassFile* getClassFile(const std::string& classname);

        ~ClassFile();

        uint16_t getMinorVersion() const { return minorV_; }
        uint16_t getMajorVersion() const { return majorV_; }

        const ConstantPool& getConstantPool() const { return cPool_; }

        bool isPublic() const { return accessFlags_ & ACC_PUBLIC; }
        bool isFinal() const { return accessFlags_ & ACC_FINAL; }
        bool isSuper() const { return accessFlags_ & ACC_SUPER; }
        bool isInterface() const { return accessFlags_ & ACC_INTERFACE; }
        bool isAbstract() const { return accessFlags_ & ACC_ABSTRACT; }

        const ConstantClass* getThisClass() const { return thisClass_; }
        const ConstantClass* getSuperClass() const { return superClass_; }

        const Classes& getInterfaces() const { return interfaces_; }

        const Fields& getFields() const { return fields_; }

        const Methods& getMethods() const { return methods_; }

        const Attributes& getAttributes() const { return attributes_; }

        const Method* getMethod(const std::string& nameAndDescr) const;

        std::ostream& dump(std::ostream& os) const;

    private:
        uint16_t majorV_;
        uint16_t minorV_;
        ConstantPool cPool_;
        uint16_t accessFlags_;
        ConstantClass* thisClass_;
        ConstantClass* superClass_;
        Classes interfaces_;
        Fields fields_;
        Methods methods_;
        Attributes attributes_;
        Name2MethodMap n2mMap_;

        ClassFile(std::istream& is);
    };

    inline std::ostream& operator<<(std::ostream& os, const ClassFile& c) {
        return c.dump(os);
    }

    class Constant {
    protected:
        const ConstantPool& cPool_;

        Constant(const ConstantPool& cp)
            : cPool_(cp) { }

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

        static Constant* readConstant(const ConstantPool& cp,
                                      std::istream& is);

        virtual bool isSingleSlot() { return true; }
        bool isDoubleSlot() { return !isSingleSlot(); }
        virtual ~Constant();

        virtual std::ostream& dump(std::ostream& os) const = 0;
    };

    inline std::ostream& operator<<(std::ostream& os, const Constant& c) {
        return c.dump(os);
    }

    class ConstantClass : public Constant {
        uint16_t nameIdx_;
    public:
        ConstantClass(const ConstantPool& cp, std::istream& is);
        const ConstantUtf8* getName() const {
            return (const ConstantUtf8*) cPool_[nameIdx_];
        }
        std::ostream& dump(std::ostream& os) const;
    };

    class ConstantMemberRef : public Constant {
        uint16_t classIdx_;
        uint16_t nameAndTypeIdx_;
    protected:
        ConstantMemberRef(const ConstantPool& cp, std::istream& is);

    public:
        const ConstantClass* getClass() const {
            return (const ConstantClass*) cPool_[classIdx_];
        }
        const ConstantNameAndType* getNameAndType() const {
            return (const ConstantNameAndType*) cPool_[nameAndTypeIdx_];
        }
        std::ostream& dump(std::ostream& os) const;
    };

    struct ConstantFieldRef : public ConstantMemberRef {
        ConstantFieldRef(const ConstantPool& cp, std::istream& is)
            : ConstantMemberRef(cp, is) { }
    };

    struct ConstantMethodRef : public ConstantMemberRef {
        ConstantMethodRef(const ConstantPool& cp, std::istream& is)
            : ConstantMemberRef(cp, is) { }
    };

    struct ConstantInterfaceMethodRef : public ConstantMemberRef {
        ConstantInterfaceMethodRef(const ConstantPool& cp, std::istream& is)
            : ConstantMemberRef(cp, is) { }
    };

    class ConstantString : public Constant {
        uint16_t stringIdx_;
    public:
        ConstantString(const ConstantPool& cp, std::istream& is);
        const ConstantUtf8* getValue() const {
            return  (const ConstantUtf8*) cPool_[stringIdx_];
        }
        std::ostream& dump(std::ostream& os) const;
    };

    class ConstantInteger : public Constant {
        int32_t value_;
    public:
        ConstantInteger(const ConstantPool& cp, std::istream& is);
        int32_t getValue() const { return value_; }
        std::ostream& dump(std::ostream& os) const;
    };

    class ConstantFloat : public Constant {
        float value_;
    public:
        ConstantFloat(const ConstantPool& cp, std::istream& is);
        float getValue() const { return value_; }
        std::ostream& dump(std::ostream& os) const;
    };

    class ConstantLong : public Constant {
        int64_t value_;
    public:
        ConstantLong(const ConstantPool& cp, std::istream& is);
        virtual bool isSingleSlot() { return false; }
        int64_t getValue() const { return value_; }
        std::ostream& dump(std::ostream& os) const;
    };

    class ConstantDouble : public Constant {
        double value_;
    public:
        ConstantDouble(const ConstantPool& cp, std::istream& is);
        virtual bool isSingleSlot() { return false; }
        double getValue() const { return value_; }
        std::ostream& dump(std::ostream& os) const;
    };

    class ConstantNameAndType : public Constant {
        uint16_t nameIdx_;
        uint16_t descriptorIdx_;
    public:
        ConstantNameAndType(const ConstantPool& cp, std::istream& is);
        const ConstantUtf8* getName() const {
            return (const ConstantUtf8*) cPool_[nameIdx_];
        }
        const ConstantUtf8* getDescriptor() const {
            return (const ConstantUtf8*) cPool_[descriptorIdx_];
        }
        std::ostream& dump(std::ostream& os) const;
    };

    class ConstantUtf8 : public Constant {
        std::string utf8_;
    public:
        ConstantUtf8(const ConstantPool& cp, std::istream& is);
        const std::string& str() const { return utf8_; }

        std::ostream& dump(std::ostream& os) const;
    };

    class Field {
    private:
        uint16_t accessFlags_;
        ConstantUtf8* name_;
        ConstantUtf8* descriptor_;
        Attributes attributes_;

        Field(const ConstantPool& cp, std::istream& is);

    public:
        static Field* readField(const ConstantPool& cp, std::istream& is) {
            return new Field(cp, is);
        }

        ~Field();

        bool isPublic() const { return accessFlags_ & ACC_PUBLIC; }
        bool isPrivate() const { return accessFlags_ & ACC_PRIVATE; }
        bool isProtected() const { return accessFlags_ & ACC_PROTECTED; }
        bool isStatic() const { return accessFlags_ & ACC_STATIC; }
        bool isFinal() const { return accessFlags_ & ACC_FINAL; }
        bool isVolatile() const { return accessFlags_ & ACC_VOLATILE; }
        bool isTransient() const { return accessFlags_ & ACC_TRANSIENT; }

        ConstantUtf8* getName() const { return name_; }
        ConstantUtf8* getDescriptor() const { return descriptor_; }
        const Attributes& getAttributes() const { return attributes_; }

        std::ostream& dump(std::ostream& os) const;
    };

    inline std::ostream& operator<<(std::ostream& os, const Field& f) {
        return f.dump(os);
    }

    class Method {
        uint16_t accessFlags_;
        ConstantUtf8* name_;
        ConstantUtf8* descriptor_;
        Attributes attributes_;

        Method(const ConstantPool& cp, std::istream& is);

    public:
        static Method* readMethod(const ConstantPool& cp, std::istream& is) {
            return new Method(cp, is);
        }

        ~Method();

        bool isPublic() const { return accessFlags_ & ACC_PUBLIC; }
        bool isPrivate() const { return accessFlags_ & ACC_PRIVATE; }
        bool isProtected() const { return accessFlags_ & ACC_PROTECTED; }
        bool isStatic() const { return accessFlags_ & ACC_STATIC; }
        bool isFinal() const { return accessFlags_ & ACC_FINAL; }
        bool isSynchronized() const { return accessFlags_ & ACC_SYNCHRONIZED; }
        bool isNative() const { return accessFlags_ & ACC_NATIVE; }
        bool isAbstract() const { return accessFlags_ & ACC_ABSTRACT; }
        bool isStrict() const { return accessFlags_ & ACC_STRICT; }

        ConstantUtf8* getName() const { return name_; }
        ConstantUtf8* getDescriptor() const { return descriptor_; }
        const Attributes& getAttributes() const { return attributes_; }

        std::ostream& dump(std::ostream& os) const;
    };

    inline std::ostream& operator<<(std::ostream& os, const Method& m) {
        return m.dump(os);
    }

    class Attribute {
        ConstantUtf8* name_;

    protected:
        Attribute(ConstantUtf8* name,
                  const ConstantPool& cp,
                  std::istream& is);

    public:
        static Attribute* readAttribute(const ConstantPool& cp,
                                        std::istream& is);

        virtual ~Attribute();

        ConstantUtf8* getName() const { return name_; }

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
        Constant* value_;
    public:
        ConstantValueAttribute(ConstantUtf8* name,
                               const ConstantPool& cp,
                               std::istream& is);

        Constant* getValue() const { return value_; }

        std::ostream& dump(std::ostream& os) const;
    };

    class CodeAttribute : public Attribute {
    public:
        class Exception {
            uint16_t startPc_;
            uint16_t endPc_;
            uint16_t handlerPc_;
            ConstantClass* catchType_;

        public:
            Exception(const ConstantPool& cp, std::istream& is);

            uint16_t getStartPc() const { return startPc_; }
            uint16_t getEndPc() const { return endPc_; }
            uint16_t getHandlerPc() const { return handlerPc_; }
            ConstantClass* getCatchType() const { return catchType_; }

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
        CodeAttribute(ConstantUtf8* name,
                      const ConstantPool& cp,
                      std::istream& is);
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
        ConstantUtf8* name_;
        Classes exceptions_;

    public:
        ExceptionsAttribute(ConstantUtf8* name,
                            const ConstantPool& cp,
                            std::istream& is);

        const Classes& getExceptions() const { return exceptions_; }

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

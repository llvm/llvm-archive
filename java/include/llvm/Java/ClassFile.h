//===-- ClassFile.h - ClassFile support library -----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the ClassFileReader class,
// which implements a ClassFile reader parser for use with the Java
// frontend.
//
//===----------------------------------------------------------------------===//

#include <iosfwd>
#include <stdexcept>
#include <vector>

#include <stdint.h>

namespace llvm { namespace Java {

    // Forward declarations
    class Attribute;
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

    class ClassFile {
    public:
        typedef std::vector<Constant*> ConstantPool;
        typedef std::vector<ConstantClass*> Interfaces;
        typedef std::vector<Field*> Fields;
        typedef std::vector<Method*> Methods;
        typedef std::vector<Attribute*> Attributes;

    public:
        static ClassFile* readClassFile(std::istream& is);

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

        const Interfaces& getInterfaces() const;

        const Fields& getFields() const;

        const Methods& getMethods() const;

        const Attributes& getAttributes() const;

        std::ostream& dump(std::ostream& os) const;

    private:
        uint16_t majorV_;
        uint16_t minorV_;
        ConstantPool cPool_;
        uint16_t accessFlags_;
        ConstantClass* thisClass_;
        ConstantClass* superClass_;
        Interfaces interfaces_;
        Fields fields_;
        Methods methods_;
        Attributes attributes_;

        ClassFile(std::istream& is);
    };

    class Constant {
    protected:
        const ClassFile::ConstantPool& cPool_;

        Constant(const ClassFile::ConstantPool& cp)
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

        static Constant* readConstant(const ClassFile::ConstantPool& cp,
                                      std::istream& is);

        virtual ~Constant();

        virtual std::ostream& dump(std::ostream& os) const;
    };

    class ConstantClass : public Constant {
        uint16_t nameIdx_;
    public:
        ConstantClass(const ClassFile::ConstantPool& cp, std::istream& is);
        const ConstantUtf8* getName() const {
            return (const ConstantUtf8*) cPool_[nameIdx_];
        }
    };

    class ConstantMemberRef : public Constant {
        uint16_t classIdx_;
        uint16_t nameAndTypeIdx_;
    protected:
        ConstantMemberRef(const ClassFile::ConstantPool& cp, std::istream& is);

    public:
        const ConstantClass* getClass() const {
            return (const ConstantClass*) cPool_[classIdx_];
        }
        const ConstantNameAndType* getNameAndType() const {
            return (const ConstantNameAndType*) cPool_[nameAndTypeIdx_];
        }
    };

    struct ConstantFieldRef : public ConstantMemberRef {
        ConstantFieldRef(const ClassFile::ConstantPool& cp, std::istream& is)
            : ConstantMemberRef(cp, is) { }
    };

    struct ConstantMethodRef : public ConstantMemberRef {
        ConstantMethodRef(const ClassFile::ConstantPool& cp, std::istream& is)
            : ConstantMemberRef(cp, is) { }
    };

    struct ConstantInterfaceMethodRef : public ConstantMemberRef {
        ConstantInterfaceMethodRef(const ClassFile::ConstantPool& cp,
                                   std::istream& is)
            : ConstantMemberRef(cp, is) { }
    };

    class ConstantString : public Constant {
        uint16_t stringIdx_;
    public:
        ConstantString(const ClassFile::ConstantPool& cp, std::istream& is);
        const ConstantUtf8* getValue() const {
            return  (const ConstantUtf8*) cPool_[stringIdx_];
        }
    };

    class ConstantInteger : public Constant {
        int32_t value_;
    public:
        ConstantInteger(const ClassFile::ConstantPool& cp, std::istream& is);
        int32_t getValue() const { return value_; }
    };

    class ConstantFloat : public Constant {
        float value_;
    public:
        ConstantFloat(const ClassFile::ConstantPool& cp, std::istream& is);
        float getValue() const { return value_; }
    };

    class ConstantLong : public Constant {
        int64_t value_;
    public:
        ConstantLong(const ClassFile::ConstantPool& cp, std::istream& is);
        int64_t getValue() const { return value_; }
    };

    class ConstantDouble : public Constant {
        double value_;
    public:
        ConstantDouble(const ClassFile::ConstantPool& cp, std::istream& is);
        double getValue() const { return value_; }
    };

    class ConstantNameAndType : public Constant {
        uint16_t nameIdx_;
        uint16_t descriptorIdx_;
    public:
        ConstantNameAndType(const ClassFile::ConstantPool& cp, std::istream& is);
        const ConstantUtf8* getName() const {
            return (const ConstantUtf8*) cPool_[nameIdx_];
        }
        const ConstantUtf8* getDescriptor() const {
            return (const ConstantUtf8*) cPool_[descriptorIdx_];
        }
    };

    class ConstantUtf8 : public Constant {
        std::string utf8_;
    public:
        ConstantUtf8(const ClassFile::ConstantPool& cp, std::istream& is);
        //const std::string& getUtf8() const { return utf8_; }
    };

    class Field {
    public:
        typedef std::vector<Attribute*> Attributes;

    private:
        uint16_t accessFlags_;
        ConstantUtf8* name_;
        ConstantUtf8* descriptor_;
        Attributes attrs_;

        Field(const ClassFile::ConstantPool& cp, std::istream& is);

    public:
        static Field* readField(const ClassFile::ConstantPool& cp,
                                std::istream& is) {
            return new Field(cp, is);
        }

        bool isPublic() const { return accessFlags_ & ACC_PUBLIC; }
        bool isPrivate() const { return accessFlags_ & ACC_PRIVATE; }
        bool isProtected() const { return accessFlags_ & ACC_PROTECTED; }
        bool isStatic() const { return accessFlags_ & ACC_STATIC; }
        bool isFinal() const { return accessFlags_ & ACC_FINAL; }
        bool isVolatile() const { return accessFlags_ & ACC_VOLATILE; }
        bool isTransient() const { return accessFlags_ & ACC_TRANSIENT; }

        ConstantUtf8* getName() const { return name_; }
        ConstantUtf8* getDescriptorIdx() const { return descriptor_; }

        std::ostream& dump(std::ostream& os) const;
    };

    class Method {
        uint16_t accessFlags_;
        ConstantUtf8* name_;
        ConstantUtf8* descriptor_;
        ClassFile::Attributes attrs_;

        Method(const ClassFile::ConstantPool& cp, std::istream& is);

    public:
        static Method* readMethod(const ClassFile::ConstantPool& cp,
                                  std::istream& is) {
            return new Method(cp, is);
        }

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

        std::ostream& dump(std::ostream& os) const;
    };

    class Attribute {
        ConstantUtf8* name_;

    protected:
        Attribute(const ClassFile::ConstantPool& cp, std::istream& is);

    public:
        static Attribute* readAttribute(const ClassFile::ConstantPool& cp,
                                        std::istream& is);

        ConstantUtf8* getName() const { return name_; }

        std::ostream& dump(std::ostream& os) const;
    };

    class ClassFileParseError : public std::exception {
        std::string msg_;
    public:
        explicit ClassFileParseError(const std::string& msg) : msg_(msg) { }
        virtual ~ClassFileParseError() throw();
        virtual const char* what() const throw() { return msg_.c_str(); }
    };

} } // namespace llvm::Java

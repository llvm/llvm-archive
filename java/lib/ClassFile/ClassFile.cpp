//===-- ClassFile.cpp - ClassFile class -----------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the implementation of the ClassFile library. It
// is used by the LLVM Java frontend to parse Java Class files.
//
//===----------------------------------------------------------------------===//

#include <llvm/Java/ClassFile.h>

#include <cassert>
#include <functional>
#include <iostream>
#include <iterator>

using namespace llvm::Java;

//===----------------------------------------------------------------------===//
// Utility functions
namespace {

    uint8_t readU1(std::istream& is) {
        char val;
        if (!is.get(val))
            throw ClassFileParseError("unexpected end of input");
        return val;
    }

    uint16_t readU2(std::istream& is) {
        uint16_t val = readU1(is);
        return (val << 8) | readU1(is);
    }

    uint32_t readU4(std::istream& is) {
        uint32_t val = readU2(is);
        return (val << 16) | readU2(is);
    }

    uint64_t readU8(std::istream& is) {
        uint64_t hi = readU4(is), lo = readU4(is);
        return hi << 32 | lo;
    }

    float int2float(uint32_t v) {
        union { uint32_t in; float out; } tmp;
        tmp.in = v;
        return tmp.out;
    }

    double long2double(uint64_t v) {
        union { uint64_t in; double out; } tmp;
        tmp.in = v;
        return tmp.out;
    }

    void readConstantPool(ClassFile::ConstantPool& cp, std::istream& is)
    {
        assert(cp.empty() && "Should not call with a non-empty constant pool");
        uint16_t count = readU2(is);
        cp.reserve(count);
        cp.push_back(NULL);
        --count;
        while (count--)
            cp.push_back(Constant::readConstant(cp, is));
    }

    void readInterfaces(ClassFile::Interfaces& i,
                        const ClassFile::ConstantPool& cp,
                        std::istream& is)
    {
        assert(i.empty() &&
               "Should not call with a non-empty interfaces vector");
        uint16_t count = readU2(is);
        i.reserve(count);
        while (count--) {
            ConstantClass* c = dynamic_cast<ConstantClass*>(cp[readU2(is)]);
            if (!c) throw "FIXME: give better error message";
            i.push_back(c);
        }
    }

    void readFields(ClassFile::Fields& f,
                    const ClassFile::ConstantPool& cp,
                    std::istream& is)
    {
        assert(f.empty() && "Should not call with a non-empty fields vector");
        uint16_t count = readU2(is);
        f.reserve(count);
        while(count--)
            f.push_back(Field::readField(cp, is));
    }

    void readMethods(ClassFile::Methods& m,
                     const ClassFile::ConstantPool& cp,
                     std::istream& is)
    {
        assert(m.empty() && "Should not call with a non-empty methods vector");
        uint16_t count = readU2(is);
        m.reserve(count);
        while(count--)
            m.push_back(Method::readMethod(cp, is));
    }

    void readAttributes(ClassFile::Attributes& a,
                        const ClassFile::ConstantPool& cp,
                        std::istream& is)
    {
        assert(a.empty() &&
               "Should not call with a non-empty attributes vector");
        uint16_t count = readU2(is);
        a.reserve(count);
        while(count--)
            a.push_back(Attribute::readAttribute(cp, is));
    }

    template <typename Container>
    std::ostream& dumpCollection(Container& c,
                                 const char* const name,
                                 std::ostream& os) {
        os << '\n' << name << "s:\n";
        for (typename Container::const_iterator
                 i = c.begin(), e = c.end(); i != e; ++i)
            (*i)->dump(os << name << ' ') << '\n';
        return os;
    }

}

//===----------------------------------------------------------------------===//
// ClassFile implementation
ClassFile* ClassFile::readClassFile(std::istream& is)
{
    if (readU1(is) != 0xCA) throw ClassFileParseError("bad magic");
    if (readU1(is) != 0xFE) throw ClassFileParseError("bad magic");
    if (readU1(is) != 0xBA) throw ClassFileParseError("bad magic");
    if (readU1(is) != 0xBE) throw ClassFileParseError("bad magic");

    return new ClassFile(is);
}

ClassFile::ClassFile(std::istream& is)
{
    minorV_ = readU2(is);
    majorV_ = readU2(is);
    readConstantPool(cPool_, is);
    accessFlags_ = readU2(is);
    thisClass_ = dynamic_cast<ConstantClass*>(cPool_[readU2(is)]);
    if (!thisClass_)
        throw ClassFileSemanticError(
            "Representation of this class is not of type ConstantClass");
    superClass_ = dynamic_cast<ConstantClass*>(cPool_[readU2(is)]);
    if (!superClass_)
        throw ClassFileSemanticError(
            "Representation of super class is not of type ConstantClass");
    readInterfaces(interfaces_, cPool_, is);
    readFields(fields_, cPool_, is);
    readMethods(methods_, cPool_, is);
    readAttributes(attributes_, cPool_, is);
}

std::ostream& ClassFile::dump(std::ostream& os) const
{
    os << "Minor version: " << getMinorVersion() << '\n'
       << "Major version: " << getMajorVersion() << "\n\n"
       << "class " << *getThisClass() << " (" << *getSuperClass() << ")\n"
       << "Flags:";
    if (isPublic()) os << " public";
    if (isFinal()) os << " final";
    if (isSuper()) os << " super";
    if (isInterface()) os << " interface";
    if (isAbstract()) os << " abstract";

    dumpCollection(interfaces_, "Interface", os);
    dumpCollection(fields_, "Field", os);
    dumpCollection(methods_, "Method", os);
    dumpCollection(attributes_, "Attribute", os);

    return os;
}

//===----------------------------------------------------------------------===//
// ClassFileParseError implementation
ClassFileParseError::~ClassFileParseError() throw()
{

}

//===----------------------------------------------------------------------===//
// ClassFileSemanticError implementation
ClassFileSemanticError::~ClassFileSemanticError() throw()
{

}

//===----------------------------------------------------------------------===//
// Constant implementation
Constant* Constant::readConstant(const ClassFile::ConstantPool& cp,
                                 std::istream& is)
{
    Constant::Tag tag = static_cast<Constant::Tag>(readU1(is));
    switch (tag) {
    case Constant::CLASS:
        return new ConstantClass(cp, is);
    case Constant::FIELD_REF:
        return new ConstantFieldRef(cp, is);
    case Constant::METHOD_REF:
        return new ConstantMethodRef(cp, is);
    case Constant::INTERFACE_METHOD_REF:
        return new ConstantInterfaceMethodRef(cp, is);
    case Constant::STRING:
        return new ConstantString(cp, is);
    case Constant::INTEGER:
        return new ConstantInteger(cp, is);
    case Constant::FLOAT:
        return new ConstantFloat(cp, is);
    case Constant::LONG:
        return new ConstantLong(cp, is);
    case Constant::DOUBLE:
        return new ConstantDouble(cp, is);
    case Constant::NAME_AND_TYPE:
        return new ConstantNameAndType(cp, is);
    case Constant::UTF8:
        return new ConstantUtf8(cp, is);
    default:
        assert(0 && "Unknown constant tag");
    }

    return NULL;
}

Constant::~Constant()
{

}

ConstantMemberRef::ConstantMemberRef(const ClassFile::ConstantPool&cp,
                                     std::istream& is)
    : Constant(cp),
      classIdx_(readU2(is)),
      nameAndTypeIdx_(readU2(is))
{

}

std::ostream& ConstantMemberRef::dump(std::ostream& os) const
{
    return os << *getNameAndType() << '(' << *getClass() << ')';
}

ConstantClass::ConstantClass(const ClassFile::ConstantPool& cp,
                             std::istream& is)
    : Constant(cp),
      nameIdx_(readU2(is))
{

}

std::ostream& ConstantClass::dump(std::ostream& os) const
{
    return os << *getName();
}

ConstantString::ConstantString(const ClassFile::ConstantPool& cp,
                               std::istream& is)
    : Constant(cp),
      stringIdx_(readU2(is))
{

}

std::ostream& ConstantString::dump(std::ostream& os) const
{
    return os << "string " << *getValue();
}

ConstantInteger::ConstantInteger(const ClassFile::ConstantPool& cp,
                                 std::istream& is)
    : Constant(cp),
      value_(static_cast<int32_t>(readU4(is)))
{

}

std::ostream& ConstantInteger::dump(std::ostream& os) const
{
    return os << value_;
}

ConstantFloat::ConstantFloat(const ClassFile::ConstantPool& cp,
                             std::istream& is)
    : Constant(cp),
      value_(int2float(readU4(is)))
{

}

std::ostream& ConstantFloat::dump(std::ostream& os) const
{
    return os << value_;
}

ConstantLong::ConstantLong(const ClassFile::ConstantPool& cp,
                           std::istream& is)
    : Constant(cp),
      value_(static_cast<int64_t>(readU8(is)))
{

}

std::ostream& ConstantLong::dump(std::ostream& os) const
{
    return os << value_;
}

ConstantDouble::ConstantDouble(const ClassFile::ConstantPool& cp,
                               std::istream& is)
    : Constant(cp),
      value_(long2double(readU8(is)))
{

}

std::ostream& ConstantDouble::dump(std::ostream& os) const
{
    return os << value_;
}

ConstantNameAndType::ConstantNameAndType(const ClassFile::ConstantPool& cp,
                                         std::istream& is)
    : Constant(cp),
      nameIdx_(readU2(is)),
      descriptorIdx_(readU2(is))
{

}

std::ostream& ConstantNameAndType::dump(std::ostream& os) const
{
    return os << *getDescriptor() << ' ' << *getName();
}

ConstantUtf8::ConstantUtf8(const ClassFile::ConstantPool& cp,
                           std::istream& is)
    : Constant(cp)
{
    uint16_t length = readU2(is);
    char buf[length];
    is.read(buf, length);
    utf8_ = std::string(buf, length);
}

std::ostream& ConstantUtf8::dump(std::ostream& os) const
{
    return os << utf8_;
}

//===----------------------------------------------------------------------===//
// Field implementation
Field::Field(const ClassFile::ConstantPool& cp, std::istream& is)
{
    accessFlags_ = readU2(is);
    if (!name_)
        throw ClassFileSemanticError(
            "Representation of field name is not of type ConstantUtf8");
    descriptor_ = dynamic_cast<ConstantUtf8*>(cp[readU2(is)]);
    if (!descriptor_)
        throw ClassFileSemanticError(
            "Representation of field descriptor is not of type ConstantUtf8");
    readAttributes(attributes_, cp, is);
}

std::ostream& Field::dump(std::ostream& os) const
{
    os << *getName() << ' ' << *getDescriptor() << '\n'
       << "Flags:";
    if (isPublic()) os << " public";
    if (isPrivate()) os << " private";
    if (isProtected()) os << " protected";
    if (isStatic()) os << " static";
    if (isFinal()) os << " final";
    if (isVolatile()) os << " volatile";
    if (isTransient()) os << " transient";

    dumpCollection(attributes_, "Attribute", os);

    return os;
}

//===----------------------------------------------------------------------===//
// Method implementation
Method::Method(const ClassFile::ConstantPool& cp, std::istream& is)
{
    accessFlags_ = readU2(is);
    name_ = dynamic_cast<ConstantUtf8*>(cp[readU2(is)]);
    if (!name_)
        throw ClassFileSemanticError(
            "Representation of method name is not of type ConstantUtf8");
    descriptor_ = dynamic_cast<ConstantUtf8*>(cp[readU2(is)]);
    if (!descriptor_)
        throw ClassFileSemanticError(
            "Representation of method descriptor is not of type ConstantUtf8");
    readAttributes(attributes_, cp, is);
}

std::ostream& Method::dump(std::ostream& os) const
{
    os << *getName() << ' ' << *getDescriptor() << '\n'
       << "Flags:";
    if (isPublic()) os << " public";
    if (isPrivate()) os << " private";
    if (isProtected()) os << " protected";
    if (isStatic()) os << " static";
    if (isFinal()) os << " final";
    if (isSynchronized()) os << " synchronized";
    if (isNative()) os << " native";
    if (isStrict()) os << " strict";

    dumpCollection(attributes_, "Attribute", os);

    return os;
}

//===----------------------------------------------------------------------===//
// Attribute implementation
Attribute::Attribute(const ClassFile::ConstantPool& cp, std::istream& is)
{
    name_ = dynamic_cast<ConstantUtf8*>(cp[readU2(is)]);
    if (!name_)
        throw ClassFileSemanticError(
            "Representation of attribute name is not of type ConstantUtf8");
    uint32_t length = readU4(is);
    is.ignore(length);
}

Attribute* Attribute::readAttribute(const ClassFile::ConstantPool& cp,
                                    std::istream& is)
{
    return new Attribute(cp, is);
}

std::ostream& Attribute::dump(std::ostream& os) const
{
    os << *getName() << '\n';
    return os;
}

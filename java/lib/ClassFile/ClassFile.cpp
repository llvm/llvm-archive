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

    char bool2cross(bool v)
    {
        return v ? 'x' : ' ';
    }

    uint8_t readU1(std::istream& is) {
        char val;
        if (!is.get(val))
            throw ClassFileParseError("unexpected end of input");
        return val;
    }

    uint16_t readU2(std::istream& is) {
        return (readU1(is) << 8) | readU1(is);
    }

    uint32_t readU4(std::istream& is) {
        return (readU2(is) << 16) | readU2(is);
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

};

//===----------------------------------------------------------------------===//
// ClassFileParseError implementation
ClassFileParseError::~ClassFileParseError() throw()
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

std::ostream& Constant::dump(std::ostream& os) const
{
    // FIXME
    return os;
}

ConstantMemberRef::ConstantMemberRef(const ClassFile::ConstantPool&cp,
                                     std::istream& is)
    : Constant(cp),
      classIdx_(readU2(is)),
      nameAndTypeIdx_(readU2(is))
{

}

ConstantClass::ConstantClass(const ClassFile::ConstantPool& cp,
                             std::istream& is)
    : Constant(cp),
      nameIdx_(readU2(is))
{

}

ConstantString::ConstantString(const ClassFile::ConstantPool& cp,
                               std::istream& is)
    : Constant(cp),
      stringIdx_(readU2(is))
{

}

ConstantInteger::ConstantInteger(const ClassFile::ConstantPool& cp,
                                 std::istream& is)
    : Constant(cp),
      value_(static_cast<int32_t>(readU4(is)))
{

}

ConstantFloat::ConstantFloat(const ClassFile::ConstantPool& cp,
                             std::istream& is)
    : Constant(cp),
      value_(int2float(readU4(is)))
{

}

ConstantLong::ConstantLong(const ClassFile::ConstantPool& cp,
                           std::istream& is)
    : Constant(cp),
      value_(static_cast<int64_t>(readU8(is)))
{

}

ConstantDouble::ConstantDouble(const ClassFile::ConstantPool& cp,
                               std::istream& is)
    : Constant(cp),
      value_(long2double(readU8(is)))
{

}

ConstantNameAndType::ConstantNameAndType(const ClassFile::ConstantPool& cp,
                                         std::istream& is)
    : Constant(cp),
      nameIdx_(readU2(is)),
      descriptorIdx_(readU2(is))
{

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

//===----------------------------------------------------------------------===//
// Field implementation
Field::Field(const ClassFile::ConstantPool& cp, std::istream& is)
{
    accessFlags_ = readU2(is);
    name_ = dynamic_cast<ConstantUtf8*>(cp[readU2(is)]);
    if (!name_) throw "FIXME: better error message";
    descriptor_ = dynamic_cast<ConstantUtf8*>(cp[readU2(is)]);
}

std::ostream& Field::dump(std::ostream& os) const
{
    // FIXME
    return os;
}

//===----------------------------------------------------------------------===//
// Method implementation
Method::Method(const ClassFile::ConstantPool& cp, std::istream& is)
{
    accessFlags_ = readU2(is);
    name_ = dynamic_cast<ConstantUtf8*>(cp[readU2(is)]);
    if (!name_) throw "FIXME: better error message";
    descriptor_ = dynamic_cast<ConstantUtf8*>(cp[readU2(is)]);
}

std::ostream& Method::dump(std::ostream& os) const
{
    // FIXME
    return os;
}

//===----------------------------------------------------------------------===//
// Attribute implementation
Attribute::Attribute(const ClassFile::ConstantPool& cp, std::istream& is)
{
    name_ = dynamic_cast<ConstantUtf8*>(cp[readU2(is)]);
    if (!name_) throw "FIXME: better error message";
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
    // FIXME
    return os;
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

namespace {

    void readConstantPool(ClassFile::ConstantPool& cp, std::istream& is)
    {
        assert(cp.empty() && "Should not call with a non-empty constant pool");
        uint16_t count = readU2(is) - 1;
        cp.reserve(count);
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

}

ClassFile::ClassFile(std::istream& is)
{
    minorV_ = readU2(is);
    majorV_ = readU2(is);
    readConstantPool(cPool_, is);
    accessFlags_ = readU2(is);
    thisClass_ = dynamic_cast<ConstantClass*>(cPool_[readU2(is)]);
    if (!thisClass_) throw "FIXME: better error message";
    superClass_ = dynamic_cast<ConstantClass*>(cPool_[readU2(is)]);
    if (!superClass_) throw "FIXME: better error message";
    readInterfaces(interfaces_, cPool_, is);
    readFields(fields_, cPool_, is);
    readMethods(methods_, cPool_, is);
    readAttributes(attributes_, cPool_, is);
}

std::ostream& ClassFile::dump(std::ostream& os) const
{
    os << "Minor version: " << getMinorVersion() << '\n'
       << "Major version: " << getMajorVersion() << '\n'
       << "Access flags (PFSIA): "
       << bool2cross(isPublic())
       << bool2cross(isFinal())

       << bool2cross(isSuper())
       << bool2cross(isInterface())
       << bool2cross(isAbstract()) << '\n'
       << "This class: " << getThisClass() << '\n'
       << "Super class: " << getSuperClass() << '\n';

    for (Interfaces::const_iterator
             i = interfaces_.begin(), e = interfaces_.end(); i != e; ++i)
        (*i)->dump(os);

    for (Fields::const_iterator
             i = fields_.begin(), e = fields_.end(); i != e; ++i)
        (*i)->dump(os);

    for (Methods::const_iterator
             i = methods_.begin(), e = methods_.end(); i != e; ++i)
        (*i)->dump(os);

    for (Attributes::const_iterator
             i = attributes_.begin(), e = attributes_.end(); i != e; ++i)
        (*i)->dump(os);

    return os;
}

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

#define DEBUG_TYPE "classfile"

#include <llvm/Java/ClassFile.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Debug.h>
#include <llvm/Config/alloca.h>

#include <algorithm>
#include <cassert>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>

using namespace llvm::Java;

namespace {

  using namespace llvm;

  static cl::opt<std::string>
  ClassPath("cp",
            cl::desc("A : separated list of directories"),
            cl::value_desc("class search path"),
            cl::init(getenv("CLASSPATH") ? getenv("CLASSPATH") : ""));
  static cl::alias
  ClassPathA("classpath",
             cl::desc("Alias for -cp"),
             cl::aliasopt(ClassPath));

}

//===----------------------------------------------------------------------===//
// Internal utility functions
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

  void readConstantPool(ConstantPool& cp, std::istream& is)
  {
    assert(cp.empty() && "Should not call with a non-empty constant pool");
    uint16_t count = readU2(is);
    cp.reserve(count);
    cp.push_back(NULL);
    while (cp.size() < count) {
      cp.push_back(Constant::readConstant(cp, is));
      if (cp.back()->isDoubleSlot())
        cp.push_back(NULL);
    }
  }

  void readClasses(Classes& i, const ConstantPool& cp, std::istream& is)
  {
    assert(i.empty() &&
           "Should not call with a non-empty classes vector");
    uint16_t count = readU2(is);
    i.reserve(count);
    while (count--) {
      ConstantClass* c = dynamic_cast<ConstantClass*>(cp[readU2(is)]);
      if (!c)
        throw ClassFileSemanticError("ConstantClass expected");
      i.push_back(c);
    }
  }

  void readFields(Fields& f,
                  ClassFile* parent,
                  const ConstantPool& cp,
                  std::istream& is)
  {
    assert(f.empty() && "Should not call with a non-empty fields vector");
    uint16_t count = readU2(is);
    f.reserve(count);
    while(count--)
      f.push_back(Field::readField(parent, cp, is));
  }

  void readMethods(Methods& m,
                   ClassFile* parent,
                   const ConstantPool& cp,
                   std::istream& is)
  {
    assert(m.empty() && "Should not call with a non-empty methods vector");
    uint16_t count = readU2(is);
    m.reserve(count);
    while(count--)
      m.push_back(Method::readMethod(parent, cp, is));
  }

  void readAttributes(Attributes& a,
                      const ConstantPool& cp,
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
      if (*i)
        (*i)->dump(os << name << ' ') << '\n';
      else
        os << name << " NULL\n";
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

std::vector<sys::Path> ClassFile::getClassPath()
{
  DEBUG(std::cerr << "CLASSPATH=" << ClassPath << '\n');

  std::vector<sys::Path> result;
  sys::Path path;
  unsigned b = 0, e = 0;
  do {
    e = ClassPath.find(':', b);
    if (path.setDirectory(ClassPath.substr(b, e - b))) {
      result.push_back(path);
      DEBUG(std::cerr << "Adding: " << path.toString() << " to CLASSPATH\n");
    }
    b = e + 1;
  } while (e != std::string::npos);

  return result;
}

sys::Path ClassFile::getFileForClass(const std::string& classname)
{
  static const std::vector<sys::Path> classpath = getClassPath();
  DEBUG(std::cerr << "Looking up class: " << classname << '\n');

  std::string clazz = classname;
  // replace '.' with '/'
  for (unsigned i = 0, e = clazz.size(); i != e; ++i)
    if (clazz[i] == '.')
      clazz[i] = '/';
  clazz += ".class";

  for (unsigned i = 0, e = classpath.size(); i != e; ++i) {
    sys::Path filename = classpath[i];
    assert(filename.isDirectory() && "CLASSPATH element not a directory!");
    filename.appendFile(clazz);
    DEBUG(std::cerr << "Trying file: " << filename.toString() << '\n');
    if (filename.exists())
      return filename;
  }

  throw ClassNotFoundException("Class " + classname + " not found");
}

ClassFile* ClassFile::get(const std::string& classname)
{
  typedef std::map<std::string, ClassFile*> Name2ClassMap;
  static Name2ClassMap n2cMap_;

  Name2ClassMap::iterator it = n2cMap_.lower_bound(classname);

  if (it == n2cMap_.end() || it->first != classname) {
    std::ifstream in(getFileForClass(classname).c_str());
    it = n2cMap_.insert(it, std::make_pair(classname, readClassFile(in)));
  }

  return it->second;
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
  if (!superClass_ && thisClass_->getName()->str() != "java/lang/Object")
    throw ClassFileSemanticError(
      "Representation of super class is not of type ConstantClass");
  readClasses(interfaces_, cPool_, is);
  readFields(fields_, this, cPool_, is);
  readMethods(methods_, this, cPool_, is);
  readAttributes(attributes_, cPool_, is);
  for (Methods::const_iterator
         i = methods_.begin(), e = methods_.end(); i != e; ++i)
    n2mMap_.insert(
      std::make_pair(
        (*i)->getName()->str() + (*i)->getDescriptor()->str(), *i));
}

ConstantClass* ClassFile::getConstantClass(unsigned index) const
{
  assert(dynamic_cast<ConstantClass*>(getConstant(index)) &&
         "Constant is not a ConstantClass!");
  return static_cast<ConstantClass*>(getConstant(index));
}

ConstantMemberRef* ClassFile::getConstantMemberRef(unsigned index) const
{
  assert(dynamic_cast<ConstantMemberRef*>(getConstant(index)) &&
         "Constant is not a ConstantMemberRef!");
  return static_cast<ConstantMemberRef*>(getConstant(index));
}

ConstantFieldRef* ClassFile::getConstantFieldRef(unsigned index) const
{
  assert(dynamic_cast<ConstantFieldRef*>(getConstant(index)) &&
         "Constant is not a ConstantFieldRef!");
  return static_cast<ConstantFieldRef*>(getConstant(index));
}

ConstantMethodRef* ClassFile::getConstantMethodRef(unsigned index) const
{
  assert(dynamic_cast<ConstantMethodRef*>(getConstant(index)) &&
         "Constant is not a ConstantMethodRef!");
  return static_cast<ConstantMethodRef*>(getConstant(index));
}

ConstantInterfaceMethodRef*
ClassFile::getConstantInterfaceMethodRef(unsigned index) const
{
  assert(dynamic_cast<ConstantInterfaceMethodRef*>(getConstant(index)) &&
         "Constant is not a ConstantInterfaceMethodRef!");
  return static_cast<ConstantInterfaceMethodRef*>(getConstant(index));
}

Method* ClassFile::getMethod(const std::string& nameAndDescr) const
{
  Name2MethodMap::const_iterator it = n2mMap_.find(nameAndDescr);
  return it == n2mMap_.end() ? NULL : it->second;
}

bool ClassFile::isNativeMethodOverloaded(const Method& method) const
{
  unsigned count = 0;
  for (Methods::const_iterator i = methods_.begin(), e = methods_.end();
       i != e; ++i)
    if ((*i)->isNative() && (*i)->getName() == method.getName())
      ++count;

  return count > 1;
}

ClassFile::~ClassFile()
{
  for_each(cPool_.begin(), cPool_.end(), deleter<Constant>);
  for_each(fields_.begin(), fields_.end(), deleter<Field>);
  for_each(methods_.begin(), methods_.end(), deleter<Method>);
  for_each(attributes_.begin(), attributes_.end(), deleter<Attribute>);
}

std::ostream& ClassFile::dump(std::ostream& os) const
{
  os << "Minor version: " << getMinorVersion() << '\n'
     << "Major version: " << getMajorVersion() << "\n\n"
     << "class " << *getThisClass();
  if (getSuperClass())
    os  << " (" << *getSuperClass() << ")\n";

  os << "Flags:";
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
// Utility functions
Attribute* llvm::Java::getAttribute(const Attributes& attrs,
                                    const std::string& name)
{
  for (unsigned i = 0, e = attrs.size(); i != e; ++i)
    if (attrs[i]->getName()->str() == name)
      return attrs[i];

  return NULL;
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
// ClassNotFoundException implementation
ClassNotFoundException::~ClassNotFoundException() throw()
{

}

//===----------------------------------------------------------------------===//
// InvocationTargetException implementation
InvocationTargetException::~InvocationTargetException() throw()
{

}

//===----------------------------------------------------------------------===//
// Constant implementation
Constant* Constant::readConstant(const ConstantPool& cp,
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

ConstantMemberRef::ConstantMemberRef(const ConstantPool&cp, std::istream& is)
  : Constant(cp),
    classIdx_(readU2(is)),
    nameAndTypeIdx_(readU2(is))
{

}

std::ostream& ConstantMemberRef::dump(std::ostream& os) const
{
  return os << *getNameAndType() << '(' << *getClass() << ')';
}

ConstantClass::ConstantClass(const ConstantPool& cp, std::istream& is)
  : Constant(cp),
    nameIdx_(readU2(is))
{

}

std::ostream& ConstantClass::dump(std::ostream& os) const
{
  return os << *getName();
}

ConstantString::ConstantString(const ConstantPool& cp, std::istream& is)
  : Constant(cp),
    stringIdx_(readU2(is))
{

}

std::ostream& ConstantString::dump(std::ostream& os) const
{
  return os << "String " << *getValue();
}

ConstantInteger::ConstantInteger(const ConstantPool& cp, std::istream& is)
  : Constant(cp),
    value_(static_cast<int32_t>(readU4(is)))
{

}

std::ostream& ConstantInteger::dump(std::ostream& os) const
{
  return os << "int " << value_;
}

ConstantFloat::ConstantFloat(const ConstantPool& cp, std::istream& is)
  : Constant(cp),
    value_(int2float(readU4(is)))
{

}

std::ostream& ConstantFloat::dump(std::ostream& os) const
{
  return os << "float " << value_;
}

ConstantLong::ConstantLong(const ConstantPool& cp, std::istream& is)
  : Constant(cp),
    value_(static_cast<int64_t>(readU8(is)))
{

}

std::ostream& ConstantLong::dump(std::ostream& os) const
{
  return os << "long " << value_;
}

ConstantDouble::ConstantDouble(const ConstantPool& cp, std::istream& is)
  : Constant(cp),
    value_(long2double(readU8(is)))
{

}

std::ostream& ConstantDouble::dump(std::ostream& os) const
{
  return os << "double " << value_;
}

ConstantNameAndType::ConstantNameAndType(const ConstantPool& cp,
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

ConstantUtf8::ConstantUtf8(const ConstantPool& cp, std::istream& is)
  : Constant(cp)
{
  uint16_t length = readU2(is);
  char *buf = (char *)alloca(length);
  std::streamsize s = is.rdbuf()->sgetn(buf, length);
  if (s != length)
    throw ClassFileParseError(
      "Could not read string constant from input stream");
  utf8_.assign(buf, length);
}

std::ostream& ConstantUtf8::dump(std::ostream& os) const
{
  return os << utf8_;
}

//===----------------------------------------------------------------------===//
// Field implementation
Field::Field(ClassFile* parent, const ConstantPool& cp, std::istream& is)
  : parent_(parent)
{
  accessFlags_ = readU2(is);
  name_ = dynamic_cast<ConstantUtf8*>(cp[readU2(is)]);
  if (!name_)
    throw ClassFileSemanticError(
      "Representation of field name is not of type ConstantUtf8");
  descriptor_ = dynamic_cast<ConstantUtf8*>(cp[readU2(is)]);
  if (!descriptor_)
    throw ClassFileSemanticError(
      "Representation of field descriptor is not of type ConstantUtf8");
  readAttributes(attributes_, cp, is);
}

Field::~Field()
{
  for_each(attributes_.begin(), attributes_.end(), deleter<Attribute>);
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

ConstantValueAttribute* Field::getConstantValueAttribute() const
{
  if (!isStatic())
    return NULL;

  return (ConstantValueAttribute*) getAttribute(attributes_,
                                                Attribute::CONSTANT_VALUE);
}

//===----------------------------------------------------------------------===//
// Method implementation
Method::Method(ClassFile* parent, const ConstantPool& cp, std::istream& is)
  : parent_(parent)
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

Method::~Method()
{
  for_each(attributes_.begin(), attributes_.end(), deleter<Attribute>);
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

CodeAttribute* Method::getCodeAttribute() const
{
  return (CodeAttribute*) getAttribute(attributes_, Attribute::CODE);
}

ExceptionsAttribute* Method::getExceptionsAttribute() const
{
  return (ExceptionsAttribute*) getAttribute(attributes_,
                                             Attribute::EXCEPTIONS);
}

//===----------------------------------------------------------------------===//
// Attribute implementation
const std::string Attribute::CONSTANT_VALUE = "ConstantValue";
const std::string Attribute::CODE = "Code";
const std::string Attribute::EXCEPTIONS = "Exceptions";
const std::string Attribute::INNER_CLASSES = "InnerClasses";
const std::string Attribute::SYNTHETIC = "Synthetic";
const std::string Attribute::SOURCE_FILE = "SourceFile";
const std::string Attribute::LINE_NUMBER_TABLE = "LineNumberTable";
const std::string Attribute::LOCAL_VARIABLE_TABLE = "LocalVariableTable";
const std::string Attribute::DEPRECATED = "Deprecated";

Attribute* Attribute::readAttribute(const ConstantPool& cp, std::istream& is)
{
  ConstantUtf8* name = dynamic_cast<ConstantUtf8*>(cp[readU2(is)]);
  if (!name)
    throw ClassFileSemanticError(
      "Representation of attribute name is not of type ConstantUtf8");

  if (CONSTANT_VALUE == name->str())
    return new ConstantValueAttribute(name, cp, is);
  else if (CODE == name->str())
    return new CodeAttribute(name, cp, is);
  else {
    uint32_t length = readU4(is);
    is.ignore(length);
    return new Attribute(name, cp, is);
  }
}

Attribute::Attribute(ConstantUtf8* name,
                     const ConstantPool& cp,
                     std::istream& is)
  : name_(name)
{

}

Attribute::~Attribute()
{

}

std::ostream& Attribute::dump(std::ostream& os) const
{
  return os << *getName();
}

//===----------------------------------------------------------------------===//
// AttributeConstantValue implementation
ConstantValueAttribute::ConstantValueAttribute(ConstantUtf8* name,
                                               const ConstantPool& cp,
                                               std::istream& is)
  : Attribute(name, cp, is)
{
  uint32_t length = readU4(is);
  if (length != 2)
    throw ClassFileSemanticError(
      "Length of ConstantValueAttribute is not 2");
  value_ = cp[readU2(is)];
}

std::ostream& ConstantValueAttribute::dump(std::ostream& os) const
{
  return Attribute::dump(os) << ": " << *value_;
}

//===----------------------------------------------------------------------===//
// AttributeCode implementation
CodeAttribute::CodeAttribute(ConstantUtf8* name,
                             const ConstantPool& cp,
                             std::istream& is)
  : Attribute(name, cp, is)
{
  uint32_t length = readU4(is);
  maxStack_ = readU2(is);
  maxLocals_ = readU2(is);
  codeSize_ = readU4(is);
  code_ = new uint8_t[codeSize_];
  std::streamsize s = is.rdbuf()->sgetn(reinterpret_cast<char*>(code_), codeSize_);
  if (s != (std::streamsize) codeSize_)
    throw ClassFileParseError(
      "Could not read code from input stream");
  uint16_t exceptCount = readU2(is);
  exceptions_.reserve(exceptCount);
  while (exceptCount--)
    exceptions_.push_back(new Exception(cp, is));
  readAttributes(attributes_, cp, is);
}

CodeAttribute::~CodeAttribute()
{
  delete[] code_;
  for_each(exceptions_.begin(), exceptions_.end(), deleter<Exception>);
  for_each(attributes_.begin(), attributes_.end(), deleter<Attribute>);
}

std::ostream& CodeAttribute::dump(std::ostream& os) const
{
  Attribute::dump(os)
    << '\n'
    << "Max stack: " << maxStack_ << '\n'
    << "Max locals: " << maxLocals_ << '\n'
    << "Code size: " << codeSize_ << '\n';
  dumpCollection(exceptions_, "Exception", os);
  dumpCollection(attributes_, "Attribute", os);

  return os;
}

CodeAttribute::Exception::Exception(const ConstantPool& cp,
                                    std::istream& is)
  : catchType_(NULL)
{
  startPc_ = readU2(is);
  endPc_ = readU2(is);
  handlerPc_ = readU2(is);
  uint16_t idx = readU2(is);
  if (idx) {
    catchType_ = dynamic_cast<ConstantClass*>(cp[idx]);
    if (!catchType_)
      throw ClassFileSemanticError
        ("Representation of catch type is not of type ConstantClass");
  }
}

std::ostream& CodeAttribute::Exception::dump(std::ostream& os) const
{
  if (getCatchType())
    os << *getCatchType() << '\n';
  else
    os << "catch-all\n";
  return os << "Start PC: " << startPc_ << '\n'
            << "End PC: " << endPc_ << '\n'
            << "Handler PC: " << handlerPc_;
}

//===----------------------------------------------------------------------===//
// AttributeExceptions implementation
ExceptionsAttribute::ExceptionsAttribute(ConstantUtf8* name,
                                         const ConstantPool& cp,
                                         std::istream& is)
  : Attribute(name, cp, is)
{
  uint32_t length = readU4(is);
  readClasses(exceptions_, cp, is);
}

std::ostream& ExceptionsAttribute::dump(std::ostream& os) const
{
  os << Attribute::dump(os) << ": ";
  for (Classes::const_iterator
         i = exceptions_.begin(), e = exceptions_.end(); i != e; ++i)
    os << *i << ' ';
  return os;
}

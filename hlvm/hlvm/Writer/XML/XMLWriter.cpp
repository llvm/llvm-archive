//===-- hlvm/Writer/XML/XMLWriter.cpp - AST XML Writer Class ----*- C++ -*-===//
//
//                      High Level Virtual Machine (HLVM)
//
// Copyright (C) 2006 Reid Spencer. All Rights Reserved.
//
// This software is free software; you can redistribute it and/or modify it 
// under the terms of the GNU Lesser General Public License as published by 
// the Free Software Foundation; either version 2.1 of the License, or (at 
// your option) any later version.
//
// This software is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for 
// more details.
//
// You should have received a copy of the GNU Lesser General Public License 
// along with this library in the file named LICENSE.txt; if not, write to the 
// Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
// MA 02110-1301 USA
//
//===----------------------------------------------------------------------===//
/// @file hlvm/Writer/XML/XMLWriter.cpp
/// @author Reid Spencer <rspencer@x10sys.com>
/// @date 2006/05/12
/// @since 0.1.0
/// @brief Provides the interface to hlvm::XMLWriter
//===----------------------------------------------------------------------===//

#include <hlvm/Writer/XML/XMLWriter.h>
#include <hlvm/AST/AST.h>
#include <hlvm/AST/Bundle.h>
#include <hlvm/AST/Documentation.h>
#include <hlvm/AST/Function.h>
#include <hlvm/AST/Import.h>
#include <hlvm/AST/ContainerType.h>
#include <hlvm/AST/Variable.h>
#include <hlvm/Base/Assert.h>
#include <llvm/ADT/StringExtras.h>
#include <libxml/xmlwriter.h>
#include <iostream>

using namespace hlvm;
using namespace llvm;

namespace {

class XMLWriterImpl : public XMLWriter {
  xmlTextWriterPtr writer;
  AST* node;
public:
  XMLWriterImpl(const char* fname)
    : writer(0), node(0)
  { 
    writer = xmlNewTextWriterFilename(fname,0);
    hlvmAssert(writer && "Can't allocate writer");
    xmlTextWriterSetIndent(writer,1);
    xmlTextWriterSetIndentString(writer,reinterpret_cast<const xmlChar*>("  "));
  }

  virtual ~XMLWriterImpl() 
  { 
    xmlFreeTextWriter(writer);
  }

  virtual void write(AST* node);

private:
  inline void writeComment(const char* cmt)
    { xmlTextWriterWriteComment(writer,
        reinterpret_cast<const xmlChar*>(cmt)); }
  inline void startElement(const char* elem) 
    { xmlTextWriterStartElement(writer, 
        reinterpret_cast<const xmlChar*>(elem)); }
  inline void endElement() 
    { xmlTextWriterEndElement(writer); }
  inline void writeAttribute(const char*name, const char*val)
    { xmlTextWriterWriteAttribute(writer, 
        reinterpret_cast<const xmlChar*>(name), 
        reinterpret_cast<const xmlChar*>(val)); }
  inline void writeAttribute(const char* name, const std::string& val) 
    { writeAttribute(name, val.c_str()); }
  inline void writeAttribute(const char* name, Type* t)
    { writeAttribute(name, t->getName()); }
  inline void writeAttribute(const char* name, uint64_t val)
    { writeAttribute(name, llvm::utostr(val)); }
  inline void writeElement(const char* elem, const char* body)
    { xmlTextWriterWriteElement(writer,
        reinterpret_cast<const xmlChar*>(elem),
        reinterpret_cast<const xmlChar*>(body)); }

  inline void putHeader();
  inline void putFooter();
  inline void putDoc(Documentable* node);
  inline void put(Bundle* b);
  inline void put(Documentation* b);
  inline void put(Variable* v);
  inline void put(Function* f);
  inline void put(AliasType* t);
  inline void put(AnyType* t);
  inline void put(BooleanType* t);
  inline void put(CharacterType* t);
  inline void put(IntegerType* t);
  inline void put(RangeType* t);
  inline void put(EnumerationType* t);
  inline void put(RealType* t);
  inline void put(OctetType* t);
  inline void put(VoidType* t);
  inline void put(PointerType* t);
  inline void put(ArrayType* t);
  inline void put(VectorType* t);
  inline void put(StructureType* t);
  inline void put(SignatureType* t);
};

inline void
XMLWriterImpl::putDoc(Documentable* node)
{
  Documentation* theDoc = node->getDoc();
  if (theDoc) {
    this->put(theDoc);
  }
}

void
XMLWriterImpl::putHeader() 
{
  xmlTextWriterStartDocument(writer,0,"UTF-8",0);
  startElement("hlvm");
  writeAttribute("xmlns","http://hlvm.org/src/hlvm/Reader/XML/HLVM.rng");
}

void
XMLWriterImpl::putFooter()
{
  endElement();
  xmlTextWriterEndDocument(writer);
}

void
XMLWriterImpl::put(Function* f)
{
}

void 
XMLWriterImpl::put(Documentation* b)
{
  startElement("doc");
  const std::string& data = b->getDoc();
  xmlTextWriterWriteRawLen(writer,
    reinterpret_cast<const xmlChar*>(data.c_str()),data.length());
  endElement();
}

void 
XMLWriterImpl::put(AliasType* t)
{
  startElement("alias");
  writeAttribute("name",t->getName());
  writeAttribute("renames",t->getType());
  putDoc(t);
  endElement();
}
void 
XMLWriterImpl::put(AnyType* t)
{
  startElement("atom");
  writeAttribute("name",t->getName());
  putDoc(t);
  startElement("intrinsic");
  writeAttribute("is","any");
  endElement();
  endElement();
}

void
XMLWriterImpl::put(BooleanType* t)
{
  startElement("atom");
  writeAttribute("name",t->getName().c_str());
  putDoc(t);
  startElement("intrinsic");
  writeAttribute("is","bool");
  endElement();
  endElement();
}

void
XMLWriterImpl::put(CharacterType* t)
{
  startElement("atom");
  writeAttribute("name",t->getName().c_str());
  putDoc(t);
  startElement("intrinsic");
  writeAttribute("is","char");
  endElement();
  endElement();
}

void
XMLWriterImpl::put(IntegerType* t)
{
  startElement("atom");
  writeAttribute("name",t->getName().c_str());
  putDoc(t);
  const char* primName = t->getPrimitiveName();
  if (primName) {
    startElement("intrinsic");
    writeAttribute("is",primName);
  } else if (t->isSigned()) {
    startElement("signed");
    writeAttribute("bits", llvm::utostr(t->getBits()));
  } else {
    startElement("unsigned");
    writeAttribute("bits", llvm::utostr(t->getBits()));
  }
  endElement();
  endElement();
}

void
XMLWriterImpl::put(RangeType* t)
{
  startElement("range");
  writeAttribute("name",t->getName());
  writeAttribute("min",t->getMin());
  writeAttribute("max",t->getMax());
  putDoc(t);
  endElement();
}

void 
XMLWriterImpl::put(EnumerationType* t)
{
  startElement("enumeration");
  writeAttribute("name",t->getName());
  putDoc(t);
  for (EnumerationType::const_iterator I = t->begin(), E = t->end(); 
       I != E; ++I)
  {
    startElement("enumerator");
    writeAttribute("id",*I);
    endElement();
  }
  endElement();
}

void
XMLWriterImpl::put(RealType* t)
{
  startElement("atom");
  writeAttribute("name",t->getName().c_str());
  putDoc(t);
  const char* primName = t->getPrimitiveName();
  if (primName) {
    startElement("intrinsic");
    writeAttribute("is",primName);
    endElement();
  } else {
    startElement("real");
    writeAttribute("mantissa", llvm::utostr(t->getMantissa()));
    writeAttribute("exponent", llvm::utostr(t->getExponent()));
    endElement();
  }
  endElement();
}

void
XMLWriterImpl::put(OctetType* t)
{
  startElement("atom");
  writeAttribute("name",t->getName().c_str());
  putDoc(t);
  startElement("intrinsic");
  writeAttribute("is","octet");
  endElement();
  endElement();
}

void
XMLWriterImpl::put(VoidType* t)
{
  startElement("atom");
  writeAttribute("name",t->getName());
  putDoc(t);
  startElement("intrinsic");
  writeAttribute("is","void");
  endElement();
  endElement();
}

void 
XMLWriterImpl::put(PointerType* t)
{
  startElement("pointer");
  writeAttribute("name", t->getName());
  writeAttribute("to", t->getTargetType());
  putDoc(t);
  endElement();
}

void 
XMLWriterImpl::put(ArrayType* t)
{
  startElement("array");
  writeAttribute("name", t->getName());
  writeAttribute("of", t->getElementType());
  writeAttribute("length", t->getMaxSize());
  putDoc(t);
  endElement();
}

void 
XMLWriterImpl::put(VectorType* t)
{
  startElement("vector");
  writeAttribute("name", t->getName());
  writeAttribute("of", t->getElementType());
  writeAttribute("length", t->getSize());
  putDoc(t);
  endElement();
}

void 
XMLWriterImpl::put(StructureType* t)
{
  startElement("structure");
  writeAttribute("name",t->getName());
  putDoc(t);
  for (StructureType::iterator I = t->begin(), E = t->end(); I != E; ++I) {
    startElement("field");
    AliasType* alias = cast<AliasType>(*I);
    writeAttribute("name",alias->getName());
    writeAttribute("type",alias->getType());
    putDoc(alias);
    endElement();
  }
  endElement();
}

void 
XMLWriterImpl::put(SignatureType* t)
{
  startElement("signature");
  writeAttribute("name",t->getName());
  writeAttribute("result",t->getResultType());
  writeAttribute("varargs",t->isVarArgs() ? "true" : "false");
  putDoc(t);
  for (SignatureType::iterator I = t->begin(), E = t->end(); I != E; ++I) {
    startElement("arg");
    AliasType* alias = cast<AliasType>(*I);
    writeAttribute("name",alias->getName());
    writeAttribute("type",alias->getType());
    putDoc(alias);
    endElement();
  }
  endElement();
}

void
XMLWriterImpl::put(Variable* v)
{
  startElement("var");
  writeAttribute("name",v->getName().c_str());
  writeAttribute("type",v->getType()->getName().c_str());
  putDoc(v);
  endElement();
}

void 
XMLWriterImpl::put(Bundle* b)
{
  startElement("bundle");
  writeAttribute("pubid",b->getName().c_str());
  putDoc(b);
  for (Bundle::const_iterator I = b->begin(),E = b->end(); I != E; ++I)
  {
    switch ((*I)->getID()) 
    {
      case DocumentationID:    put(cast<Documentation>(*I)); break;
      case VariableID:         put(cast<Variable>(*I)); break;
      case FunctionID:         put(cast<Function>(*I)); break;
      case AliasTypeID:        put(cast<AliasType>(*I)); break;
      case AnyTypeID:          put(cast<AnyType>(*I)); break;
      case BooleanTypeID:      put(cast<BooleanType>(*I)); break;
      case CharacterTypeID:    put(cast<CharacterType>(*I)); break;
      case IntegerTypeID:      put(cast<IntegerType>(*I)); break;
      case RangeTypeID:        put(cast<RangeType>(*I)); break;
      case EnumerationTypeID:  put(cast<EnumerationType>(*I)); break;
      case RealTypeID:         put(cast<RealType>(*I)); break;
      case OctetTypeID:        put(cast<OctetType>(*I)); break;
      case VoidTypeID:         put(cast<VoidType>(*I)); break;
      case PointerTypeID:      put(cast<PointerType>(*I)); break;
      case ArrayTypeID:        put(cast<ArrayType>(*I)); break;
      case VectorTypeID:       put(cast<VectorType>(*I)); break;
      case StructureTypeID:    put(cast<StructureType>(*I)); break;
      case SignatureTypeID:    put(cast<SignatureType>(*I)); break;
      default:
        hlvmDeadCode("Invalid bundle content");
    }
  }
  endElement();
}

void
XMLWriterImpl::write(AST* ast) 
{
  node = ast;
  putHeader();
  put(ast->getRoot());
  putFooter();
}

}

XMLWriter* 
hlvm::XMLWriter::create(const char* fname)
{
  return new XMLWriterImpl(fname);
}

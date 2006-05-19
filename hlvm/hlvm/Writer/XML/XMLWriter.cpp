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
#include <hlvm/AST/Function.h>
#include <hlvm/AST/Import.h>
#include <hlvm/AST/ContainerType.h>
#include <hlvm/AST/Variable.h>
#include <llvm/ADT/StringExtras.h>
#include <libxml/xmlwriter.h>
#include <iostream>
#include <cassert>

using namespace hlvm;
using namespace llvm;

namespace {

class XMLWriterImpl : public XMLWriter {
  xmlTextWriterPtr writer;
  AST::AST* node;
public:
  XMLWriterImpl(const char* fname)
    : writer(0), node(0)
  { 
    writer = xmlNewTextWriterFilename(fname,0);
    assert(writer && "Can't allocate writer");
    xmlTextWriterSetIndent(writer,1);
    xmlTextWriterSetIndentString(writer,reinterpret_cast<const xmlChar*>("  "));
  }

  virtual ~XMLWriterImpl() 
  { 
    xmlFreeTextWriter(writer);
  }

  virtual void write(AST::AST* node);

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
  inline void writeAttribute(const char* name, AST::Type* t)
    { writeAttribute(name, t->getName()); }
  inline void writeAttribute(const char* name, uint64_t val)
    { writeAttribute(name, llvm::utostr(val)); }
  inline void writeElement(const char* elem, const char* body)
    { xmlTextWriterWriteElement(writer,
        reinterpret_cast<const xmlChar*>(elem),
        reinterpret_cast<const xmlChar*>(body)); }

  inline void putHeader();
  inline void putFooter();
  inline void put(AST::Bundle* b);
  inline void put(AST::Variable* v);
  inline void put(AST::Function* f);
  inline void put(AST::AliasType* t);
  inline void put(AST::AnyType* t);
  inline void put(AST::BooleanType* t);
  inline void put(AST::CharacterType* t);
  inline void put(AST::IntegerType* t);
  inline void put(AST::RangeType* t);
  inline void put(AST::EnumerationType* t);
  inline void put(AST::RealType* t);
  inline void put(AST::OctetType* t);
  inline void put(AST::VoidType* t);
  inline void put(AST::PointerType* t);
  inline void put(AST::ArrayType* t);
  inline void put(AST::VectorType* t);
  inline void put(AST::StructureType* t);
  inline void put(AST::SignatureType* t);
};


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
XMLWriterImpl::put(AST::Function* f)
{
}

void 
XMLWriterImpl::put(AST::AliasType* t)
{
  startElement("alias");
  writeAttribute("name",t->getName());
  writeAttribute("renames",t->getType());
  endElement();
}
void 
XMLWriterImpl::put(AST::AnyType* t)
{
  startElement("atom");
  writeAttribute("name",t->getName());
  startElement("intrinsic");
  writeAttribute("is","any");
  endElement();
  endElement();
}

void
XMLWriterImpl::put(AST::BooleanType* t)
{
  startElement("atom");
  writeAttribute("name",t->getName().c_str());
  startElement("intrinsic");
  writeAttribute("is","bool");
  endElement();
  endElement();
}

void
XMLWriterImpl::put(AST::CharacterType* t)
{
  startElement("atom");
  writeAttribute("name",t->getName().c_str());
  startElement("intrinsic");
  writeAttribute("is","char");
  endElement();
  endElement();
}

void
XMLWriterImpl::put(AST::IntegerType* t)
{
  startElement("atom");
  writeAttribute("name",t->getName().c_str());
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
XMLWriterImpl::put(AST::RangeType* t)
{
  startElement("range");
  writeAttribute("name",t->getName());
  writeAttribute("min",t->getMin());
  writeAttribute("max",t->getMax());
  endElement();
}

void 
XMLWriterImpl::put(AST::EnumerationType* t)
{
  startElement("enumeration");
  writeAttribute("name",t->getName());
  for (AST::EnumerationType::const_iterator I = t->begin(), E = t->end(); 
       I != E; ++I)
  {
    startElement("enumerator");
    writeAttribute("id",*I);
    endElement();
  }
  endElement();
}

void
XMLWriterImpl::put(AST::RealType* t)
{
  startElement("atom");
  writeAttribute("name",t->getName().c_str());
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
XMLWriterImpl::put(AST::OctetType* t)
{
  startElement("atom");
  writeAttribute("name",t->getName().c_str());
  startElement("intrinsic");
  writeAttribute("is","octet");
  endElement();
  endElement();
}

void
XMLWriterImpl::put(AST::VoidType* t)
{
  startElement("atom");
  writeAttribute("name",t->getName());
  startElement("intrinsic");
  writeAttribute("is","void");
  endElement();
  endElement();
}

void 
XMLWriterImpl::put(AST::PointerType* t)
{
  startElement("pointer");
  writeAttribute("name", t->getName());
  writeAttribute("to", t->getTargetType());
  endElement();
}

void 
XMLWriterImpl::put(AST::ArrayType* t)
{
  startElement("array");
  writeAttribute("name", t->getName());
  writeAttribute("of", t->getElementType());
  writeAttribute("length", t->getMaxSize());
  endElement();
}

void 
XMLWriterImpl::put(AST::VectorType* t)
{
  startElement("vector");
  writeAttribute("name", t->getName());
  writeAttribute("of", t->getElementType());
  writeAttribute("length", t->getSize());
  endElement();
}

void 
XMLWriterImpl::put(AST::StructureType* t)
{
  startElement("structure");
  writeAttribute("name",t->getName());
  for (AST::StructureType::iterator I = t->begin(), E = t->end(); I != E; ++I) {
    startElement("field");
    AST::AliasType* nt = cast<AST::AliasType>(*I);
    writeAttribute("name",nt->getName());
    writeAttribute("type",nt->getType());
    endElement();
  }
  endElement();
}

void 
XMLWriterImpl::put(AST::SignatureType* t)
{
  startElement("signature");
  writeAttribute("name",t->getName());
  writeAttribute("result",t->getResultType());
  writeAttribute("varargs",t->isVarArgs() ? "true" : "false");
  for (AST::SignatureType::iterator I = t->begin(), E = t->end(); I != E; ++I) {
    startElement("arg");
    AST::AliasType* nt = cast<AST::AliasType>(*I);
    writeAttribute("name",nt->getName());
    writeAttribute("type",nt->getType());
    endElement();
  }
  endElement();
}

void
XMLWriterImpl::put(AST::Variable* v)
{
  startElement("var");
  writeAttribute("name",v->getName().c_str());
  writeAttribute("type",v->getType()->getName().c_str());
  endElement();
}

void 
XMLWriterImpl::put(AST::Bundle* b)
{
  startElement("bundle");
  writeAttribute("pubid",b->getName().c_str());
  for (AST::Bundle::const_iterator I = b->begin(),E = b->end(); I != E; ++I)
  {
    switch ((*I)->getID()) 
    {
      case AST::VariableID:         put(cast<AST::Variable>(*I)); break;
      case AST::FunctionID:         put(cast<AST::Function>(*I)); break;
      case AST::AliasTypeID:        put(cast<AST::AliasType>(*I)); break;
      case AST::AnyTypeID:          put(cast<AST::AnyType>(*I)); break;
      case AST::BooleanTypeID:      put(cast<AST::BooleanType>(*I)); break;
      case AST::CharacterTypeID:    put(cast<AST::CharacterType>(*I)); break;
      case AST::IntegerTypeID:      put(cast<AST::IntegerType>(*I)); break;
      case AST::RangeTypeID:        put(cast<AST::RangeType>(*I)); break;
      case AST::EnumerationTypeID:  put(cast<AST::EnumerationType>(*I)); break;
      case AST::RealTypeID:         put(cast<AST::RealType>(*I)); break;
      case AST::OctetTypeID:        put(cast<AST::OctetType>(*I)); break;
      case AST::VoidTypeID:         put(cast<AST::VoidType>(*I)); break;
      case AST::PointerTypeID:      put(cast<AST::PointerType>(*I)); break;
      case AST::ArrayTypeID:        put(cast<AST::ArrayType>(*I)); break;
      case AST::VectorTypeID:       put(cast<AST::VectorType>(*I)); break;
      case AST::StructureTypeID:    put(cast<AST::StructureType>(*I)); break;
      case AST::SignatureTypeID:    put(cast<AST::SignatureType>(*I)); break;
      default:
        assert(!"Invalid bundle content");
    }
  }
  endElement();
}

void
XMLWriterImpl::write(AST::AST* ast) 
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

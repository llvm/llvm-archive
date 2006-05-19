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
  inline void writeElement(const char* elem, const char* body)
    { xmlTextWriterWriteElement(writer,
        reinterpret_cast<const xmlChar*>(elem),
        reinterpret_cast<const xmlChar*>(body)); }

  inline void putHeader();
  inline void putFooter();
  inline void put(AST::Node* node);
  inline void put(AST::Bundle* b);
  inline void put(AST::Variable* v);
  inline void put(AST::Function* f);
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

inline void
XMLWriterImpl::put(AST::Function* f)
{
}

inline void
XMLWriterImpl::put(AST::Variable* v)
{
  startElement("var");
  writeAttribute("name",v->getName().c_str());
  writeAttribute("type",v->getType()->getName().c_str());
  endElement();
}

inline void 
XMLWriterImpl::put(AST::Bundle* b)
{
  startElement("bundle");
  writeAttribute("pubid",b->getName().c_str());
  for (AST::ParentNode::const_iterator I = b->begin(),E = b->end(); I != E; ++I)
  {
    switch ((*I)->getID()) 
    {
      case AST::VariableID: put(cast<AST::Variable>(*I)); break;
      case AST::FunctionID: put(cast<AST::Function>(*I)); break;
      default:
        assert(!"Invalid bundle content");
    }
  }
  endElement();
}

void
XMLWriterImpl::put(AST::Node* node) 
{
  switch (node->getID()) 
  {
    case hlvm::AST::VoidTypeID:		break;     
    case hlvm::AST::AnyTypeID:		break;          
    case hlvm::AST::BooleanTypeID:	break;      
    case hlvm::AST::CharacterTypeID:	break;    
    case hlvm::AST::OctetTypeID:	break;        
    case hlvm::AST::IntegerTypeID:	break;      
    case hlvm::AST::RangeTypeID:	break;        
    case hlvm::AST::RealTypeID:		break;         
    case hlvm::AST::RationalTypeID:	break;     
    case hlvm::AST::StringTypeID:	break;       
    case hlvm::AST::PointerTypeID:	break;      
    case hlvm::AST::ArrayTypeID:	break;        
    case hlvm::AST::VectorTypeID:	break;       
    case hlvm::AST::StructureTypeID:	break;    
    case hlvm::AST::FieldID:		break;            
    case hlvm::AST::SignatureTypeID:	break;    
    case hlvm::AST::ArgumentID:		break;         
    case hlvm::AST::ContinuationTypeID:	break; 
    case hlvm::AST::InterfaceID:	break;        
    case hlvm::AST::ClassID:		break;            
    case hlvm::AST::MethodID:		break;           
    case hlvm::AST::ImplementsID:	break;       
    case hlvm::AST::VariableID:		break;         
    case hlvm::AST::FunctionID:		break;         
    case hlvm::AST::ProgramID:		break;          
    case hlvm::AST::BundleID:		
      put(llvm::cast<hlvm::AST::Bundle>(node)); 
      break;
    case hlvm::AST::BlockID:		break;            
    case hlvm::AST::CallOpID:		break;           
    case hlvm::AST::InvokeOpID:		break;         
    case hlvm::AST::DispatchOpID:	break;
    case hlvm::AST::CreateContOpID:	break;     
    case hlvm::AST::CallWithContOpID:	break;   
    case hlvm::AST::ReturnOpID:		break;         
    case hlvm::AST::ThrowOpID:		break;          
    case hlvm::AST::JumpToOpID:		break;         
    case hlvm::AST::BreakOpID:		break;          
    case hlvm::AST::IfOpID:		break;             
    case hlvm::AST::LoopOpID:		break;           
    case hlvm::AST::SelectOpID:		break;         
    case hlvm::AST::WithOpID:		break;           
    case hlvm::AST::LoadOpID:		break;           
    case hlvm::AST::StoreOpID:		break;          
    case hlvm::AST::AllocateOpID:	break;       
    case hlvm::AST::FreeOpID:		break;           
    case hlvm::AST::ReallocateOpID:	break;     
    case hlvm::AST::StackAllocOpID:	break;     
    case hlvm::AST::ReferenceOpID:	break;      
    case hlvm::AST::DereferenceOpID:	break;    
    case hlvm::AST::NegateOpID:		break;         
    case hlvm::AST::ComplementOpID:	break;     
    case hlvm::AST::PreIncrOpID:	break;        
    case hlvm::AST::PostIncrOpID:	break;       
    case hlvm::AST::PreDecrOpID:	break;        
    case hlvm::AST::PostDecrOpID:	break;       
    case hlvm::AST::AddOpID:		break;            
    case hlvm::AST::SubtractOpID:	break;       
    case hlvm::AST::MultiplyOpID:	break;       
    case hlvm::AST::DivideOpID:		break;         
    case hlvm::AST::ModulusOpID:	break;        
    case hlvm::AST::BAndOpID:		break;           
    case hlvm::AST::BOrOpID:		break;            
    case hlvm::AST::BXOrOpID:		break;           
    case hlvm::AST::AndOpID:		break;            
    case hlvm::AST::OrOpID:		break;             
    case hlvm::AST::NorOpID:		break;            
    case hlvm::AST::XorOpID:		break;            
    case hlvm::AST::NotOpID:		break;            
    case hlvm::AST::LTOpID:		break;             
    case hlvm::AST::GTOpID:		break;             
    case hlvm::AST::LEOpID:		break;             
    case hlvm::AST::GEOpID:		break;             
    case hlvm::AST::EQOpID:		break;             
    case hlvm::AST::NEOpID:		break;             
    case hlvm::AST::IsPInfOpID:		break;         
    case hlvm::AST::IsNInfOpID:		break;         
    case hlvm::AST::IsNaNOpID:		break;          
    case hlvm::AST::TruncOpID:		break;          
    case hlvm::AST::RoundOpID:		break;          
    case hlvm::AST::FloorOpID:		break;          
    case hlvm::AST::CeilingOpID:	break;        
    case hlvm::AST::PowerOpID:		break;          
    case hlvm::AST::LogEOpID:		break;           
    case hlvm::AST::Log2OpID:		break;           
    case hlvm::AST::Log10OpID:		break;          
    case hlvm::AST::SqRootOpID:		break;         
    case hlvm::AST::RootOpID:		break;           
    case hlvm::AST::FactorialOpID:	break;      
    case hlvm::AST::GCDOpID:		break;            
    case hlvm::AST::LCMOpID:		break;            
    case hlvm::AST::MungeOpID:		break;          
    case hlvm::AST::LengthOpID:		break;         
    case hlvm::AST::IntOpID:		break;            
    case hlvm::AST::RealOpID:		break;           
    case hlvm::AST::PInfOpID:		break;           
    case hlvm::AST::NInfOpID:		break;           
    case hlvm::AST::NaNOpID:		break;            
    case hlvm::AST::StringOpID:		break;         
    case hlvm::AST::ArrayOpID:		break;          
    case hlvm::AST::VectorOpID:		break;         
    case hlvm::AST::StructureOpID:	break;      
    case hlvm::AST::MapFileOpID:	break;        
    case hlvm::AST::OpenOpID:		break;           
    case hlvm::AST::CloseOpID:		break;          
    case hlvm::AST::ReadOpID:		break;           
    case hlvm::AST::WriteOpID:		break;          
    case hlvm::AST::PositionOpID:	break;       
    default:
      assert(!"Invalid Node ID");
      break;
  }
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

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
#include <iostream>
#include <cassert>

using namespace hlvm;

namespace {

// An ostream that automatically indents and provides methods to control it.
class ostream_indent
{
  uint32_t indent_;
public:
  std::ostream& strm_;
  ostream_indent(std::ostream& os) : indent_(0), strm_(os) {}

  inline void nl()
  {
    strm_ << '\n';
    strm_.width(indent_);
    strm_ << ' ';
  }

  inline void in( bool with_newline = false)
  {
    indent_++;
    if (with_newline)
      nl();
  }

  inline void out( bool with_newline = false)
  {
    indent_--;
    if (with_newline)
      nl();
  }
};

class XMLWriterImpl : public XMLWriter {
  ostream_indent ind_;
  std::ostream& out_;
  AST::AST* node_;
public:
  XMLWriterImpl(std::ostream& out)
    : ind_(out), out_(out), node_(0)
  { }

  virtual ~XMLWriterImpl() 
  { 
    out_.flush();
  }

  virtual void write(AST::AST* node);

private:
  inline void putHeader();
  inline void putFooter();
  inline void put(AST::Node* node);
  inline void put(AST::Bundle* node);
};

std::string 
sanitize(const std::string* input)
{
  // Replace all the & in the name with &amp;  and simliarly for < and > 
  // because XML doesn't like 'em
  std::string output(*input);
  std::string::size_type pos = 0;
  while (std::string::npos != (pos = output.find('&',pos)))
  {
    output.replace(pos,1,"&amp;");
    pos += 5;
  }
  pos = 0;
  while (std::string::npos != (pos = output.find('<',pos)))
  {
    output.replace(pos,1,"&lt;");
    pos += 4;
  }
  pos = 0;
  while (std::string::npos != (pos = output.find('>',pos)))
  {
    output.replace(pos,1,"&gt;");
    pos += 4;
  }
  return output;
}

std::string 
sanitize(const std::string& input)
{
  return sanitize(&input);
}

void
XMLWriterImpl::putHeader() 
{
  out_ << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  out_ << "<hlvm xmlns=\"http://hlvm.org/src/hlvm/Reader/XML/HLVM.rng\">";
  ind_.in(true);
}

void
XMLWriterImpl::putFooter()
{
  ind_.out(true);
  out_ << "</hlvm>\n";
}

inline void 
XMLWriterImpl::put(AST::Bundle* b)
{
  out_ << "<bundle pubid=\"" << b->getName() << "\">";
  ind_.in(true);
  ind_.out(true);
  out_ << "</bundle>";
  ind_.out(false);
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
  node_ = ast;
  putHeader();
  put(ast->getRoot());
  putFooter();
}

}

XMLWriter* 
hlvm::XMLWriter::create(std::ostream& out)
{
  return new XMLWriterImpl(out);
}

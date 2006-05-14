//===-- hlvm/Reader/XML/XMLReader.cpp - AST XML Reader Class ----*- C++ -*-===//
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
/// @file hlvm/Reader/XML/XMLReader.cpp
/// @author Reid Spencer <rspencer@x10sys.com>
/// @date 2006/05/12
/// @since 0.1.0
/// @brief Provides the interface to hlvm::XMLReader
//===----------------------------------------------------------------------===//

#include <hlvm/Reader/XML/XMLReader.h>
#include <hlvm/Reader/XML/HLVMTokenizer.h>
#include <hlvm/Base/Locator.h>
#include <hlvm/AST/AST.h>
#include <expat.h>
#include <vector>
#include <string>

using namespace hlvm;

namespace {

/// This structure provides information about an attribute and its value.
/// It is used during parsing of an XML document when the parser calls
/// the Handler's ElementStart method.
/// @brief Attribute Information Structure.
enum AttributeTypes
{
  CDATA_AttrType,
  ID_AttrType,
  IDREF_AttrType,
  IDREFS_AttrType,
  NMTOKEN_AttrType,
  NMTOKENS_AttrType,
  ENTITY_AttrType,
  ENTITIES_AttrType,
  NOTATION_AttrType,
};

enum SpecialTokens 
{
  NamespaceToken = -1,
  CharactersToken = -2,
  CommentToken = -3,
  CDATASectionToken = -4,
  ProcessingInstructionToken = -5,

};

struct AttrInfo
{
  std::string uri;    ///< The namespace URI of the attribute
  std::string local;  ///< The name of the attribute
  std::string value;  ///< The value of the attribute
  AttributeTypes type;///< The basic type of the attribute
  int32_t token;      ///< The token for the attribute name
  uint32_t ns;        ///< The token for the attribute namespace
};

struct NodeInfo : public hlvm::Locator
{
  std::string uri;    ///< The namespace uri of the element
  std::string local;  ///< The local name of the element
  int32_t token;      ///< Tokenized value of local name
  uint32_t ns;        ///< Tokenized value of namespace name
};

/// This structure provides information about an element. It is used during
/// parsing of an XML document when the parser calls the Handler's 
/// ElementStart method. 
/// @brief Element Information Structure.
struct ElementInfo : public NodeInfo {
  std::vector<NodeInfo> kids;   ///< Node info of child elements
  std::vector<AttrInfo> attrs; ///< Attributes of the element
  void find_attrs(
    int token1, const std::string*& value1) const;
  void find_attrs(
    int token1, const std::string*& value1,
    int token2, const std::string*& value2
  ) const;
  void find_attrs(
    int token1, const std::string*& value1,
    int token2, const std::string*& value2,
    int token3, const std::string*& value3
  ) const;
};

class XMLReaderImpl : public XMLReader {
  llvm::sys::Path path_;
  AST::AST* ast_;
  XML_Parser xp_;
  std::vector<ElementInfo> elems_; ///< The element stack
  ElementInfo* etop_; ///< A pointer to the top of the element stack
public:
  XMLReaderImpl(const llvm::sys::Path& path) :
    path_(path), ast_(0), xp_(0), elems_(), etop_(0)
  {
    xp_ = XML_ParserCreate( "UTF-8");
    // Reserve some space on the elements and attributes list so we aren't
    // mucking around with tiny allocations. If we cross 64 elements on the
    // stack or 64 attributes on one element, then they will double to 128. 
    // Its unlikely that documents will reach these limits and so there 
    // will be no reallocation after this initial reserve.
    elems_.reserve(64);
  }

  virtual ~XMLReaderImpl() 
  { 
    if (ast_) delete ast_; 
    XML_ParserFree( xp_ );
  }

  virtual void read();
  virtual AST::AST* get();

/// @name Expat Parsing Handlers
/// @{
private:

  static void XMLCALL 
  StartElementHandler(
    void *user_data, const XML_Char* name, const XML_Char** attributes
  )
  {
    // Convert the user data to our XMLReaderImpl pointer
    register XMLReaderImpl* p = reinterpret_cast<XMLReaderImpl*>(user_data);

    // Make a new element info on the top of the stack.
    p->elems_.resize(p->elems_.size()+1);
    p->etop_ = &p->elems_.back();
    ElementInfo& ei = *(p->etop_);

    // Fill in the element info
    ei.local = name;
    ei.token = HLVM_Reader_XML::HLVMTokenizer::recognize(name);
    ei.set(
      "",
      p->path_.c_str(),
      uint32_t(XML_GetCurrentLineNumber(p->xp_)), 
      uint32_t(XML_GetCurrentColumnNumber(p->xp_))
    );
    ei.kids.clear();
    ei.attrs.clear();

    // Handle the attributes
    if ( attributes )
    {
      // Determine index of first default attribute
      // size_t default_attr_index = XML_GetSpecifiedAttributeCount( p->xp_ );

      // Process all the attributes
      size_t curr_attr = 0;
      while ( *attributes != 0 )
      {
        // Resize the attrs vector to accommodate this attribute and get
        // a preference to that current attribute for ease of expression
        ei.attrs.resize(curr_attr+1);
        AttrInfo& attr = ei.attrs[curr_attr];

        // Get the token for the
        attr.local = *attributes;
        attr.token = HLVM_Reader_XML::HLVMTokenizer::recognize(*attributes);
        attr.value = attributes[1];

        // Increment loop counters
        attributes +=2;
        curr_attr++;
      }
    }

    // Tell the handler about the element
    //p->handler_->ElementStart(ei);
  }

  static void XMLCALL 
  EndElementHandler( void *user_data, const XML_Char *name)
  {
    // Get the parser
    register XMLReaderImpl* p = reinterpret_cast<XMLReaderImpl*>(user_data);

    // Get the current position
    int line = XML_GetCurrentLineNumber( p->xp_ );
    int column = XML_GetCurrentColumnNumber( p->xp_ );

    // Convert the element name to a token
    int name_token = HLVM_Reader_XML::HLVMTokenizer::recognize(name);

    // Save the previous token before poping it and make sure that it is the
    // same as the one the parser told us we're popping.
    int32_t token = p->elems_.back().token;
    assert(token == name_token);

    // Tell the handler that we're ending an element.
    // p->handler_->ElementEnd( p->elems_.back(), line, column );

    // Pop the element token and then push it on the "kids" list of the 
    // parent element indicating that we've completed parsing one child element.
    NodeInfo ki = static_cast<NodeInfo&>(p->elems_.back());
    p->elems_.pop_back();
    if (!p->elems_.empty())
    {
      p->etop_ = & p->elems_.back();
      p->etop_->kids.push_back(ki);
    }
  }

  static void XMLCALL 
  CharacterDataHandler( void *user_data, const XML_Char *s, int len)
  {
    // Get the parser
    register XMLReaderImpl* p = reinterpret_cast<XMLReaderImpl*>(user_data);

    // Tell the handler about the characters
    std::string tmp;
    tmp.assign(s,len);
    // p->handler_->Characters(tmp);
  }

  static void XMLCALL 
  ProcessingInstructionHandler(
    void *user_data, const XML_Char *target, const XML_Char *data)
  {
    // Get the parser
    register XMLReaderImpl* p = reinterpret_cast<XMLReaderImpl*>(user_data);

    // Tell the handler about the processing instruction
    // p->handler_->ProcessingInstruction(target,data);
  }

  static void XMLCALL 
  CommentHandler( void *user_data, const XML_Char *data)
  {
    // Get the parser
    register XMLReaderImpl* p = reinterpret_cast<XMLReaderImpl*>(user_data);

    // Comments are always valid
    // p->handler_->Comment(data);
  }

  static void XMLCALL 
  StartCdataSectionHandler(void *user_data)
  {
    // Get the parser
    register XMLReaderImpl* p = reinterpret_cast<XMLReaderImpl*>(user_data);

    // Put the CData Section on the element stack
    ElementInfo ei;
    ei.ns = 0;
    ei.local = "CDATA";
    ei.token = CDATASectionToken;
    ei.setLine( XML_GetCurrentLineNumber( p->xp_ ));
    ei.setColumn( XML_GetCurrentColumnNumber( p->xp_ ));
    ei.kids.clear();
    p->elems_.push_back(ei);

    // Inform the handler of the CData Section
    // p->handler_->CDataSectionStart();
  }

  static void XMLCALL 
  EndCdataSectionHandler(void *user_data)
  {
    // Get the parser
    register XMLReaderImpl* p = reinterpret_cast<XMLReaderImpl*>(user_data);

    // validate that the top of stack is a CDataSection
    assert(p->etop_->token == CDATASectionToken);

    // Pop the CData off the stack
    NodeInfo ki = static_cast<NodeInfo&>(p->elems_.back());
    p->elems_.pop_back();
    p->etop_ = & p->elems_.back();
    p->etop_->kids.push_back(ki);

    // Inform the handler (always valid)
    // p->handler_->CDataSectionEnd();
  }

  static void XMLCALL 
  DefaultHandler(
    void *user_data, const XML_Char *s, int len)
  {
    // static_cast<XMLReaderImpl*>(user_data)->handler_->Other(s,len);
  }

  static void XMLCALL 
  StartDoctypeDeclHandler(
    void * /*user_data*/, 
    const XML_Char * /*doctypeName*/, 
    const XML_Char * /*sysid*/, 
    const XML_Char * /*pubid*/, 
    int /*has_internal_subset*/)
  {
    // FIXME: Implement
  }

  static void XMLCALL 
  EndDoctypeDeclHandler(void * /*user_data*/)
  {
    // FIXME: Implement
  }

  static void XMLCALL 
  EntityDeclHandler( 
    void * /*user_data*/, 
    const XML_Char * /*entityName*/, 
    int /*is_parameter_entity*/, 
    const XML_Char * /*value*/,
    int /*value_length*/, 
    const XML_Char * /*base*/, 
    const XML_Char * /*systemId*/, 
    const XML_Char * /*publicId*/, 
    const XML_Char * /*notationName*/)
  {
    // FIXME: Implement
  }

  static void XMLCALL 
  NotationDeclHandler( 
    void * /*user_data*/,
    const XML_Char * /*notationName*/, 
    const XML_Char * /*base*/, 
    const XML_Char * /*systemId*/, 
    const XML_Char * /*publicId*/)
  {
    // FIXME: Implement
  }

  static int XMLCALL 
  NotStandaloneHandler(void * /*user_data*/ )
  {
    // FIXME: Implement
    return XML_STATUS_ERROR;
  }

  static int XMLCALL 
  ExternalEntityRefHandler( 
    XML_Parser /*parser*/,
    const XML_Char * /*context*/, 
    const XML_Char * /*base*/, 
    const XML_Char * /*systemId*/,
    const XML_Char * /*publicId*/)
  {
    // FIXME: Implement
    return XML_STATUS_ERROR;
  }

  static void XMLCALL 
  SkippedEntityHandler( 
    void * /*user_data*/, 
    const XML_Char * /*entityName*/, 
    int /*is_parameter_entity*/)
  {
    // FIXME: Implement
  }

  static int XMLCALL 
  UnknownEncodingHandler( 
    void * /*encodingHandlerData*/,
    const XML_Char * /*name*/, 
    XML_Encoding * /*info*/)
  {
    // FIXME: Implement
    return XML_STATUS_ERROR;
  }

/// @}
};

AST::AST*
XMLReaderImpl::get()
{
  return ast_;
}

static const XML_Char Namespace_Separator = 4; 

void
XMLReaderImpl::read() {
  ast_ = new AST::AST();

  // Set up the parser for parsing a document.
  XML_ParserReset(xp_,"UTF-8");
  XML_SetUserData(xp_, this );
  XML_SetElementHandler(xp_, &StartElementHandler, &EndElementHandler );
  XML_SetCharacterDataHandler( xp_, CharacterDataHandler );
  XML_SetProcessingInstructionHandler(xp_, ProcessingInstructionHandler );
  XML_SetCommentHandler( xp_, CommentHandler );
  XML_SetCdataSectionHandler( xp_, StartCdataSectionHandler, 
    EndCdataSectionHandler );
  XML_SetNotStandaloneHandler( xp_, NotStandaloneHandler );
  XML_SetExternalEntityRefHandler( xp_, ExternalEntityRefHandler);
  XML_SetSkippedEntityHandler( xp_, SkippedEntityHandler);
  XML_SetUnknownEncodingHandler( xp_, UnknownEncodingHandler, this);
}

}

XMLReader* 
XMLReader::create(const llvm::sys::Path& path)
{
  return new XMLReaderImpl(path);
}

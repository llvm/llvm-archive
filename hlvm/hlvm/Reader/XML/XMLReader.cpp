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
#include <hlvm/Base/Source.h>
#include <hlvm/AST/AST.h>
#include <hlvm/AST/Bundle.h>
#include <hlvm/AST/ContainerType.h>
#include <hlvm/AST/Function.h>
#include <hlvm/AST/Import.h>
#include <hlvm/AST/Variable.h>
#include <libxml/parser.h>
#include <libxml/relaxng.h>
#include <vector>
#include <string>
#include <iostream>

using namespace hlvm;
using namespace HLVM_Reader_XML;

namespace {

const char HLVMGrammar[] = 
#include "HLVM.rng.inc"
;

class XMLReaderImpl : public XMLReader {
  std::string path;
  AST::AST* ast;
  xmlDocPtr doc;
public:
  XMLReaderImpl(const std::string& p)
    : path(p), ast(0)
  {
    ast = new AST::AST();
    ast->setSystemID(p);
  }

  virtual ~XMLReaderImpl() 
  { 
    if (ast) delete ast; 
    if (doc) xmlFreeDoc(doc);
  }

  virtual void read();
  virtual AST::AST* get();

  void error(const std::string& msg) {
    std::cerr << msg << "\n";
  }

  std::string lookupToken(int32_t token) const
  {
    return HLVMTokenizer::lookup(token);
  }

  inline void handleParseError(xmlErrorPtr error);
  inline void handleValidationError(xmlErrorPtr error);

  void parseTree();
  AST::Bundle*   parseBundle(xmlNodePtr& cur);
  AST::Function* parseFunction(xmlNodePtr& cur);
  AST::Import*   parseImport(xmlNodePtr& cur);
  AST::Type*     parseType(xmlNodePtr& cur);
  AST::Variable* parseVariable(xmlNodePtr& cur);
private:
};

inline void
XMLReaderImpl::handleValidationError(xmlErrorPtr e)
{
  std::cerr << e->file << ":" << e->line << ": validation " <<
    (e->level == XML_ERR_WARNING ? "warning" : 
     (e->level == XML_ERR_ERROR ? "error" :
      (e->level == XML_ERR_FATAL ? "fatal error" : "message"))) 
    << ": " << e->message << "\n";
}

inline void
XMLReaderImpl::handleParseError(xmlErrorPtr e)
{
  std::cerr << e->file << ":" << e->line << ": parse " <<
    (e->level == XML_ERR_WARNING ? "warning" : 
     (e->level == XML_ERR_ERROR ? "error" :
      (e->level == XML_ERR_FATAL ? "fatal error" : "message"))) 
    << ": " << e->message << "\n";
}

void 
ParseHandler(void* userData, xmlErrorPtr error) 
{
  XMLReaderImpl* reader = reinterpret_cast<XMLReaderImpl*>(userData);
  reader->handleParseError(error);
}

void 
ValidationHandler(void* userData, xmlErrorPtr error)
{
  XMLReaderImpl* reader = reinterpret_cast<XMLReaderImpl*>(userData);
  reader->handleValidationError(error);
}

bool 
skipBlanks(xmlNodePtr &cur)
{
  while (cur && 
      (cur->type == XML_TEXT_NODE ||
       cur->type == XML_COMMENT_NODE ||
       cur->type == XML_PI_NODE))
  {
    cur = cur -> next;
  }
  return cur != 0;
}

inline const char* 
getAttribute(xmlNodePtr cur,const char*name)
{
  return reinterpret_cast<const char*>(
    xmlGetNoNsProp(cur,reinterpret_cast<const xmlChar*>(name)));
}

inline int 
getToken(const xmlChar* name)
{
  return HLVMTokenizer::recognize(reinterpret_cast<const char*>(name));
}

inline void 
getNameType(xmlNodePtr& cur, std::string& name, std::string& type)
{
  name = getAttribute(cur,"name");
  type = getAttribute(cur,"type");
}

AST::Function*
XMLReaderImpl::parseFunction(xmlNodePtr& cur)
{
  assert(getToken(cur->name)==TKN_import);
  AST::Locator loc(cur->line,0,&ast->getSystemID());
  std::string name, type;
  getNameType(cur, name, type);
  AST::Function* func = ast->new_Function(loc,name);
  return func;
}

AST::Import*
XMLReaderImpl::parseImport(xmlNodePtr& cur)
{
  assert(getToken(cur->name)==TKN_import);
  AST::Locator loc(cur->line,0,&ast->getSystemID());
  std::string pfx = getAttribute(cur,"prefix");
  AST::Import* imp = ast->new_Import(loc,pfx);
  return imp;
}

AST::Type*
XMLReaderImpl::parseType(xmlNodePtr& cur)
{
  return 0;
}

AST::Variable*
XMLReaderImpl::parseVariable(xmlNodePtr& cur)
{
  assert(getToken(cur->name)==TKN_var);
  AST::Locator loc(cur->line,0,&ast->getSystemID());
  std::string name, type;
  getNameType(cur, name, type);
  AST::Variable* var = ast->new_Variable(loc,name);
  var->setType(ast->resolveType(type));
  return var;
}

AST::Bundle*
XMLReaderImpl::parseBundle(xmlNodePtr& cur) 
{
  assert(getToken(cur->name) == TKN_bundle && "Expecting bundle element");
  std::string pubid(getAttribute(cur, "pubid"));
  AST::Locator loc(cur->line,0,&ast->getSystemID());
  AST::Bundle* bundle = ast->new_Bundle(loc,pubid);
  xmlNodePtr child = cur->children;
  while (child && skipBlanks(child) && child->type == XML_ELEMENT_NODE) 
  {
    int tkn = getToken(child->name);
    AST::Node* n = 0;
    switch (tkn) {
      case TKN_import   : n = parseImport(child); break;
      case TKN_bundle   : n = parseBundle(child); break;
      case TKN_function : n = parseFunction(child); break;
      case TKN_type:      n = parseType(child); break;
      case TKN_var:       n = parseVariable(child); break;
      default:
        assert(!"Invalid content for bundle");
        break;
    }
    if (n)
      bundle->addChild(n);
    child = child->next;
  }
  return bundle;
}

void
XMLReaderImpl::parseTree() 
{
  xmlNodePtr cur = xmlDocGetRootElement(doc);
  if (!cur) {
    error("No root node");
    return;
  }
  int tkn = getToken(cur->name);
  assert(tkn == TKN_hlvm && "Expecting hlvm element");
  cur = cur->children;
  if (skipBlanks(cur)) {
    AST::Bundle* bundle = parseBundle(cur);
    ast->setRoot(bundle);
  }
}

// Implement the read interface to parse, validate, and convert the
// XML document into AST Nodes. 
void
XMLReaderImpl::read() {

  // create the RelaxNG Parser Context
  xmlRelaxNGParserCtxtPtr rngparser =
    xmlRelaxNGNewMemParserCtxt(HLVMGrammar, sizeof(HLVMGrammar));
  if (!rngparser) {
    error("Failed to allocate RNG Parser Context");
    return;
  }

  // Provide the error handler for parsing the schema
  xmlRelaxNGSetParserStructuredErrors(rngparser, ParseHandler, this);

  // Parse the schema and build an internal structure for it
  xmlRelaxNGPtr schema = xmlRelaxNGParse(rngparser);
  if (!schema) {
    error("Failed to parse the RNG Schema");
    xmlRelaxNGFreeParserCtxt(rngparser);
    return;
  }

  // create a document parser context
  xmlParserCtxtPtr ctxt = xmlNewParserCtxt();
  if (!ctxt) {
    error("Failed to allocate document parser context");
    xmlRelaxNGFreeParserCtxt(rngparser);
    xmlRelaxNGFree(schema);
    return;
  }

  // Parse the file, creating a Document tree
  doc = xmlCtxtReadFile(ctxt, path.c_str(), 0, 0);
  if (!doc) {
    error("Failed to parse the document");
    xmlRelaxNGFreeParserCtxt(rngparser);
    xmlRelaxNGFree(schema);
    xmlFreeParserCtxt(ctxt);
    return;
  }

  // Create a validation context
  xmlRelaxNGValidCtxtPtr validation = xmlRelaxNGNewValidCtxt(schema);
  if (!validation) {
    error("Failed to create the validation context");
    xmlRelaxNGFreeParserCtxt(rngparser);
    xmlRelaxNGFree(schema);
    xmlFreeParserCtxt(ctxt);
    xmlFreeDoc(doc);
    doc = 0;
    return;
  }

  // Provide the error handler for parsing the schema
  xmlRelaxNGSetValidStructuredErrors(validation, ValidationHandler, this);

  // Validate the document with the schema
  if (xmlRelaxNGValidateDoc(validation, doc)) {
    error("Document didn't pass RNG schema validation");
    xmlRelaxNGFreeParserCtxt(rngparser);
    xmlRelaxNGFree(schema);
    xmlFreeParserCtxt(ctxt);
    xmlFreeDoc(doc);
    doc = 0;
    xmlRelaxNGFreeValidCtxt(validation);
    return;
  }

  // Parse
  parseTree();
  xmlRelaxNGFreeParserCtxt(rngparser);
  xmlRelaxNGFree(schema);
  xmlFreeParserCtxt(ctxt);
  xmlRelaxNGFreeValidCtxt(validation);
  xmlFreeDoc(doc);
  doc = 0;
}

AST::AST*
XMLReaderImpl::get()
{
  return ast;
}

}

XMLReader* 
hlvm::XMLReader::create(const std::string& src)
{
  return new XMLReaderImpl(src);
}

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
#include <hlvm/AST/Locator.h>
#include <hlvm/Base/Source.h>
#include <hlvm/AST/AST.h>
#include <hlvm/AST/Bundle.h>
#include <hlvm/AST/ContainerType.h>
#include <hlvm/AST/Documentation.h>
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
  AST* ast;
  xmlDocPtr doc;
public:
  XMLReaderImpl(const std::string& p)
    : path(p), ast(0)
  {
    ast = AST::create();
    ast->setSystemID(p);
  }

  virtual ~XMLReaderImpl() 
  { 
    if (ast) AST::destroy(ast); 
    if (doc) xmlFreeDoc(doc);
  }

  virtual void read();
  virtual AST* get();

  void error(const std::string& msg) {
    std::cerr << msg << "\n";
  }

  std::string lookupToken(int32_t token) const
  {
    return HLVMTokenizer::lookup(token);
  }

  inline void handleParseError(xmlErrorPtr error);
  inline void handleValidationError(xmlErrorPtr error);

  void                parseTree          ();
  AliasType*     parseAlias         (xmlNodePtr& cur);
  Type*          parseArray         (xmlNodePtr& cur);
  Type*          parseAtom          (xmlNodePtr& cur);
  Bundle*        parseBundle        (xmlNodePtr& cur);
  Documentation* parseDocumentation (xmlNodePtr& cur);
  Type*          parseEnumeration   (xmlNodePtr& cur);
  Function*      parseFunction      (xmlNodePtr& cur);
  Import*        parseImport        (xmlNodePtr& cur);
  Type*          parsePointer       (xmlNodePtr& cur);
  Type*          parseStructure     (xmlNodePtr& cur);
  Type*          parseSignature     (xmlNodePtr& cur);
  Variable*      parseVariable      (xmlNodePtr& cur);
  Type*          parseVector        (xmlNodePtr& cur);
  inline xmlNodePtr   checkDoc(xmlNodePtr cur, Documentable* node);
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


inline bool 
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

uint64_t
recognize_nonNegativeInteger(const char* str)
{
    return uint64_t(::atoll(str));
}

int64_t
recognize_Integer(const char * str)
{
  return ::atoll(str);
}

inline bool 
recognize_boolean(const std::string& str)
{
  switch (str[0])
  {
    case 'F': if (str == "FALSE") return false; break;
    case 'N': if (str == "NO")    return false; break;
    case 'T': if (str == "TRUE")  return true; break;
    case 'Y': if (str == "YES")   return true; break;
    case 'f': if (str == "false") return false; break;
    case 'n': if (str == "no")    return false; break;
    case 't': if (str == "true")  return true; break;
    case 'y': if (str == "yes")   return true; break;
    case '0': if (str == "0")     return false; break;
    case '1': if (str == "1")     return true; break;
    default: break;
  }
  assert(!"Invalid boolean value");
}

inline const char* 
getAttribute(xmlNodePtr cur,const char*name,bool required = true)
{
  const char* result = reinterpret_cast<const char*>(
   xmlGetNoNsProp(cur,reinterpret_cast<const xmlChar*>(name)));
  if (!result && required) {
    assert(!"Missing Attribute");
  }
  return result;
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

Documentation*
XMLReaderImpl::parseDocumentation(xmlNodePtr& cur)
{
  // Documentation is always optional so don't error out if the
  // node is not a TKN_doc
  if (cur && skipBlanks(cur) && getToken(cur->name) == TKN_doc) {
    Locator loc(cur->line,0,&ast->getSystemID());
    xmlBufferPtr buffer = xmlBufferCreate();
    xmlNodeDump(buffer,doc,cur,0,0);
    int length = xmlBufferLength(buffer);
    std::string str(reinterpret_cast<const char*>(xmlBufferContent(buffer)));
    str.erase(0,5); // Zap the <doc> at the start
    str.erase(str.length()-6); // Zap the </doc> at the end
    Documentation* progDoc = ast->new_Documentation(loc);
    progDoc->setDoc(str);
    xmlBufferFree(buffer);
    return progDoc;
  }
  // Just signal that there's no documentation in this node
  return 0;
}

inline xmlNodePtr
XMLReaderImpl::checkDoc(xmlNodePtr cur, Documentable* node)
{
  xmlNodePtr child = cur->children;
  Documentation* theDoc = parseDocumentation(child);
  if (theDoc) {
    node->setDoc(theDoc);
    return child->next;
  }
  return child;
}

Function*
XMLReaderImpl::parseFunction(xmlNodePtr& cur)
{
  assert(getToken(cur->name)==TKN_function);
  Locator loc(cur->line,0,&ast->getSystemID());
  std::string name, type;
  getNameType(cur, name, type);
  Function* func = ast->new_Function(loc,name);
  checkDoc(cur,func);
  return func;
}

Import*
XMLReaderImpl::parseImport(xmlNodePtr& cur)
{
  assert(getToken(cur->name)==TKN_import);
  Locator loc(cur->line,0,&ast->getSystemID());
  std::string pfx = getAttribute(cur,"prefix");
  Import* imp = ast->new_Import(loc,pfx);
  checkDoc(cur,imp);
  return imp;
}

AliasType*
XMLReaderImpl::parseAlias(xmlNodePtr& cur)
{
  assert(getToken(cur->name)==TKN_alias);
  Locator loc(cur->line,0,&ast->getSystemID());
  std::string name = getAttribute(cur,"name");
  std::string type = getAttribute(cur,"renames");
  AliasType* alias = ast->new_AliasType(loc,name,ast->resolveType(type));
  checkDoc(cur,alias);
  return alias;
}

Type*     
XMLReaderImpl::parseAtom(xmlNodePtr& cur)
{
  assert(getToken(cur->name)==TKN_atom);
  Locator loc(cur->line,0,&ast->getSystemID());
  std::string name = getAttribute(cur,"name");
  xmlNodePtr child = cur->children;
  Documentation* theDoc = parseDocumentation(child);
  child = (theDoc==0 ? child : child->next );
  Type* result = 0;
  if (child && skipBlanks(child) && child->type == XML_ELEMENT_NODE) {
    int tkn = getToken(child->name);
    switch (tkn) {
      case TKN_intrinsic: {
        const char* is = getAttribute(child,"is");
        if (!is)
          assert(!"intrinsic element requires 'is' attribute");
        int typeTkn = getToken(reinterpret_cast<const xmlChar*>(is));
        switch (typeTkn) {
          case TKN_any:  result=ast->new_AnyType(loc,name); break;
          case TKN_bool: result=ast->new_BooleanType(loc,name); break;
          case TKN_char: result=ast->new_CharacterType(loc,name); break;
          case TKN_f128: result=ast->new_f128(loc,name); break;
          case TKN_f32:  result=ast->new_f32(loc,name); break;
          case TKN_f43:  result=ast->new_f43(loc,name); break;
          case TKN_f64:  result=ast->new_f64(loc,name); break;
          case TKN_f80:  result=ast->new_f80(loc,name); break;
          case TKN_octet:result=ast->new_OctetType(loc,name); break;
          case TKN_s128: result=ast->new_s128(loc,name); break;
          case TKN_s16:  result=ast->new_s16(loc,name); break;
          case TKN_s32:  result=ast->new_s32(loc,name); break;
          case TKN_s64:  result=ast->new_s64(loc,name); break;
          case TKN_s8:   result=ast->new_s8(loc,name); break;
          case TKN_u128: result=ast->new_u128(loc,name); break;
          case TKN_u16:  result=ast->new_u16(loc,name); break;
          case TKN_u32:  result=ast->new_u32(loc,name); break;
          case TKN_u64:  result=ast->new_u64(loc,name); break;
          case TKN_u8:   result=ast->new_u8(loc,name); break;
          case TKN_void: result=ast->new_VoidType(loc,name); break;
          default:
            assert(!"Invalid intrinsic kind");
        }
        break;
      }
      case TKN_signed: {
        const char* bits = getAttribute(child,"bits");
        if (bits) {
          uint64_t numBits = recognize_nonNegativeInteger(bits);
          result = ast->new_IntegerType(loc,name,numBits,/*signed=*/true);
          break;
        }
        assert(!"Missing 'bits' attribute");
        break;
      }
      case TKN_unsigned: {
        const char* bits = getAttribute(child,"bits");
        if (bits) {
          uint64_t numBits = recognize_nonNegativeInteger(bits);
          result = ast->new_IntegerType(loc,name,numBits,/*signed=*/false);
          break;
        }
        assert(!"Missing 'bits' attribute");
        break;
      }      
      case TKN_range: {
        const char* min = getAttribute(child, "min");
        const char* max = getAttribute(child, "max");
        if (min && max) {
          int64_t minVal = recognize_Integer(min);
          int64_t maxVal = recognize_Integer(max);
          result = ast->new_RangeType(loc,name,minVal,maxVal);
          break;
        }
        assert(!"Missing 'min' or 'max' attribute");
        break;
      }
      case TKN_real: {
        const char* mantissa = getAttribute(child, "mantissa");
        const char* exponent = getAttribute(child, "exponent");
        if (mantissa && exponent) {
          int32_t mantVal = recognize_nonNegativeInteger(mantissa);
          int32_t expoVal = recognize_nonNegativeInteger(exponent);
          result = ast->new_RealType(loc,name,mantVal,expoVal);
        }
        break;
      }
      default:
        assert(!"Invalid content for atom");
        break;
    }
    if (result) {
      if (theDoc)
        result->setDoc(theDoc);
      return result;
    }
  }
  assert(!"Atom definition element expected");
}

Type*
XMLReaderImpl::parseEnumeration(xmlNodePtr& cur)
{
  assert(getToken(cur->name)==TKN_enumeration);
  Locator loc(cur->line,0,&ast->getSystemID());
  std::string name = getAttribute(cur,"name");
  EnumerationType* en = ast->new_EnumerationType(loc,name);
  xmlNodePtr child = checkDoc(cur,en);
  while (child && skipBlanks(child) && child->type == XML_ELEMENT_NODE) {
    assert(getToken(child->name) == TKN_enumerator);
    std::string id = getAttribute(child,"id");
    en->addEnumerator(id);
    child = child->next;
  }
  return en;
}

Type*     
XMLReaderImpl::parsePointer(xmlNodePtr& cur)
{
  assert(getToken(cur->name)==TKN_pointer);
  Locator loc(cur->line,0,&ast->getSystemID());
  std::string name = getAttribute(cur,"name");
  std::string type = getAttribute(cur,"to");
  PointerType* result = 
    ast->new_PointerType(loc,name,ast->resolveType(type));
  checkDoc(cur,result);
  return result;
}

Type*     
XMLReaderImpl::parseArray(xmlNodePtr& cur)
{
  assert(getToken(cur->name)==TKN_array);
  Locator loc(cur->line,0,&ast->getSystemID());
  std::string name = getAttribute(cur,"name");
  std::string type = getAttribute(cur,"of");
  const char* len = getAttribute(cur,"length");
  ArrayType* result = ast->new_ArrayType(
    loc, name, ast->resolveType(type), recognize_nonNegativeInteger(len));
  checkDoc(cur,result);
  return result;
}

Type*     
XMLReaderImpl::parseVector(xmlNodePtr& cur)
{
  assert(getToken(cur->name)==TKN_vector);
  Locator loc(cur->line,0,&ast->getSystemID());
  std::string name = getAttribute(cur,"name");
  std::string type = getAttribute(cur,"of");
  const char* len  = getAttribute(cur,"length");
  VectorType* result =
    ast->new_VectorType(
      loc,name,ast->resolveType(type), recognize_nonNegativeInteger(len));
  checkDoc(cur,result);
  return result;
}

Type*
XMLReaderImpl::parseStructure(xmlNodePtr& cur)
{
  assert(getToken(cur->name)==TKN_structure);
  Locator loc(cur->line,0,&ast->getSystemID());
  std::string name = getAttribute(cur,"name");
  StructureType* struc = ast->new_StructureType(loc,name);
  xmlNodePtr child = checkDoc(cur,struc); 
  while (child && skipBlanks(child) && child->type == XML_ELEMENT_NODE) {
    assert(getToken(child->name) == TKN_field && "Structure only has fields");
    std::string name = getAttribute(child,"name");
    std::string type = getAttribute(child,"type");
    AliasType* alias = ast->new_AliasType(loc,name,ast->resolveType(type));
    alias->setParent(struc);
    checkDoc(child,alias);
    child = child->next;
  }
  return struc;
}

Type*     
XMLReaderImpl::parseSignature(xmlNodePtr& cur)
{
  assert(getToken(cur->name)==TKN_signature);
  Locator loc(cur->line,0,&ast->getSystemID());
  std::string name = getAttribute(cur,"name");
  std::string result = getAttribute(cur,"result");
  const char* varargs = getAttribute(cur,"varargs",false);
  SignatureType* sig = 
    ast->new_SignatureType(loc,name,ast->resolveType(result));
  if (varargs)
    sig->setIsVarArgs(recognize_boolean(varargs));
  xmlNodePtr child = checkDoc(cur,sig); 
  while (child && skipBlanks(child) && child->type == XML_ELEMENT_NODE) {
    assert(getToken(child->name) == TKN_arg && "Signature only has args");
    std::string name = getAttribute(child,"name");
    std::string type = getAttribute(child,"type");
    AliasType* alias = ast->new_AliasType(loc,name,ast->resolveType(type));
    alias->setParent(sig);
    checkDoc(child,alias);
    child = child->next;
  }
  return sig;
}

Variable*
XMLReaderImpl::parseVariable(xmlNodePtr& cur)
{
  assert(getToken(cur->name)==TKN_var);
  Locator loc(cur->line,0,&ast->getSystemID());
  std::string name, type;
  getNameType(cur, name, type);
  Variable* var = ast->new_Variable(loc,name);
  var->setType(ast->resolveType(type));
  checkDoc(cur,var);
  return var;
}

Bundle*
XMLReaderImpl::parseBundle(xmlNodePtr& cur) 
{
  assert(getToken(cur->name) == TKN_bundle && "Expecting bundle element");
  std::string pubid(getAttribute(cur, "pubid"));
  Locator loc(cur->line,0,&ast->getSystemID());
  Bundle* bundle = ast->new_Bundle(loc,pubid);
  xmlNodePtr child = cur->children;
  while (child && skipBlanks(child) && child->type == XML_ELEMENT_NODE) 
  {
    int tkn = getToken(child->name);
    Node* n = 0;
    switch (tkn) {
      case TKN_doc      :
        Documentation* theDoc = parseDocumentation(child);
        if (theDoc)
          bundle->setDoc(theDoc);
        break;
      case TKN_import   : n = parseImport(child); break;
      case TKN_bundle   : n = parseBundle(child); break;
      case TKN_function : n = parseFunction(child); break;
      case TKN_alias    : n = parseAlias(child); break;
      case TKN_atom     : n = parseAtom(child); break;
      case TKN_enumeration: n = parseEnumeration(child); break;
      case TKN_pointer  : n = parsePointer(child); break;
      case TKN_array    : n = parseArray(child); break;
      case TKN_vector   : n = parseVector(child); break;
      case TKN_structure: n = parseStructure(child); break;
      case TKN_signature: n = parseSignature(child); break;
      case TKN_var      : n = parseVariable(child); break;
      default:
        assert(!"Invalid content for bundle");
        break;
    }
    if (n)
      n->setParent(bundle); 
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
    Bundle* bundle = parseBundle(cur);
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

AST*
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

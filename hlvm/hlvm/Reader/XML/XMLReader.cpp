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
#include <hlvm/AST/Program.h>
#include <hlvm/AST/Constants.h>
#include <hlvm/AST/ControlFlow.h>
#include <hlvm/Base/Assert.h>
#include <libxml/parser.h>
#include <libxml/relaxng.h>
#include <vector>
#include <string>
#include <iostream>

using namespace hlvm;
using namespace HLVM_Reader_XML;

namespace {

const char HLVMGrammar[] = 
#include <hlvm/Reader/XML/HLVM.rng.inc>
;

class XMLReaderImpl : public XMLReader {
  std::string path;
  AST* ast;
  xmlDocPtr doc;
  Locator* loc;
  URI* uri;
public:
  XMLReaderImpl(const std::string& p)
    : path(p), ast(0), loc(0)
  {
    ast = AST::create();
    ast->setSystemID(p);
    uri = URI::create(p,ast->getPool());
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

  Locator* getLocator(xmlNodePtr& cur) {
    if (loc) {
      LineLocator tmp(uri,cur->line);
      if (*loc == tmp)
        return loc;
    }
    return loc = new LineLocator(uri,cur->line);
  }

  inline void handleParseError(xmlErrorPtr error);
  inline void handleValidationError(xmlErrorPtr error);

  inline ConstantInteger* parseBinary(xmlNodePtr& cur);
  inline ConstantInteger* parseOctal(xmlNodePtr& cur);
  inline ConstantInteger* parseDecimal(xmlNodePtr& cur);
  inline ConstantInteger* parseHexadecimal(xmlNodePtr& cur);
  inline ConstantReal*    parseFloat(xmlNodePtr& cur);
  inline ConstantReal*    parseDouble(xmlNodePtr& cur);
  inline ConstantText*    parseText(xmlNodePtr& cur);
  void           parseTree          ();
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
  Constant*      parseConstant      (xmlNodePtr& cur);
  Constant*      parseConstant      (xmlNodePtr& cur, int token);
  Operator*      parseOperator      (xmlNodePtr& cur);
  Operator*      parseOperator      (xmlNodePtr& cur, int token);
  Value*         parseValue         (xmlNodePtr& cur);
  Value*         parseValue         (xmlNodePtr& cur, int token);
  ReturnOp*      parseReturn        (xmlNodePtr& cur);
  Block*         parseBlock         (xmlNodePtr& cur);
  Program*       parseProgram       (xmlNodePtr& cur);
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


inline int 
getToken(const xmlChar* name)
{
  return HLVMTokenizer::recognize(reinterpret_cast<const char*>(name));
}

inline int
getToken(const char* name)
{
  return HLVMTokenizer::recognize(name);
}

inline bool 
skipBlanks(xmlNodePtr &cur, bool skipText = true)
{
  while (cur) {
    switch (cur->type) {
      case XML_TEXT_NODE:
        if (!skipText)
          return true;
        /* FALL THROUGH */
      case XML_COMMENT_NODE:
      case XML_PI_NODE:
        break;
      default:
        return true;
    }
    cur = cur->next;
  }
  return cur != 0;
}

LinkageKinds
recognize_LinkageKinds(const char* str)
{
  switch (getToken(str)) 
  {
    case TKN_weak      : return WeakLinkage;
    case TKN_appending : return AppendingLinkage;
    case TKN_external  : return ExternalLinkage;
    case TKN_internal  : return InternalLinkage;
    case TKN_linkonce  : return LinkOnceLinkage;
    default:
      hlvmDeadCode("Invalid Linkage Type");
  }
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
  hlvmDeadCode("Invalid boolean value");
  return 0;
}

inline const char* 
getAttribute(xmlNodePtr cur,const char*name,bool required = true)
{
  const char* result = reinterpret_cast<const char*>(
   xmlGetNoNsProp(cur,reinterpret_cast<const xmlChar*>(name)));
  if (!result && required) {
    hlvmAssert(!"Missing Attribute");
  }
  return result;
}

inline void 
getNameType(xmlNodePtr& cur, std::string& name, std::string& type)
{
  name = getAttribute(cur,"id");
  type = getAttribute(cur,"type");
}

inline ConstantInteger*
XMLReaderImpl::parseBinary(xmlNodePtr& cur)
{
  uint64_t value = 0;
  xmlNodePtr child = cur->children;
  if (child) skipBlanks(child,false);
  while (child && child->type == XML_TEXT_NODE) {
    const xmlChar* p = child->content;
    while (*p != 0) {
      value <<= 1;
      hlvmAssert(*p == '1' || *p == '0');
      value |= (*p++ == '1' ? 1 : 0);
    }
    child = child->next;
  }
  if (child) skipBlanks(child);
  hlvmAssert(!child && "Illegal chlldren of <bin> element");
  ConstantInteger* result = ast->new_ConstantInteger(value,getLocator(cur));
  result->setType(ast->getPrimitiveType(UInt64TypeID));
  return result;
}

inline ConstantInteger*
XMLReaderImpl::parseOctal(xmlNodePtr& cur)
{
  uint64_t value = 0;
  xmlNodePtr child = cur->children;
  if (child) skipBlanks(child,false);
  while (child && child->type == XML_TEXT_NODE) {
    const xmlChar* p = cur->content;
    while (*p != 0) {
      value <<= 3;
      hlvmAssert(*p >= '0' || *p == '7');
      value |= *p++ - '0';
    }
    child = child->next;
  }
  if (child) skipBlanks(child);
  hlvmAssert(!child && "Illegal chlldren of <oct> element");
  ConstantInteger* result = ast->new_ConstantInteger(value,getLocator(cur));
  result->setType(ast->getPrimitiveType(UInt64TypeID));
  return result;
}

inline ConstantInteger*
XMLReaderImpl::parseDecimal(xmlNodePtr& cur)
{
  uint64_t value = 0;
  xmlNodePtr child = cur->children;
  if (child) skipBlanks(child,false);
  while (child && child->type == XML_TEXT_NODE) {
    const xmlChar* p = child->content;
    while (*p != 0) {
      value *= 10;
      hlvmAssert(*p >= '0' || *p <= '9');
      value |= *p++ - '0';
    }
    child = child->next;
  }
  if (child) skipBlanks(child);
  hlvmAssert(!child && "Illegal chlldren of <dec> element");
  ConstantInteger* result = ast->new_ConstantInteger(value,getLocator(cur));
  result->setType(ast->getPrimitiveType(UInt64TypeID));
  return result;
}

inline ConstantInteger*
XMLReaderImpl::parseHexadecimal(xmlNodePtr& cur)
{
  uint64_t value = 0;
  xmlNodePtr child = cur->children;
  if (child) skipBlanks(child,false);
  while (child && child->type == XML_TEXT_NODE) {
    const xmlChar* p = cur->content;
    while (*p != 0) {
      value <<= 4;
      switch (*p) {
        case '0': break;
        case '1': value |= 1; break;
        case '2': value |= 2; break;
        case '3': value |= 3; break;
        case '4': value |= 4; break;
        case '5': value |= 5; break;
        case '6': value |= 6; break;
        case '7': value |= 7; break;
        case '8': value |= 8; break;
        case '9': value |= 9; break;
        case 'a': case 'A': value |= 10; break;
        case 'b': case 'B': value |= 11; break;
        case 'c': case 'C': value |= 12; break;
        case 'd': case 'D': value |= 13; break;
        case 'e': case 'E': value |= 14; break;
        case 'f': case 'F': value |= 15; break;
        default:
          hlvmAssert("Invalid hex digit");
          break;
      }
      p++;
    }
    child = child->next;
  }
  if (child) skipBlanks(child);
  hlvmAssert(!child && "Illegal chlldren of <hex> element");
  ConstantInteger* result = ast->new_ConstantInteger(value,getLocator(cur));
  result->setType(ast->getPrimitiveType(UInt64TypeID));
  return result;
}

inline ConstantReal*
XMLReaderImpl::parseFloat(xmlNodePtr& cur)
{
  double value = 0;
  std::string buffer;
  xmlNodePtr child = cur->children;
  if (child) skipBlanks(child,false);
  while (child && child->type == XML_TEXT_NODE) {
    buffer += reinterpret_cast<const char*>(cur->content);
    child = child->next;
  }
  if (child) skipBlanks(child);
  hlvmAssert(!child && "Illegal chlldren of <flt> element");
  value = atof(buffer.c_str());
  ConstantReal* result = ast->new_ConstantReal(value,getLocator(cur));
  result->setType(ast->getPrimitiveType(UInt64TypeID));
  return result;
}

inline ConstantReal*
XMLReaderImpl::parseDouble(xmlNodePtr& cur)
{
  double value = 0;
  std::string buffer;
  xmlNodePtr child = cur->children;
  if (child) skipBlanks(child,false);
  while (child && child->type == XML_TEXT_NODE) {
    buffer += reinterpret_cast<const char*>(cur->content);
    child = child->next;
  }
  if (child) skipBlanks(child);
  hlvmAssert(!child && "Illegal chlldren of <dbl> element");
  value = atof(buffer.c_str());
  ConstantReal* result = ast->new_ConstantReal(value,getLocator(cur));
  result->setType(ast->getPrimitiveType(UInt64TypeID));
  return result;
}

inline ConstantText*
XMLReaderImpl::parseText(xmlNodePtr& cur)
{
  std::string buffer;
  xmlNodePtr child = cur->children;
  if (child) skipBlanks(child,false);
  while (child && child->type == XML_TEXT_NODE) {
    buffer += reinterpret_cast<const char*>(cur->content);
    child = child->next;
  }
  if (child) skipBlanks(child);
  hlvmAssert(!child && "Illegal chlldren of <text> element");
  return ast->new_ConstantText(buffer,getLocator(cur));
}

Documentation*
XMLReaderImpl::parseDocumentation(xmlNodePtr& cur)
{
  // Documentation is always optional so don't error out if the
  // node is not a TKN_doc
  if (cur && skipBlanks(cur) && getToken(cur->name) == TKN_doc) {
    xmlBufferPtr buffer = xmlBufferCreate();
    xmlNodeDump(buffer,doc,cur,0,0);
    int length = xmlBufferLength(buffer);
    std::string 
      str(reinterpret_cast<const char*>(xmlBufferContent(buffer)),length);
    str.erase(0,5); // Zap the <doc> at the start
    str.erase(str.length()-6); // Zap the </doc> at the end
    Documentation* progDoc = ast->new_Documentation(getLocator(cur));
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

Import*
XMLReaderImpl::parseImport(xmlNodePtr& cur)
{
  hlvmAssert(getToken(cur->name)==TKN_import);
  std::string pfx = getAttribute(cur,"prefix");
  Import* imp = ast->new_Import(pfx,getLocator(cur));
  checkDoc(cur,imp);
  return imp;
}

AliasType*
XMLReaderImpl::parseAlias(xmlNodePtr& cur)
{
  hlvmAssert(getToken(cur->name)==TKN_alias);
  std::string name = getAttribute(cur,"id");
  std::string type = getAttribute(cur,"renames");
  AliasType* alias = 
    ast->new_AliasType(name,ast->resolveType(type),getLocator(cur));
  checkDoc(cur,alias);
  return alias;
}

Type*     
XMLReaderImpl::parseAtom(xmlNodePtr& cur)
{
  hlvmAssert(getToken(cur->name)==TKN_atom);
  Locator* loc = getLocator(cur);
  std::string name = getAttribute(cur,"id");
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
          hlvmAssert(!"intrinsic element requires 'is' attribute");
        int typeTkn = getToken(reinterpret_cast<const xmlChar*>(is));
        switch (typeTkn) {
          case TKN_any:  result=ast->new_AnyType(name,loc); break;
          case TKN_bool: result=ast->new_BooleanType(name,loc); break;
          case TKN_char: result=ast->new_CharacterType(name,loc); break;
          case TKN_f128: result=ast->new_f128(name,loc); break;
          case TKN_f32:  result=ast->new_f32(name,loc); break;
          case TKN_f44:  result=ast->new_f44(name,loc); break;
          case TKN_f64:  result=ast->new_f64(name,loc); break;
          case TKN_f80:  result=ast->new_f80(name,loc); break;
          case TKN_octet:result=ast->new_OctetType(name,loc); break;
          case TKN_s128: result=ast->new_s128(name,loc); break;
          case TKN_s16:  result=ast->new_s16(name,loc); break;
          case TKN_s32:  result=ast->new_s32(name,loc); break;
          case TKN_s64:  result=ast->new_s64(name,loc); break;
          case TKN_s8:   result=ast->new_s8(name,loc); break;
          case TKN_u128: result=ast->new_u128(name,loc); break;
          case TKN_u16:  result=ast->new_u16(name,loc); break;
          case TKN_u32:  result=ast->new_u32(name,loc); break;
          case TKN_u64:  result=ast->new_u64(name,loc); break;
          case TKN_u8:   result=ast->new_u8(name,loc); break;
          case TKN_void: result=ast->new_VoidType(name,loc); break;
          default:
            hlvmDeadCode("Invalid intrinsic kind");
        }
        break;
      }
      case TKN_signed: {
        const char* bits = getAttribute(child,"bits");
        if (bits) {
          uint64_t numBits = recognize_nonNegativeInteger(bits);
          result = ast->new_IntegerType(name,numBits,/*signed=*/true,loc);
          break;
        }
        hlvmAssert(!"Missing 'bits' attribute");
        break;
      }
      case TKN_unsigned: {
        const char* bits = getAttribute(child,"bits");
        if (bits) {
          uint64_t numBits = recognize_nonNegativeInteger(bits);
          result = ast->new_IntegerType(name,numBits,/*signed=*/false,loc);
          break;
        }
        hlvmAssert(!"Missing 'bits' attribute");
        break;
      }      
      case TKN_range: {
        const char* min = getAttribute(child, "min");
        const char* max = getAttribute(child, "max");
        if (min && max) {
          int64_t minVal = recognize_Integer(min);
          int64_t maxVal = recognize_Integer(max);
          result = ast->new_RangeType(name,minVal,maxVal,loc);
          break;
        }
        hlvmAssert(!"Missing 'min' or 'max' attribute");
        break;
      }
      case TKN_real: {
        const char* mantissa = getAttribute(child, "mantissa");
        const char* exponent = getAttribute(child, "exponent");
        if (mantissa && exponent) {
          int32_t mantVal = recognize_nonNegativeInteger(mantissa);
          int32_t expoVal = recognize_nonNegativeInteger(exponent);
          result = ast->new_RealType(name,mantVal,expoVal,loc);
        }
        break;
      }
      default:
        hlvmAssert(!"Invalid content for atom");
        break;
    }
    if (result) {
      if (theDoc)
        result->setDoc(theDoc);
      return result;
    }
  }
  hlvmAssert(!"Atom definition element expected");
  return 0;
}

Type*
XMLReaderImpl::parseEnumeration(xmlNodePtr& cur)
{
  hlvmAssert(getToken(cur->name)==TKN_enumeration);
  Locator* loc = getLocator(cur);
  std::string name = getAttribute(cur,"id");
  EnumerationType* en = ast->new_EnumerationType(name,loc);
  xmlNodePtr child = checkDoc(cur,en);
  while (child && skipBlanks(child) && child->type == XML_ELEMENT_NODE) {
    hlvmAssert(getToken(child->name) == TKN_enumerator);
    std::string id = getAttribute(child,"id");
    en->addEnumerator(id);
    child = child->next;
  }
  return en;
}

Type*     
XMLReaderImpl::parsePointer(xmlNodePtr& cur)
{
  hlvmAssert(getToken(cur->name)==TKN_pointer);
  Locator* loc = getLocator(cur);
  std::string name = getAttribute(cur,"id");
  std::string type = getAttribute(cur,"to");
  PointerType* result = 
    ast->new_PointerType(name,ast->resolveType(type),loc);
  checkDoc(cur,result);
  return result;
}

Type*     
XMLReaderImpl::parseArray(xmlNodePtr& cur)
{
  hlvmAssert(getToken(cur->name)==TKN_array);
  Locator* loc = getLocator(cur);
  std::string name = getAttribute(cur,"id");
  std::string type = getAttribute(cur,"of");
  const char* len = getAttribute(cur,"length");
  ArrayType* result = ast->new_ArrayType(
    name, ast->resolveType(type), recognize_nonNegativeInteger(len),loc);
  checkDoc(cur,result);
  return result;
}

Type*     
XMLReaderImpl::parseVector(xmlNodePtr& cur)
{
  hlvmAssert(getToken(cur->name)==TKN_vector);
  Locator* loc = getLocator(cur);
  std::string name = getAttribute(cur,"id");
  std::string type = getAttribute(cur,"of");
  const char* len  = getAttribute(cur,"length");
  VectorType* result =
    ast->new_VectorType(
      name,ast->resolveType(type), recognize_nonNegativeInteger(len),loc);
  checkDoc(cur,result);
  return result;
}

Type*
XMLReaderImpl::parseStructure(xmlNodePtr& cur)
{
  hlvmAssert(getToken(cur->name)==TKN_structure);
  Locator* loc = getLocator(cur);
  std::string name = getAttribute(cur,"id");
  StructureType* struc = ast->new_StructureType(name,loc);
  xmlNodePtr child = checkDoc(cur,struc); 
  while (child && skipBlanks(child) && child->type == XML_ELEMENT_NODE) {
    hlvmAssert(getToken(child->name) == TKN_field && 
               "Structure only has fields");
    std::string name = getAttribute(child,"id");
    std::string type = getAttribute(child,"type");
    AliasType* alias = ast->new_AliasType(name,ast->resolveType(type),loc);
    alias->setParent(struc);
    checkDoc(child,alias);
    child = child->next;
  }
  return struc;
}

Type*     
XMLReaderImpl::parseSignature(xmlNodePtr& cur)
{
  hlvmAssert(getToken(cur->name)==TKN_signature);
  Locator* loc = getLocator(cur);
  std::string name = getAttribute(cur,"id");
  std::string result = getAttribute(cur,"result");
  const char* varargs = getAttribute(cur,"varargs",false);
  SignatureType* sig = 
    ast->new_SignatureType(name,ast->resolveType(result),loc);
  if (varargs)
    sig->setIsVarArgs(recognize_boolean(varargs));
  xmlNodePtr child = checkDoc(cur,sig); 
  while (child && skipBlanks(child) && child->type == XML_ELEMENT_NODE) {
    hlvmAssert(getToken(child->name) == TKN_arg && "Signature only has args");
    std::string name = getAttribute(child,"id");
    std::string type = getAttribute(child,"type");
    AliasType* alias = ast->new_AliasType(name,ast->resolveType(type),loc);
    alias->setParent(sig);
    checkDoc(child,alias);
    child = child->next;
  }
  return sig;
}

Variable*
XMLReaderImpl::parseVariable(xmlNodePtr& cur)
{
  hlvmAssert(getToken(cur->name)==TKN_variable);
  Locator* loc = getLocator(cur);
  std::string name, type;
  getNameType(cur, name, type);
  const char* cnst = getAttribute(cur, "const", false);
  const char* lnkg = getAttribute(cur, "linkage", false);
  const Type* Ty = ast->resolveType(type);
  Variable* var = ast->new_Variable(name,Ty,loc);
  if (cnst)
    var->setIsConstant(recognize_boolean(cnst));
  if (lnkg)
    var->setLinkageKind(recognize_LinkageKinds(lnkg));
  xmlNodePtr child = checkDoc(cur,var);
  if (child && skipBlanks(child) && child->type == XML_ELEMENT_NODE) {
    Constant* C = parseConstant(child);
    var->setInitializer(C);
  }
  return var;
}

ReturnOp*
XMLReaderImpl::parseReturn(xmlNodePtr& cur)
{
  hlvmAssert(getToken(cur->name) == TKN_ret && "Expecting block element");
  Locator* loc = getLocator(cur);
  ReturnOp* ret = ast->new_ReturnOp(loc);
  xmlNodePtr child = cur->children;
  if (child && skipBlanks(child) && child->type == XML_ELEMENT_NODE) {
    Value* op = parseValue(child);
    ret->setResult(op);
  }
  return ret;
}

inline Constant*
XMLReaderImpl::parseConstant(xmlNodePtr& cur)
{
  return parseConstant(cur, getToken(cur->name));
}

Constant*
XMLReaderImpl::parseConstant(xmlNodePtr& cur, int tkn)
{
  Constant* C = 0;
  switch (tkn) {
    case TKN_bin:          C = parseBinary(cur); break;
    case TKN_oct:          C = parseOctal(cur); break;
    case TKN_dec:          C = parseDecimal(cur); break;
    case TKN_hex:          C = parseHexadecimal(cur); break;
    case TKN_flt:          C = parseFloat(cur); break;
    case TKN_dbl:          C = parseDouble(cur); break;
    case TKN_text:         C = parseText(cur); break;
    default:
      hlvmAssert(!"Invalid kind of constant");
      break;
  }
  return C;
}


inline Operator*
XMLReaderImpl::parseOperator(xmlNodePtr& cur)
{
  return parseOperator(cur, getToken(cur->name));
}

Operator*
XMLReaderImpl::parseOperator(xmlNodePtr& cur, int tkn)
{
  Operator* op = 0;
  switch (tkn) {
    case TKN_ret:          op = parseReturn(cur); break;
    case TKN_block:        op = parseBlock(cur); break;
    case TKN_store:
    case TKN_load:
    case TKN_open:
    case TKN_write:
    case TKN_close:
      std::cerr << "Operator " << cur->name << " not imlpemented.\n";
      break;
    default:
      hlvmDeadCode("Unrecognized operator");
      break;
  }
  return op;
}

inline Value*
XMLReaderImpl::parseValue(xmlNodePtr& cur)
{
  return parseValue(cur,getToken(cur->name));
}

Value*
XMLReaderImpl::parseValue(xmlNodePtr& cur, int tkn)
{
  Value* v = 0;
  switch (tkn) {
    case TKN_bin:
    case TKN_oct:
    case TKN_dec:
    case TKN_hex:
    case TKN_flt:
    case TKN_dbl:
    case TKN_text:
    case TKN_zero:
      v = parseConstant(cur,tkn);
      break;
    case TKN_ret:
    case TKN_open:
    case TKN_write:
    case TKN_close:
    case TKN_store:
    case TKN_load:
    case TKN_block: 
      v = parseOperator(cur,tkn);
      break;
    case TKN_program:
      v = parseProgram(cur);
      break;
    case TKN_function:
      v = parseFunction(cur);
      break;
    case TKN_variable:
      v = parseVariable(cur);
      break;
    default:
      hlvmDeadCode("Unrecognized operator");
      break;
  }
  return v;
}

Block*
XMLReaderImpl::parseBlock(xmlNodePtr& cur)
{
  hlvmAssert(getToken(cur->name) == TKN_block && "Expecting block element");
  Locator* loc = getLocator(cur);
  const char* label = getAttribute(cur, "label",false);
  Block* block = 0;
  if (label)
    block = ast->new_Block(label,loc);
  else
    block = ast->new_Block("",loc);
  xmlNodePtr child = cur->children;
  while (child && skipBlanks(child) && child->type == XML_ELEMENT_NODE) 
  {
    Value* op = parseValue(child);
    block->addOperand(op);
    child = child->next;
  }
  return block;
}

Function*
XMLReaderImpl::parseFunction(xmlNodePtr& cur)
{
  hlvmAssert(getToken(cur->name)==TKN_function);
  Locator* loc = getLocator(cur);
  std::string name, type;
  getNameType(cur, name, type);
  Function* func = ast->new_Function(name,loc);
  const char* lnkg = getAttribute(cur, "linkage", false);
  if (lnkg)
    func->setLinkageKind(recognize_LinkageKinds(lnkg));
  checkDoc(cur,func);
  return func;
}

Program*
XMLReaderImpl::parseProgram(xmlNodePtr& cur)
{
  hlvmAssert(getToken(cur->name) == TKN_program && "Expecting program element");
  Locator* loc = getLocator(cur);
  std::string name(getAttribute(cur, "id"));
  Program* program = ast->new_Program(name,loc);
  xmlNodePtr child = cur->children;
  if (child && skipBlanks(child) && child->type == XML_ELEMENT_NODE) {
    Block* b = parseBlock(child);
    program->setBlock(b);
  } else {
    hlvmDeadCode("Program Without Block!");
  }
  return program;
}

Bundle*
XMLReaderImpl::parseBundle(xmlNodePtr& cur) 
{
  hlvmAssert(getToken(cur->name) == TKN_bundle && "Expecting bundle element");
  std::string pubid(getAttribute(cur, "id"));
  Locator* loc = getLocator(cur);
  Bundle* bundle = ast->new_Bundle(pubid,loc);
  xmlNodePtr child = cur->children;
  while (child && skipBlanks(child) && child->type == XML_ELEMENT_NODE) 
  {
    int tkn = getToken(child->name);
    Node* n = 0;
    switch (tkn) {
      case TKN_doc      : {
        Documentation* theDoc = parseDocumentation(child);
        if (theDoc)
          bundle->setDoc(theDoc);
        break;
      }
      case TKN_import      : { n = parseImport(child); break; }
      case TKN_bundle      : { n = parseBundle(child); break; }
      case TKN_alias       : { n = parseAlias(child); break; }
      case TKN_atom        : { n = parseAtom(child); break; }
      case TKN_enumeration : { n = parseEnumeration(child); break; }
      case TKN_pointer     : { n = parsePointer(child); break; }
      case TKN_array       : { n = parseArray(child); break; }
      case TKN_vector      : { n = parseVector(child); break; }
      case TKN_structure   : { n = parseStructure(child); break; }
      case TKN_signature   : { n = parseSignature(child); break; }
      case TKN_variable    : { n = parseVariable(child); break; }
      case TKN_program     : { n = parseProgram(child); break; }
      case TKN_function    : { n = parseFunction(child); break; }
      default:
      {
        hlvmDeadCode("Invalid content for bundle");
        break;
      }
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
  hlvmAssert(getToken(cur->name) == TKN_hlvm && "Expecting hlvm element");
  const std::string pubid = getAttribute(cur,"pubid");
  ast->setPublicID(pubid);
  cur = cur->children;
  if (skipBlanks(cur)) {
    Bundle* bundle = parseBundle(cur);
    ast->addBundle(bundle);
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

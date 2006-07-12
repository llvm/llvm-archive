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
/// @file hlvm/Reader/XMLReader.cpp
/// @author Reid Spencer <rspencer@x10sys.com>
/// @date 2006/05/12
/// @since 0.1.0
/// @brief Provides the interface to hlvm::XMLReader
//===----------------------------------------------------------------------===//

#include <hlvm/Reader/XMLReader.h>
#include <hlvm/Reader/HLVMTokenizer.h>
#include <hlvm/AST/Locator.h>
#include <hlvm/Base/Source.h>
#include <hlvm/AST/AST.h>
#include <hlvm/AST/Bundle.h>
#include <hlvm/AST/ContainerType.h>
#include <hlvm/AST/Documentation.h>
#include <hlvm/AST/Linkables.h>
#include <hlvm/AST/Constants.h>
#include <hlvm/AST/ControlFlow.h>
#include <hlvm/AST/MemoryOps.h>
#include <hlvm/AST/InputOutput.h>
#include <hlvm/AST/Arithmetic.h>
#include <hlvm/AST/BooleanOps.h>
#include <hlvm/AST/RealMath.h>
#include <hlvm/Base/Assert.h>
#include <llvm/ADT/StringExtras.h>
#include <libxml/parser.h>
#include <libxml/relaxng.h>
#include <vector>
#include <string>
#include <iostream>

using namespace hlvm;
using namespace HLVM_Reader;

namespace {

const char HLVMGrammar[] = 
#include <hlvm/Reader/HLVM.rng.inc>
;

class XMLReaderImpl : public XMLReader {
  std::string path;
  AST* ast;
  xmlDocPtr doc;
  Locator* loc;
  URI* uri;
  Block* block;
  typedef std::vector<Block*> BlockStack;
  BlockStack blocks;
  Function* func;
  Bundle* bundle;
  bool isError;
public:
  XMLReaderImpl(const std::string& p)
    : path(p), ast(0), loc(0), uri(0), block(0), blocks(), func(0), bundle(0), 
      isError(0)
  {
    ast = AST::create();
    ast->setSystemID(p);
    uri = ast->new_URI(p);
  }

  virtual ~XMLReaderImpl() 
  { 
    if (ast) AST::destroy(ast); 
    if (doc) xmlFreeDoc(doc);
  }

  virtual bool read();
  virtual AST* get();

  std::string lookupToken(int32_t token) const
  {
    return HLVMTokenizer::lookup(token);
  }

  Locator* getLocator(xmlNodePtr& cur) {
    if (loc) {
      if (loc->getLine() == cur->line)
        return loc;
    }
    return loc = ast->new_Locator(uri,cur->line);
  }

  inline Type* getType(const std::string& name );
  bool checkNewType(const std::string& name, Locator* loc);


  inline void handleParseError(xmlErrorPtr error);
  inline void handleValidationError(xmlErrorPtr error);

  inline void error(Locator* loc, const std::string& msg);

  inline xmlNodePtr   checkDoc(xmlNodePtr cur, Documentable* node);

  ConstantValue* parseLiteralConstant(xmlNodePtr& cur, const std::string& name,
    const Type* Ty);
  Constant*      parseConstant      (xmlNodePtr& cur);
  Operator*      parseOperator      (xmlNodePtr& cur);
  void           parseTree          ();
  Type*          parseInteger       (xmlNodePtr& cur, bool isSigned);

  template<class OpClass>
  OpClass*       parse(xmlNodePtr& cur);

  template<class OpClass>
  OpClass* parseNilaryOp(xmlNodePtr& cur);
  template<class OpClass>
  OpClass* parseUnaryOp(xmlNodePtr& cur);
  template<class OpClass>
  OpClass* parseBinaryOp(xmlNodePtr& cur);
  template<class OpClass>
  OpClass* parseTernaryOp(xmlNodePtr& cur);
  template<class OpClass>
  OpClass* parseMultiOp(xmlNodePtr& cur);

  inline const char* 
  getAttribute(xmlNodePtr cur,const char*name,bool required = true);
  inline void getTextContent(xmlNodePtr cur, std::string& buffer);
  inline bool getNameType(
    xmlNodePtr& cur, std::string& name,std::string& type, bool required = true);
  inline Type* createIntrinsicType(
    const std::string& tname, const std::string& name, Locator* loc);

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
  return ExternalLinkage;
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
recognize_boolean(const char* str)
{
  switch (getToken(str))
  {
    case TKN_FALSE: return false;
    case TKN_False: return false;
    case TKN_false: return false;
    case TKN_NO: return false;
    case TKN_No: return false;
    case TKN_no: return false;
    case TKN_0: return false;
    case TKN_TRUE: return true;
    case TKN_True: return true;
    case TKN_true: return true;
    case TKN_YES: return true;
    case TKN_Yes: return true;
    case TKN_yes: return true;
    case TKN_1: return true;
    default: break;
  }
  hlvmDeadCode("Invalid boolean value");
  return 0;
}

inline const char* 
XMLReaderImpl::getAttribute(xmlNodePtr cur,const char*name,bool required )
{
  const char* result = reinterpret_cast<const char*>(
   xmlGetNoNsProp(cur,reinterpret_cast<const xmlChar*>(name)));
  if (!result && required) {
    error(getLocator(cur),std::string("Requred Attribute '") + name + 
          "' is missing.");
  }
  return result;
}

inline void
XMLReaderImpl::getTextContent(xmlNodePtr cur, std::string& buffer)
{
  buffer.clear();
  if (cur) skipBlanks(cur,false);
  while (cur && cur->type == XML_TEXT_NODE) {
    buffer += reinterpret_cast<const char*>(cur->content);
    cur = cur->next;
  }
  if (cur) skipBlanks(cur);
}

inline bool 
XMLReaderImpl::getNameType(
  xmlNodePtr& cur, 
  std::string& name,
  std::string& type,
  bool required)
{
  name.clear();
  type.clear();
  const char* nm = getAttribute(cur,"id",required);
  if (nm)
    name = nm;
  const char* ty = getAttribute(cur,"type",required);
  if (ty)
    type = ty;
  return !required || (nm && ty);
}

inline void
XMLReaderImpl::error(Locator* loc, const std::string& msg)
{
  std::string location;
  if (loc)
    loc->getLocation(location);
  else
    location = "Unknown Location";
  std::cerr << location << ": " << msg << "\n";
  isError = true;
}

Type*
XMLReaderImpl::getType(const std::string& name)
{
  IntrinsicTypes IT = bundle->getIntrinsicTypesValue(name);
  if (NoIntrinsicType != IT)
    return bundle->getIntrinsicType(IT);
  Type* Ty = bundle->getOrCreateType(name);
  hlvmAssert(Ty != 0 && "Couldn't get Type!");
  return Ty;
}

bool
XMLReaderImpl::checkNewType(const std::string& name, Locator* loc)
{
  bool result = true;
  if (NoIntrinsicType != bundle->getIntrinsicTypesValue(name)) {
    error(loc,"Attempt to redefine intrinsic type '" + name + "'");
    result = false;
  }
  if (Type* Ty = bundle->getType(name))
    if (OpaqueType* OTy = llvm::dyn_cast<OpaqueType>(Ty))
      if (!OTy->isUnresolved()) {
        error(loc, "Attempt to redefine type '" + name + "'");
        result = false;
      }
  return result;
}

template<> Documentation*
XMLReaderImpl::parse<Documentation>(xmlNodePtr& cur)
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
    Documentation* doc = ast->new_Documentation(getLocator(cur));
    doc->setDoc(str);
    xmlBufferFree(buffer);
    cur = cur->next;
    return doc;
  }
  // Just signal that there's no documentation in this node
  return 0;
}

inline xmlNodePtr
XMLReaderImpl::checkDoc(xmlNodePtr cur, Documentable* node)
{
  xmlNodePtr child = cur->children;
  Documentation* theDoc = parse<Documentation>(child);
  if (theDoc)
    node->setDoc(theDoc);
  return child;
}

ConstantValue*
XMLReaderImpl::parseLiteralConstant(
    xmlNodePtr& cur, 
    const std::string& name,
    const Type* Ty)
{
  if (!name.empty() && bundle->getConst(name) != 0) {
    error(getLocator(cur),std::string("Constant '") + name 
          + "' already exists.");
    return 0;
  }

  // skip over blank text to find next element
  skipBlanks(cur);

  ConstantValue* C = 0;
  const char* prefix = 0;
  int token = getToken(cur->name);
  switch (token) {
    case TKN_false:   
    {
      C = ast->new_ConstantBoolean(name, bundle,Ty,false, getLocator(cur)); 
      break;
    }
    case TKN_true:
    {
      C = ast->new_ConstantBoolean(name,bundle,Ty,true, getLocator(cur));
      break;
    }
    case TKN_bool:
    {
      std::string buffer;
      xmlNodePtr child = cur->children;
      getTextContent(child,buffer);
      bool value = recognize_boolean( buffer.c_str() );
      C = ast->new_ConstantBoolean(name, bundle,Ty,value, getLocator(cur));
      break;
    }
    case TKN_char:
    {
      std::string buffer;
      xmlNodePtr child = cur->children;
      getTextContent(child,buffer);
      C = ast->new_ConstantCharacter(name, bundle,Ty,buffer, getLocator(cur));
      break;
    }
    case TKN_enum:
    {
      std::string value;
      xmlNodePtr child = cur->children;
      getTextContent(child,value);
      C = ast->new_ConstantEnumerator(name,bundle,Ty,value,getLocator(cur));
      break;
    }
    case TKN_bin:
    case TKN_oct:
    case TKN_dec:
    case TKN_hex:
    {
      std::string value;
      xmlNodePtr child = cur->children;
      getTextContent(child,value);
      uint16_t base = (token == TKN_dec ? 10 : (token == TKN_hex ? 16 : 
                      (token == TKN_oct ? 8 : (token == TKN_bin ? 2 : 10))));
      C = ast->new_ConstantInteger(name,bundle,Ty,value,base,getLocator(cur));
      break;
    }
    case TKN_flt:
    case TKN_dbl:
    case TKN_real:
    {
      std::string value;
      xmlNodePtr child = cur->children;
      getTextContent(child,value);
      C = ast->new_ConstantReal(name,bundle,Ty,value,getLocator(cur));
      break;
    }
    case TKN_str:
    {
      std::string value;
      xmlNodePtr child = cur->children;
      getTextContent(child,value);
      C =  ast->new_ConstantString(name,bundle,Ty,value,getLocator(cur));
      break;
    }
    case TKN_ptr:
    {
      std::string to = getAttribute(cur,"to");
      Constant* referent = bundle->getConst(to);
      if (!referent)
        error(loc,"Unkown referent for constant pointer");
      C = ast->new_ConstantPointer(name,bundle,Ty,referent,loc);
      break;
    }
    case TKN_arr:
    {
      const ArrayType* AT = llvm::cast<ArrayType>(Ty);
      const Type* ElemType = AT->getElementType();
      xmlNodePtr child = cur->children;
      std::vector<ConstantValue*> elems;
      while (child && skipBlanks(child) && child->type == XML_ELEMENT_NODE) {
        ConstantValue* elem = parseLiteralConstant(child,"",ElemType);
        elems.push_back(elem);
        child = child->next;
      }
      C = ast->new_ConstantArray(name,bundle,AT,elems,getLocator(cur));
      break;
    }
    case TKN_vect:
    {
      const VectorType* VT = llvm::cast<VectorType>(Ty);
      const Type* ElemType = VT->getElementType();
      xmlNodePtr child = cur->children;
      std::vector<ConstantValue*> elems;
      while (child && skipBlanks(child) && child->type == XML_ELEMENT_NODE) {
        ConstantValue* elem = parseLiteralConstant(child,"",ElemType);
        elems.push_back(elem);
        child = child->next;
      }
      C = ast->new_ConstantVector(name,bundle,VT,elems,getLocator(cur));
      break;
    }
    case TKN_struct:
    {
      const StructureType* ST = llvm::cast<StructureType>(Ty);
      xmlNodePtr child = cur->children;
      std::vector<ConstantValue*> fields;
      StructureType::const_iterator I = ST->begin();
      while (child && skipBlanks(child) && child->type == XML_ELEMENT_NODE) {
        ConstantValue* field = parseLiteralConstant(child,"",(*I)->getType());
        fields.push_back(field);
        child = child->next;
        ++I;
      }
      C = ast->new_ConstantStructure(name,bundle,ST,fields,getLocator(cur));
      break;
    }
    case TKN_cont:
    {
      const ContinuationType* CT = llvm::cast<ContinuationType>(Ty);
      xmlNodePtr child = cur->children;
      std::vector<ConstantValue*> fields;
      ContinuationType::const_iterator I = CT->begin();
      while (child && skipBlanks(child) && child->type == XML_ELEMENT_NODE) {
        ConstantValue* field = parseLiteralConstant(child,"",(*I)->getType());
        fields.push_back(field);
        child = child->next;
        ++I;
      }
      C = ast->new_ConstantContinuation(name,bundle,CT,fields,getLocator(cur));
      break;
    }
    default:
      hlvmAssert(!"Invalid kind of constant");
      break;
  }
  hlvmAssert(C && C->getType() == Ty && "Constant/Type mismatch");
  if (C)
    C->setParent(bundle);
  return C;
}

inline Constant*
XMLReaderImpl::parseConstant(xmlNodePtr& cur)
{
  hlvmAssert(getToken(cur->name) == TKN_constant);
  std::string name;
  std::string type;
  if (getNameType(cur,name,type)) {
    Type* Ty = getType(type);
    xmlNodePtr child = cur->children;
    Documentation* theDoc = parse<Documentation>(child);
    Constant* C = parseLiteralConstant(child,name,Ty);
    if (theDoc)
      C->setDoc(theDoc);
    return C;
  }
  return 0;
}

template<> AnyType*
XMLReaderImpl::parse<AnyType>(xmlNodePtr& cur)
{
  Locator* loc = getLocator(cur);
  std::string name = getAttribute(cur,"id");
  if (!checkNewType(name,loc))
    return 0;
  xmlNodePtr child = cur->children;
  Documentation* theDoc = parse<Documentation>(child);
  AnyType* result = ast->new_AnyType(name,bundle,loc);
  if (theDoc)
    result->setDoc(theDoc);
  return result;
}

template<> BooleanType*
XMLReaderImpl::parse<BooleanType>(xmlNodePtr& cur)
{
  Locator* loc = getLocator(cur);
  std::string name = getAttribute(cur,"id");
  if (!checkNewType(name,loc))
    return 0;
  xmlNodePtr child = cur->children;
  Documentation* theDoc = parse<Documentation>(child);
  BooleanType* result = ast->new_BooleanType(name,bundle,loc);
  if (theDoc)
    result->setDoc(theDoc);
  return result;
}

template<> BufferType*
XMLReaderImpl::parse<BufferType>(xmlNodePtr& cur)
{
  Locator* loc = getLocator(cur);
  std::string name = getAttribute(cur,"id");
  if (!checkNewType(name,loc))
    return 0;
  xmlNodePtr child = cur->children;
  Documentation* theDoc = parse<Documentation>(child);
  Type* result = ast->new_IntrinsicType(name,bundle,bufferTy,loc);
  if (theDoc)
    result->setDoc(theDoc);
  return llvm::cast<BufferType>(result);
}

template<> CharacterType*
XMLReaderImpl::parse<CharacterType>(xmlNodePtr& cur)
{
  Locator* loc = getLocator(cur);
  std::string name = getAttribute(cur,"id");
  if (!checkNewType(name,loc))
    return 0;
  const char* encoding = getAttribute(cur,"encoding");
  xmlNodePtr child = cur->children;
  Documentation* theDoc = parse<Documentation>(child);
  std::string enc;
  if (encoding)
    enc = encoding;
  else
    enc = "utf-8";
  CharacterType* result = 
    ast->new_CharacterType(name,bundle,encoding,loc);
  if (theDoc)
    result->setDoc(theDoc);
  return result;
}

Type*
XMLReaderImpl::parseInteger(xmlNodePtr& cur, bool isSigned)
{
  Locator* loc = getLocator(cur);
  std::string name = getAttribute(cur,"id");
  if (!checkNewType(name,loc))
    return 0;
  const char* bits = getAttribute(cur,"bits");
  xmlNodePtr child = cur->children;
  Documentation* theDoc = parse<Documentation>(child);
  uint64_t numBits = 0;
  if (bits)
    numBits = recognize_nonNegativeInteger(bits);
  else
    numBits = 32;
  IntegerType* result = 
    ast->new_IntegerType(name,bundle,numBits,isSigned,loc);
  if (theDoc)
    result->setDoc(theDoc);
  return result;
}

template<> RangeType*
XMLReaderImpl::parse<RangeType>(xmlNodePtr& cur)
{
  Locator* loc = getLocator(cur);
  std::string name = getAttribute(cur,"id");
  if (!checkNewType(name,loc))
    return 0;
  const char* min = getAttribute(cur, "min");
  const char* max = getAttribute(cur, "max");
  xmlNodePtr child = cur->children;
  Documentation* theDoc = parse<Documentation>(child);
  if (min && max) {
    int64_t minVal = recognize_Integer(min);
    int64_t maxVal = recognize_Integer(max);
    RangeType* result = ast->new_RangeType(name,bundle,minVal,maxVal,loc);
    if (theDoc)
      result->setDoc(theDoc);
    return result;
  }
  error(loc,"Invalid min/max specification");
  return 0;
}

template<> RealType*
XMLReaderImpl::parse<RealType>(xmlNodePtr& cur)
{
  Locator* loc = getLocator(cur);
  std::string name = getAttribute(cur,"id");
  if (!checkNewType(name,loc))
    return 0;
  const char* mantissa = getAttribute(cur, "mantissa");
  const char* exponent = getAttribute(cur, "exponent");
  xmlNodePtr child = cur->children;
  Documentation* theDoc = parse<Documentation>(child);
  if (mantissa && exponent) {
    int32_t mantVal = recognize_nonNegativeInteger(mantissa);
    int32_t expoVal = recognize_nonNegativeInteger(exponent);
    RealType* result = ast->new_RealType(name,bundle,mantVal,expoVal,loc);
    if (theDoc)
      result->setDoc(theDoc);
    return result;
  }
  error(loc,"Invalid mantissa/exponent specification");
  return 0;
}

template<> EnumerationType*
XMLReaderImpl::parse<EnumerationType>(xmlNodePtr& cur)
{
  hlvmAssert(getToken(cur->name)==TKN_enumeration);
  Locator* loc = getLocator(cur);
  std::string name = getAttribute(cur,"id");
  if (!checkNewType(name,loc))
    return 0;
  EnumerationType* en = ast->new_EnumerationType(name,bundle,loc);
  xmlNodePtr child = checkDoc(cur,en);
  while (child && skipBlanks(child) && child->type == XML_ELEMENT_NODE) {
    hlvmAssert(getToken(child->name) == TKN_enumerator);
    std::string id = getAttribute(child,"id");
    en->addEnumerator(id);
    child = child->next;
  }
  return en;
}

template<> PointerType*     
XMLReaderImpl::parse<PointerType>(xmlNodePtr& cur)
{
  hlvmAssert(getToken(cur->name)==TKN_pointer);
  Locator* loc = getLocator(cur);
  std::string name = getAttribute(cur,"id");
  if (!checkNewType(name,loc))
    return 0;
  std::string type = getAttribute(cur,"to");
  PointerType* result = 
    ast->new_PointerType(name,bundle,getType(type),loc);
  checkDoc(cur,result);
  return result;
}

template<> ArrayType*     
XMLReaderImpl::parse<ArrayType>(xmlNodePtr& cur)
{
  hlvmAssert(getToken(cur->name)==TKN_array);
  Locator* loc = getLocator(cur);
  std::string name = getAttribute(cur,"id");
  if (!checkNewType(name,loc))
    return 0;
  std::string type = getAttribute(cur,"of");
  const char* len = getAttribute(cur,"length");
  ArrayType* result = ast->new_ArrayType(
    name, bundle, getType(type), recognize_nonNegativeInteger(len),loc);
  checkDoc(cur,result);
  return result;
}

template<> VectorType*     
XMLReaderImpl::parse<VectorType>(xmlNodePtr& cur)
{
  hlvmAssert(getToken(cur->name)==TKN_vector);
  Locator* loc = getLocator(cur);
  std::string name = getAttribute(cur,"id");
  if (!checkNewType(name,loc))
    return 0;
  std::string type = getAttribute(cur,"of");
  const char* len  = getAttribute(cur,"length");
  VectorType* result = ast->new_VectorType(
      name, bundle, getType(type), recognize_nonNegativeInteger(len),loc);
  checkDoc(cur,result);
  return result;
}

template<> StreamType*
XMLReaderImpl::parse<StreamType>(xmlNodePtr& cur)
{
  Locator* loc = getLocator(cur);
  std::string name = getAttribute(cur,"id");
  if (!checkNewType(name,loc))
    return 0;
  xmlNodePtr child = cur->children;
  Documentation* theDoc = parse<Documentation>(child);
  Type* result = ast->new_IntrinsicType(name,bundle,streamTy,loc);
  if (theDoc)
    result->setDoc(theDoc);
  return llvm::cast<StreamType>(result);
}

template<> StringType*
XMLReaderImpl::parse<StringType>(xmlNodePtr& cur)
{
  Locator* loc = getLocator(cur);
  std::string name = getAttribute(cur,"id");
  if (!checkNewType(name,loc))
    return 0;
  const char* encoding = getAttribute(cur,"encoding");
  xmlNodePtr child = cur->children;
  Documentation* theDoc = parse<Documentation>(child);
  std::string enc;
  if (encoding)
    enc = encoding;
  else
    enc = "utf-8";
  StringType* result = 
    ast->new_StringType(name,bundle,encoding,loc);
  if (theDoc)
    result->setDoc(theDoc);
  return result;
}

template<> StructureType*
XMLReaderImpl::parse<StructureType>(xmlNodePtr& cur)
{
  hlvmAssert(getToken(cur->name)==TKN_structure);
  Locator* loc = getLocator(cur);
  std::string name = getAttribute(cur,"id");
  if (!checkNewType(name,loc))
    return 0;
  StructureType* struc = ast->new_StructureType(name, bundle, loc);
  xmlNodePtr child = checkDoc(cur,struc); 
  while (child && skipBlanks(child) && child->type == XML_ELEMENT_NODE) {
    hlvmAssert(getToken(child->name) == TKN_field && 
               "Structure only has fields");
    std::string name = getAttribute(child,"id");
    std::string type = getAttribute(child,"type");
    Locator* fldLoc = getLocator(child);
    Field* fld = ast->new_Field(name, getType(type),fldLoc);
    struc->addField(fld);
    checkDoc(child,fld);
    child = child->next;
  }
  return struc;
}

template<> ContinuationType*
XMLReaderImpl::parse<ContinuationType>(xmlNodePtr& cur)
{
  hlvmAssert(getToken(cur->name)==TKN_structure);
  Locator* loc = getLocator(cur);
  std::string name = getAttribute(cur,"id");
  if (!checkNewType(name,loc))
    return 0;
  ContinuationType* cont = ast->new_ContinuationType(name, bundle, loc);
  xmlNodePtr child = checkDoc(cur,cont); 
  while (child && skipBlanks(child) && child->type == XML_ELEMENT_NODE) {
    hlvmAssert(getToken(child->name) == TKN_field && 
               "Continuation only has fields");
    std::string name = getAttribute(child,"id");
    std::string type = getAttribute(child,"type");
    Locator* fldLoc = getLocator(child);
    Field* fld = ast->new_Field(name,getType(type),fldLoc);
    cont->addField(fld);
    checkDoc(child,fld);
    child = child->next;
  }
  return cont;
}

template<> SignatureType*
XMLReaderImpl::parse<SignatureType>(xmlNodePtr& cur)
{
  hlvmAssert(getToken(cur->name)==TKN_signature);
  Locator* loc = getLocator(cur);
  std::string name = getAttribute(cur,"id");
  if (!checkNewType(name,loc))
    return 0;
  std::string result = getAttribute(cur,"result");
  const char* varargs = getAttribute(cur,"varargs",false);
  SignatureType* sig = 
    ast->new_SignatureType(name, bundle, getType(result),loc);
  if (varargs)
    sig->setIsVarArgs(recognize_boolean(varargs));
  xmlNodePtr child = checkDoc(cur,sig); 
  while (child && skipBlanks(child) && child->type == XML_ELEMENT_NODE) {
    hlvmAssert(getToken(child->name) == TKN_arg && "Signature only has args");
    std::string name = getAttribute(child,"id");
    std::string type = getAttribute(child,"type");
    Locator* paramLoc = getLocator(child);
    Parameter* param = ast->new_Parameter(name,getType(type),paramLoc);
    sig->addParameter(param);
    checkDoc(child,param);
    child = child->next;
  }
  return sig;
}

template<> TextType*
XMLReaderImpl::parse<TextType>(xmlNodePtr& cur)
{
  Locator* loc = getLocator(cur);
  std::string name = getAttribute(cur,"id");
  if (!checkNewType(name,loc))
    return 0;
  xmlNodePtr child = cur->children;
  Documentation* theDoc = parse<Documentation>(child);
  Type* result = ast->new_IntrinsicType(name,bundle,textTy,loc);
  if (theDoc)
    result->setDoc(theDoc);
  return llvm::cast<TextType>(result);
}

template<> OpaqueType*
XMLReaderImpl::parse<OpaqueType>(xmlNodePtr& cur)
{
  hlvmAssert(getToken(cur->name)==TKN_opaque);
  std::string name = getAttribute(cur,"id");
  if (!checkNewType(name,loc))
    return 0;
  Locator* loc = getLocator(cur);
  OpaqueType* result = ast->new_OpaqueType(name, false, bundle, loc);
  checkDoc(cur,result);
  return result;
}

template<> Variable*
XMLReaderImpl::parse<Variable>(xmlNodePtr& cur)
{
  hlvmAssert(getToken(cur->name)==TKN_variable);
  Locator* loc = getLocator(cur);
  std::string name, type;
  getNameType(cur, name, type);
  if (Constant* lkbl = bundle->getConst(name)) 
    error(loc, "Name '" + name + "' is already in use");
  const char* cnst = getAttribute(cur, "const", false);
  const char* lnkg = getAttribute(cur, "linkage", false);
  const char* init = getAttribute(cur, "init", false);
  const Type* Ty = getType(type);
  Variable* var = ast->new_Variable(name, bundle, Ty,loc);
  if (cnst)
    var->setIsConstant(recognize_boolean(cnst));
  if (lnkg)
    var->setLinkageKind(recognize_LinkageKinds(lnkg));
  if (init) {
    Constant* initializer = bundle->getConst(init);
    if (initializer)
      var->setInitializer(initializer);
    else 
      error(loc,std::string("Constant '") + init + 
            "' not found in initializer."); 
  }
  checkDoc(cur,var);
  return var;
}

template<> AutoVarOp*
XMLReaderImpl::parse<AutoVarOp>(xmlNodePtr& cur)
{
  Locator* loc = getLocator(cur);
  std::string name, type;
  getNameType(cur, name, type);
  const Type* Ty = getType(type);
  const char* init = getAttribute(cur,"init",false);
  Constant*initializer = 0;
  if (init) {
    initializer = bundle->getConst(init);
    if (!initializer)
      error(loc,std::string("Initializer '") + init + "' not found .");
  }
  AutoVarOp* autovar = ast->AST::new_AutoVarOp(name,Ty,initializer,loc);
  checkDoc(cur,autovar);
  return autovar;
}

template<> ReferenceOp*
XMLReaderImpl::parse<ReferenceOp>(xmlNodePtr& cur)
{
  std::string id = getAttribute(cur,"id");
  Locator* loc = getLocator(cur);

  // Find the referrent variable in a block
  Value* referent = 0;
  for (BlockStack::reverse_iterator I = blocks.rbegin(), E = blocks.rend(); 
       I != E; ++I )
  {
    Block* blk = *I;
    if (AutoVarOp* av = blk->getAutoVar(id))
      if (av->getName() == id) {
        referent = av;
        break;
      }
  }

  // Didn't find an autovar? Try a function argument
  if (!referent)
    referent = func->getArgument(id);

  // Didn't find an autovar? Try a constant value.
  if (!referent)
    referent= bundle->getConst(id);
    
  // Didn't find a linkable? Try an error message for size
  if (!referent)
      error(loc,std::string("Referent '") + id + "' not found");

  ReferenceOp* refop = ast->AST::new_ReferenceOp(referent, loc);
  checkDoc(cur,refop);
  return refop;
}

template<class OpClass>
OpClass*
XMLReaderImpl::parseNilaryOp(xmlNodePtr& cur)
{
  Locator* loc = getLocator(cur);
  xmlNodePtr child = cur->children;
  Documentation* doc = parse<Documentation>(child); 
  OpClass* result = ast->AST::new_NilaryOp<OpClass>(bundle,loc);
  if (doc)
    result->setDoc(doc);
  return result;
}

template<class OpClass>
OpClass*
XMLReaderImpl::parseUnaryOp(xmlNodePtr& cur)
{
  Locator* loc = getLocator(cur);
  xmlNodePtr child = cur->children;
  Documentation* doc = parse<Documentation>(child); 
  if (child && skipBlanks(child)) {
    Operator* oprnd1 = parseOperator(child);
    OpClass* result = ast->AST::new_UnaryOp<OpClass>(oprnd1,bundle,loc);
    if (doc)
      result->setDoc(doc);
    return result;
  } else
    error(loc,std::string("Operator '") + 
      reinterpret_cast<const char*>(cur->name) + "' requires one operand.");
  return 0;
}

template<class OpClass>
OpClass*
XMLReaderImpl::parseBinaryOp(xmlNodePtr& cur)
{
  Locator* loc = getLocator(cur);
  xmlNodePtr child = cur->children;
  Documentation* doc = parse<Documentation>(child); 
  if (child && skipBlanks(child)) {
    Operator* oprnd1 = parseOperator(child);
    child = child->next;
    if (child && skipBlanks(child)) {
      Operator* oprnd2 = parseOperator(child);
      OpClass* result =  
        ast->AST::new_BinaryOp<OpClass>(oprnd1,oprnd2,bundle,loc);
      if (doc)
        result->setDoc(doc);
      return result;
    } else {
      error(loc,std::string("Operator '") + 
            reinterpret_cast<const char*>(cur->name) + 
            "' needs a second operand.");
    }
  } else {
    error(loc,std::string("Operator '") + 
          reinterpret_cast<const char*>(cur->name) + "' requires 2 operands.");
  }
  return 0;
}

template<class OpClass>
OpClass*
XMLReaderImpl::parseTernaryOp(xmlNodePtr& cur)
{
  Locator* loc = getLocator(cur);
  xmlNodePtr child = cur->children;
  Documentation* doc = parse<Documentation>(child); 
  if (child && skipBlanks(child)) {
    Operator* oprnd1 = parseOperator(child);
    child = child->next;
    if (child && skipBlanks(child)) {
      Operator* oprnd2 = parseOperator(child);
      child = child->next;
      if (child && skipBlanks(child)) {
        Operator* oprnd3 = parseOperator(child);
        OpClass* result =  
          ast->AST::new_TernaryOp<OpClass>(oprnd1,oprnd2,oprnd3,bundle,loc);
        if (doc)
          result->setDoc(doc);
        return result;
      } else
        error(loc,std::string("Operator '") + 
              reinterpret_cast<const char*>(cur->name) +
              "' needs a third operand.");
    } else
      error(loc,std::string("Operator '") + 
            reinterpret_cast<const char*>(cur->name) + 
            "' needs a second operand.");
  } else
    error(loc,std::string("Operator '") + 
          reinterpret_cast<const char*>(cur->name) + "' requires 3 operands.");
  return 0;
}

template<class OpClass>
OpClass*
XMLReaderImpl::parseMultiOp(xmlNodePtr& cur)
{
  Locator* loc = getLocator(cur);
  xmlNodePtr child = cur->children;
  Documentation* doc = parse<Documentation>(child); 
  MultiOperator::OprndList ol;
  while (child != 0 && skipBlanks(child)) {
    Operator* operand = parseOperator(child);
    if (operand)
      ol.push_back(operand);
    else
      break;
    child = child->next;
  }
  OpClass* result = ast->AST::new_MultiOp<OpClass>(ol,bundle,loc);
  if (doc)
    result->setDoc(doc);
  return result;
}

template<> Block*
XMLReaderImpl::parse<Block>(xmlNodePtr& cur)
{
  hlvmAssert(getToken(cur->name) == TKN_block && "Expecting block element");
  hlvmAssert(func != 0);
  Locator* loc = getLocator(cur);
  const char* label = getAttribute(cur, "label",false);
  Block* result = ast->new_Block(loc);
  xmlNodePtr child = checkDoc(cur,result);
  MultiOperator::OprndList ops;
  block = result;
  if (label)
    block->setLabel(label);
  blocks.push_back(block);
  while (child && skipBlanks(child) && child->type == XML_ELEMENT_NODE) 
  {
    Operator* op = parseOperator(child);
    block->addOperand(op);
    child = child->next;
  }
  blocks.pop_back();
  if (blocks.empty())
    block = 0;
  else
    block = blocks.back();
  return result;
}

template<> Function*
XMLReaderImpl::parse<Function>(xmlNodePtr& cur)
{
  hlvmAssert(getToken(cur->name)==TKN_function);
  Locator* loc = getLocator(cur);
  std::string name, type;
  getNameType(cur, name, type);
  Constant* lkbl = bundle->getConst(name);
  if (lkbl) {
    if (llvm::isa<Function>(lkbl)) {
      func = llvm::cast<Function>(lkbl);
      if (func->hasBlock()) {
        error(loc,std::string("Function '") + name + "' was already defined.");
        return func;
      }
    } else {
      error(loc,std::string("Name '") + name + "' was already used.");
      return 0;
    }
  } else {
    const Type* Ty = getType(type);
    if (llvm::isa<SignatureType>(Ty)) {
      func = ast->new_Function(name,bundle,llvm::cast<SignatureType>(Ty),loc);
      const char* lnkg = getAttribute(cur, "linkage", false);
      if (lnkg)
        func->setLinkageKind(recognize_LinkageKinds(lnkg));
    } else {
      error(loc,"Invalid type for a function, must be signature");
    }
  }
  xmlNodePtr child = checkDoc(cur,func);
  if (child && skipBlanks(child) && child->type == XML_ELEMENT_NODE) {
    Block* b = parse<Block>(child);
    b->setParent(func);
  }
  return func;
}

template<> Program*
XMLReaderImpl::parse<Program>(xmlNodePtr& cur)
{
  hlvmAssert(getToken(cur->name) == TKN_program && "Expecting program element");
  Locator* loc = getLocator(cur);
  std::string name(getAttribute(cur, "id"));
  Program* program = ast->new_Program(name,bundle,loc);
  func = program;
  xmlNodePtr child = checkDoc(cur,func);
  if (child && skipBlanks(child) && child->type == XML_ELEMENT_NODE) {
    Block* b = parse<Block>(child);
    b->setParent(func);
  } else {
    hlvmDeadCode("Program Without Block!");
  }
  return program;
}

template<> Import*
XMLReaderImpl::parse<Import>(xmlNodePtr& cur)
{
  hlvmAssert(getToken(cur->name)==TKN_import);
  std::string pfx = getAttribute(cur,"prefix");
  Import* imp = ast->new_Import(pfx,getLocator(cur));
  checkDoc(cur,imp);
  return imp;
}

template<> Bundle*
XMLReaderImpl::parse<Bundle>(xmlNodePtr& cur) 
{
  hlvmAssert(getToken(cur->name) == TKN_bundle && "Expecting bundle element");
  std::string pubid(getAttribute(cur, "id"));
  Locator* loc = getLocator(cur);
  bundle = ast->new_Bundle(pubid,loc);
  xmlNodePtr child = checkDoc(cur,bundle);
  while (child && skipBlanks(child) && child->type == XML_ELEMENT_NODE) 
  {
    int tkn = getToken(child->name);
    Node* n = 0;
    switch (tkn) {
      case TKN_array       : { n = parse<ArrayType>(child); break; }
      case TKN_any         : { n = parse<AnyType>(child); break; }
      case TKN_boolean     : { n = parse<BooleanType>(child); break; }
      case TKN_buffer      : { n = parse<BufferType>(child); break; }
      case TKN_character   : { n = parse<CharacterType>(child); break; }
      case TKN_constant    : { n = parseConstant(child); break; }
      case TKN_enumeration : { n = parse<EnumerationType>(child); break; }
      case TKN_function    : { n = parse<Function>(child); break; }
      case TKN_import      : { n = parse<Import>(child); break; }
      case TKN_opaque      : { n = parse<OpaqueType>(child); break; }
      case TKN_pointer     : { n = parse<PointerType>(child); break; }
      case TKN_program     : { n = parse<Program>(child); break; }
      case TKN_range       : { n = parse<RangeType>(child); break; }
      case TKN_real        : { n = parse<RealType>(child); break; }
      case TKN_signature   : { n = parse<SignatureType>(child); break; }
      case TKN_signed      : { n = parseInteger(child,true); break; }
      case TKN_stream      : { n = parse<StreamType>(child); break; }
      case TKN_string      : { n = parse<StringType>(child); break; }
      case TKN_structure   : { n = parse<StructureType>(child); break; }
      case TKN_text        : { n = parse<TextType>(child); break; }
      case TKN_unsigned    : { n = parseInteger(child,false); break; }
      case TKN_variable    : { n = parse<Variable>(child); break; }
      case TKN_vector      : { n = parse<VectorType>(child); break; }
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

Operator*
XMLReaderImpl::parseOperator(xmlNodePtr& cur)
{
  if (cur && skipBlanks(cur) && cur->type == XML_ELEMENT_NODE) {
    Operator* op = 0;
    switch (getToken(cur->name)) {
      case TKN_neg:          op = parseUnaryOp<NegateOp>(cur); break;
      case TKN_cmpl:         op = parseUnaryOp<ComplementOp>(cur); break;
      case TKN_preinc:       op = parseUnaryOp<PreIncrOp>(cur); break;
      case TKN_predec:       op = parseUnaryOp<PreDecrOp>(cur); break;
      case TKN_postinc:      op = parseUnaryOp<PostIncrOp>(cur); break;
      case TKN_postdec:      op = parseUnaryOp<PostDecrOp>(cur); break;
      case TKN_add:          op = parseBinaryOp<AddOp>(cur); break;
      case TKN_sub:          op = parseBinaryOp<SubtractOp>(cur); break;
      case TKN_mul:          op = parseBinaryOp<MultiplyOp>(cur); break;
      case TKN_div:          op = parseBinaryOp<DivideOp>(cur); break;
      case TKN_mod:          op = parseBinaryOp<ModuloOp>(cur); break;
      case TKN_band:         op = parseBinaryOp<BAndOp>(cur); break;
      case TKN_bor:          op = parseBinaryOp<BOrOp>(cur); break;
      case TKN_bxor:         op = parseBinaryOp<BXorOp>(cur); break;
      case TKN_bnor:         op = parseBinaryOp<BNorOp>(cur); break;
      case TKN_not:          op = parseUnaryOp<NotOp>(cur); break;
      case TKN_and:          op = parseBinaryOp<AndOp>(cur); break;
      case TKN_or:           op = parseBinaryOp<OrOp>(cur); break;
      case TKN_nor:          op = parseBinaryOp<NorOp>(cur); break;
      case TKN_xor:          op = parseBinaryOp<XorOp>(cur); break;
      case TKN_eq:           op = parseBinaryOp<EqualityOp>(cur); break;
      case TKN_ne:           op = parseBinaryOp<InequalityOp>(cur); break;
      case TKN_lt:           op = parseBinaryOp<LessThanOp>(cur); break;
      case TKN_gt:           op = parseBinaryOp<GreaterThanOp>(cur); break;
      case TKN_ge:           op = parseBinaryOp<GreaterEqualOp>(cur); break;
      case TKN_le:           op = parseBinaryOp<LessEqualOp>(cur); break;
      case TKN_select:       op = parseTernaryOp<SelectOp>(cur); break;
      case TKN_switch:       op = parseMultiOp<SwitchOp>(cur); break;
      case TKN_while:        op = parseBinaryOp<WhileOp>(cur); break;
      case TKN_unless:       op = parseBinaryOp<UnlessOp>(cur); break;
      case TKN_until:        op = parseBinaryOp<UntilOp>(cur); break;
      case TKN_loop:         op = parseTernaryOp<LoopOp>(cur); break;
      case TKN_break:        op = parseNilaryOp<BreakOp>(cur); break;
      case TKN_continue:     op = parseNilaryOp<ContinueOp>(cur); break;
      case TKN_ret:          op = parseNilaryOp<ReturnOp>(cur); break;
      case TKN_result:       op = parseUnaryOp<ResultOp>(cur); break;
      case TKN_call:         op = parseMultiOp<CallOp>(cur); break;
      case TKN_store:        op = parseBinaryOp<StoreOp>(cur); break;
      case TKN_load:         op = parseUnaryOp<LoadOp>(cur); break;
      case TKN_open:         op = parseUnaryOp<OpenOp>(cur); break;
      case TKN_write:        op = parseBinaryOp<WriteOp>(cur); break;
      case TKN_close:        op = parseUnaryOp<CloseOp>(cur); break;
      case TKN_ref:          op = parse<ReferenceOp>(cur); break;
      case TKN_autovar:      op = parse<AutoVarOp>(cur); break;
      case TKN_block:        op = parse<Block>(cur); break;
      default:
        hlvmDeadCode("Unrecognized operator");
        break;
    }
    return op;
  } else if (cur != 0)
    hlvmDeadCode("Expecting a value");
  return 0;
}

void
XMLReaderImpl::parseTree() 
{
  xmlNodePtr cur = xmlDocGetRootElement(doc);
  if (!cur) {
    error(0,"No root node");
    return;
  }
  hlvmAssert(getToken(cur->name) == TKN_hlvm && "Expecting hlvm element");
  const std::string pubid = getAttribute(cur,"pubid");
  ast->setPublicID(pubid);
  cur = cur->children;
  if (skipBlanks(cur)) {
    Bundle* b = parse<Bundle>(cur);
  }
}

// Implement the read interface to parse, validate, and convert the
// XML document into AST Nodes. 
bool
XMLReaderImpl::read() {

  // create the RelaxNG Parser Context
  xmlRelaxNGParserCtxtPtr rngparser =
    xmlRelaxNGNewMemParserCtxt(HLVMGrammar, sizeof(HLVMGrammar));
  if (!rngparser) {
    error(0,"Failed to allocate RNG Parser Context");
    return false;
  }

  // Provide the error handler for parsing the schema
  xmlRelaxNGSetParserStructuredErrors(rngparser, ParseHandler, this);

  // Parse the schema and build an internal structure for it
  xmlRelaxNGPtr schema = xmlRelaxNGParse(rngparser);
  if (!schema) {
    error(0,"Failed to parse the RNG Schema");
    xmlRelaxNGFreeParserCtxt(rngparser);
    return false;
  }

  // create a document parser context
  xmlParserCtxtPtr ctxt = xmlNewParserCtxt();
  if (!ctxt) {
    error(0,"Failed to allocate document parser context");
    xmlRelaxNGFreeParserCtxt(rngparser);
    xmlRelaxNGFree(schema);
    return false;
  }

  // Parse the file, creating a Document tree
  doc = xmlCtxtReadFile(ctxt, path.c_str(), 0, 0);
  if (!doc) {
    error(0,"Failed to parse the document");
    xmlRelaxNGFreeParserCtxt(rngparser);
    xmlRelaxNGFree(schema);
    xmlFreeParserCtxt(ctxt);
    return false;
  }

  // Create a validation context
  xmlRelaxNGValidCtxtPtr validation = xmlRelaxNGNewValidCtxt(schema);
  if (!validation) {
    error(0,"Failed to create the validation context");
    xmlRelaxNGFreeParserCtxt(rngparser);
    xmlRelaxNGFree(schema);
    xmlFreeParserCtxt(ctxt);
    xmlFreeDoc(doc);
    doc = 0;
    return false;
  }

  // Provide the error handler for parsing the schema
  xmlRelaxNGSetValidStructuredErrors(validation, ValidationHandler, this);

  // Validate the document with the schema
  if (xmlRelaxNGValidateDoc(validation, doc)) {
    error(0,"Document didn't pass RNG schema validation");
    xmlRelaxNGFreeParserCtxt(rngparser);
    xmlRelaxNGFree(schema);
    xmlFreeParserCtxt(ctxt);
    xmlFreeDoc(doc);
    doc = 0;
    xmlRelaxNGFreeValidCtxt(validation);
    return false;
  }

  // Parse
  parseTree();
  xmlRelaxNGFreeParserCtxt(rngparser);
  xmlRelaxNGFree(schema);
  xmlFreeParserCtxt(ctxt);
  xmlRelaxNGFreeValidCtxt(validation);
  xmlFreeDoc(doc);
  doc = 0;
  return true;
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

//===-- AST XML Writer Implementation -----------------------*- C++ -*-===//
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
/// @file hlvm/Writer/XMLWriter.cpp
/// @author Reid Spencer <rspencer@reidspencer.com>
/// @date 2006/05/12
/// @since 0.1.0
/// @brief Provides the interface to hlvm::XMLWriter
//===----------------------------------------------------------------------===//

#include <hlvm/Writer/XMLWriter.h>
#include <hlvm/AST/AST.h>
#include <hlvm/AST/Bundle.h>
#include <hlvm/AST/Documentation.h>
#include <hlvm/AST/ContainerType.h>
#include <hlvm/AST/Linkables.h>
#include <hlvm/AST/Constants.h>
#include <hlvm/AST/Block.h>
#include <hlvm/AST/ControlFlow.h>
#include <hlvm/AST/MemoryOps.h>
#include <hlvm/AST/InputOutput.h>
#include <hlvm/AST/Arithmetic.h>
#include <hlvm/AST/BooleanOps.h>
#include <hlvm/AST/RealMath.h>
#include <hlvm/Base/Assert.h>
#include <hlvm/Pass/Pass.h>
#include <llvm/ADT/StringExtras.h>
#include <libxml/xmlwriter.h>
#include <iostream>

using namespace hlvm;
using namespace llvm;

namespace {

class XMLWriterImpl : public XMLWriter {
public:
  XMLWriterImpl(const char* fname) : pass(fname) { }
  virtual ~XMLWriterImpl() { }
  virtual void write(AST* node);

private:
  class WriterPass : public Pass
  {
    public:
      WriterPass(const char* fname) : Pass(0,Pass::PreAndPostOrderTraversal) {
        writer = xmlNewTextWriterFilename(fname,0);
        hlvmAssert(writer && "Can't allocate writer");
        xmlTextWriterSetIndent(writer,1);
        xmlTextWriterSetIndentString(writer,
            reinterpret_cast<const xmlChar*>("  "));
      }
      ~WriterPass() {
        xmlFreeTextWriter(writer);
      }
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
    inline void writeAttribute(const char* name, const Type* t)
      { writeAttribute(name, t->getName()); }
    inline void writeAttribute(const char* name, uint64_t val)
      { writeAttribute(name, llvm::utostr(val)); }
    inline void writeAttribute(const char* name, int64_t val)
      { writeAttribute(name, llvm::itostr(val)); }
    inline void writeElement(const char* elem, const char* body)
      { xmlTextWriterWriteElement(writer,
          reinterpret_cast<const xmlChar*>(elem),
          reinterpret_cast<const xmlChar*>(body)); }
    inline void writeString(const std::string& str) ;

    inline void putHeader();
    inline void putFooter();
    inline void putDoc(const Documentable* node);

    void putConstantValue(const ConstantValue* CV,bool nested);

    template<class NodeClass>
    inline void put(const NodeClass* nc);

    template<class NodeClass>
    inline void put(const NodeClass* nc, bool nested);

    virtual void handle(Node* n,Pass::TraversalKinds mode);

  private:
    xmlTextWriterPtr writer;
    AST* node;
    friend class XMLWriterImpl;
  };
private:
  WriterPass pass;
};

inline const char* 
getLinkageKind(LinkageKinds lk)
{
  switch (lk) {
    case WeakLinkage:        return "weak";
    case ExternalLinkage:    return "external";
    case InternalLinkage:    return "internal";
    case LinkOnceLinkage:    return "linkonce";
    case AppendingLinkage:   return "appending";
    default:
      hlvmDeadCode("Bad LinkageKinds");
      break;
  }
  return "error";
}

void 
XMLWriterImpl::WriterPass::writeString(const std::string& str)
{ 
  const xmlChar* str_to_write = // xmlEncodeSpecialChars(
      reinterpret_cast<const xmlChar*>(str.c_str()); // ); 
  xmlTextWriterWriteString(writer, str_to_write);
  // xmlMemFree(str_to_write);
}

void
XMLWriterImpl::WriterPass::putHeader() 
{
  xmlTextWriterStartDocument(writer,0,"UTF-8",0);
  startElement("hlvm");
  writeAttribute("xmlns","http://hlvm.org/src/hlvm/Reader/XML/HLVM.rng");
  writeAttribute("pubid",node->getPublicID());
}

void
XMLWriterImpl::WriterPass::putFooter()
{
  endElement();
  xmlTextWriterEndDocument(writer);
}

template<> void 
XMLWriterImpl::WriterPass::put(const Documentation* b)
{
  startElement("doc");
  const std::string& data = b->getDoc();
  xmlTextWriterWriteRawLen(writer,
    reinterpret_cast<const xmlChar*>(data.c_str()),data.length());
  endElement();
}

inline void
XMLWriterImpl::WriterPass::putDoc(const Documentable* node)
{
  Documentation* theDoc = node->getDoc();
  if (theDoc) {
    this->put<Documentation>(theDoc);
  }
}

template<> void 
XMLWriterImpl::WriterPass::put(const NamedType* t)
{
  startElement("alias");
  writeAttribute("id",t->getName());
  writeAttribute("is",t->getType()->getName());
  putDoc(t);
  endElement();
}

template<> void 
XMLWriterImpl::WriterPass::put(const AnyType* t)
{
  if (t->isIntrinsic())
    return;
  startElement("any");
  writeAttribute("id",t->getName());
  putDoc(t);
  endElement();
}

template<> void 
XMLWriterImpl::WriterPass::put(const StringType* t)
{
  if (t->isIntrinsic())
    return;
  startElement("string");
  writeAttribute("id",t->getName());
  writeAttribute("encoding",t->getEncoding());
  putDoc(t);
  endElement();
}

template<>void
XMLWriterImpl::WriterPass::put(const BooleanType* t)
{
  if (t->isIntrinsic())
    return;
  startElement("boolean");
  writeAttribute("id",t->getName());
  putDoc(t);
  endElement();
}

template<> void
XMLWriterImpl::WriterPass::put(const CharacterType* t)
{
  if (t->isIntrinsic())
    return;
  startElement("character");
  writeAttribute("id",t->getName());
  writeAttribute("encoding",t->getEncoding());
  putDoc(t);
  endElement();
}

template<> void
XMLWriterImpl::WriterPass::put(const IntegerType* t)
{
  if (t->isIntrinsic())
    return;
  if (t->isSigned()) {
    startElement("signed");
    writeAttribute("id",t->getName());
    writeAttribute("bits", llvm::utostr(t->getBits()));
  } else {
    startElement("unsigned");
    writeAttribute("id",t->getName());
    writeAttribute("bits", llvm::utostr(t->getBits()));
  }
  putDoc(t);
  endElement();
}

template<> void
XMLWriterImpl::WriterPass::put(const RangeType* t)
{
  if (t->isIntrinsic())
    return;
  startElement("range");
  writeAttribute("id",t->getName());
  writeAttribute("min",t->getMin());
  writeAttribute("max",t->getMax());
  putDoc(t);
  endElement();
}

template<> void 
XMLWriterImpl::WriterPass::put(const EnumerationType* t)
{
  if (t->isIntrinsic())
    return;
  startElement("enumeration");
  writeAttribute("id",t->getName());
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

template<> void
XMLWriterImpl::WriterPass::put(const RealType* t)
{
  if (t->isIntrinsic())
    return;
  startElement("real");
  writeAttribute("id",t->getName());
  writeAttribute("mantissa", llvm::utostr(t->getMantissa()));
  writeAttribute("exponent", llvm::utostr(t->getExponent()));
  putDoc(t);
  endElement();
}

template<> void
XMLWriterImpl::WriterPass::put(const OpaqueType* op)
{
  if (op->isIntrinsic())
    return;
  startElement("opaque");
  writeAttribute("id",op->getName());
  putDoc(op);
  endElement();
}

template<> void 
XMLWriterImpl::WriterPass::put(const PointerType* t)
{
  if (t->isIntrinsic())
    return;
  startElement("pointer");
  writeAttribute("id", t->getName());
  writeAttribute("to", t->getElementType());
  putDoc(t);
  endElement();
}

template<> void 
XMLWriterImpl::WriterPass::put(const ArrayType* t)
{
  if (t->isIntrinsic())
    return;
  startElement("array");
  writeAttribute("id", t->getName());
  writeAttribute("of", t->getElementType());
  writeAttribute("length", t->getMaxSize());
  putDoc(t);
  endElement();
}

template<> void 
XMLWriterImpl::WriterPass::put(const VectorType* t)
{
  if (t->isIntrinsic())
    return;
  startElement("vector");
  writeAttribute("id", t->getName());
  writeAttribute("of", t->getElementType());
  writeAttribute("length", t->getSize());
  putDoc(t);
  endElement();
}

template<> void 
XMLWriterImpl::WriterPass::put(const StructureType* t)
{
  if (t->isIntrinsic())
    return;
  startElement("structure");
  writeAttribute("id",t->getName());
  putDoc(t);
  for (StructureType::const_iterator I = t->begin(), E = t->end(); 
       I != E; ++I) {
    startElement("field");
    Field* field = cast<Field>(*I);
    writeAttribute("id",field->getName());
    writeAttribute("type",field->getType());
    putDoc(field);
    endElement();
  }
  endElement();
}

template<> void 
XMLWriterImpl::WriterPass::put(const ContinuationType* t)
{
  if (t->isIntrinsic())
    return;
  startElement("continuation");
  writeAttribute("id",t->getName());
  putDoc(t);
  for (ContinuationType::const_iterator I = t->begin(), E = t->end(); 
       I != E; ++I) {
    startElement("field");
    Field* field = cast<Field>(*I);
    writeAttribute("id",field->getName());
    writeAttribute("type",field->getType());
    putDoc(field);
    endElement();
  }
  endElement();
}

template<> void 
XMLWriterImpl::WriterPass::put(const SignatureType* t)
{
  if (t->isIntrinsic())
    return;
  startElement("signature");
  writeAttribute("id",t->getName());
  writeAttribute("result",t->getResultType());
  if (t->isVarArgs())
    writeAttribute("varargs","true");
  putDoc(t);
  for (SignatureType::const_iterator I = t->begin(), E = t->end(); I != E; ++I)
  {
    startElement("arg");
    Parameter* param = cast<Parameter>(*I);
    writeAttribute("id",param->getName());
    writeAttribute("type",param->getType());
    putDoc(param);
    endElement();
  }
  endElement();
}

template<> void 
XMLWriterImpl::WriterPass::put(const StreamType* t)
{
  if (t->isIntrinsic())
    return;
  startElement("stream");
  writeAttribute("id",t->getName());
  putDoc(t);
  endElement();
}

template<> void 
XMLWriterImpl::WriterPass::put(const TextType* t)
{
  if (t->isIntrinsic())
    return;
  startElement("text");
  writeAttribute("id",t->getName());
  putDoc(t);
  endElement();
}

template<> void 
XMLWriterImpl::WriterPass::put(const BufferType* t)
{
  if (t->isIntrinsic())
    return;
  startElement("buffer");
  writeAttribute("id",t->getName());
  putDoc(t);
  endElement();
}

template<> void
XMLWriterImpl::WriterPass::put<ConstantAny>(const ConstantAny* i, bool nested)
{
  if (!nested) {
    startElement("constant");
    writeAttribute("id",i->getName());
    writeAttribute("type",i->getType()->getName());
    putDoc(i);
  }
  putConstantValue(i->getValue(),true);
}

template<> void 
XMLWriterImpl::WriterPass::put(const ConstantBoolean* i, bool nested)
{
  if (!nested) {
    startElement("constant");
    writeAttribute("id",i->getName());
    writeAttribute("type",i->getType()->getName());
    putDoc(i);
  }
  if (i->getValue())
    startElement("true");
  else
    startElement("false");
  endElement();
}

template<> void
XMLWriterImpl::WriterPass::put(const ConstantCharacter* i, bool nested)
{
  if (!nested) {
    startElement("constant");
    writeAttribute("id",i->getName());
    writeAttribute("type",i->getType()->getName());
    putDoc(i);
  }
  startElement("char");
  writeString(i->getValue());
  endElement();
}

template<> void
XMLWriterImpl::WriterPass::put(const ConstantEnumerator* i, bool nested)
{
  if (!nested) {
    startElement("constant");
    writeAttribute("id",i->getName());
    writeAttribute("type",i->getType()->getName());
    putDoc(i);
  }
  startElement("enum");
  writeString(i->getValue());
  endElement();
}

template<> void 
XMLWriterImpl::WriterPass::put(const ConstantInteger* i, bool nested)
{
  if (!nested) {
    startElement("constant");
    writeAttribute("id",i->getName());
    writeAttribute("type",i->getType()->getName());
    putDoc(i);
  }
  switch (i->getBase()) {
    case 2: startElement("bin"); break;
    case 8: startElement("oct"); break;
    case 16: startElement("hex"); break;
    default: startElement("dec"); break;
  }
  writeString(i->getValue());
  endElement();
}

template<> void
XMLWriterImpl::WriterPass::put(const ConstantReal* r, bool nested)
{
  if (!nested) {
    startElement("constant");
    writeAttribute("id",r->getName());
    writeAttribute("type",r->getType()->getName());
    putDoc(r);
  }
  const RealType* RT = cast<RealType>(r->getType());
  if (RT->getBits() <= 32)
    startElement("flt");
  else
    startElement("dbl");
  writeString(r->getValue());
  endElement();
}

template<> void
XMLWriterImpl::WriterPass::put(const ConstantString* t, bool nested)
{
  if (!nested) {
    startElement("constant");
    writeAttribute("id",t->getName());
    writeAttribute("type",t->getType()->getName());
    putDoc(t);
  }
  startElement("str");
  writeString(t->getValue());
  endElement();
}

template<> void
XMLWriterImpl::WriterPass::put(const ConstantPointer* i, bool nested)
{
  if (!nested) {
    startElement("constant");
    writeAttribute("id",i->getName());
    writeAttribute("type",i->getType()->getName());
    putDoc(i);
  }
  startElement("ptr");
  writeAttribute("to",i->getValue()->getName());
  endElement();
}

template<> void
XMLWriterImpl::WriterPass::put(const ConstantArray* i, bool nested)
{
  if (!nested) {
    startElement("constant");
    writeAttribute("id",i->getName());
    writeAttribute("type",i->getType()->getName());
    putDoc(i);
  }
  startElement("arr");
  for (ConstantArray::const_iterator I = i->begin(), E = i->end(); I != E; ++I)
    putConstantValue(*I,true);
  endElement();
}

template<> void
XMLWriterImpl::WriterPass::put(const ConstantVector* i, bool nested)
{
  if (!nested) {
    startElement("constant");
    writeAttribute("id",i->getName());
    writeAttribute("type",i->getType()->getName());
    putDoc(i);
  }
  startElement("vect");
  for (ConstantArray::const_iterator I = i->begin(), E = i->end(); I != E; ++I)
    putConstantValue(*I,true);
  endElement();
}

template<> void
XMLWriterImpl::WriterPass::put(const ConstantStructure* i, bool nested)
{
  if (!nested) {
    startElement("constant");
    writeAttribute("id",i->getName());
    writeAttribute("type",i->getType()->getName());
    putDoc(i);
  }
  startElement("struct");
  for (ConstantStructure::const_iterator I = i->begin(), E = i->end(); 
       I != E; ++I)
    putConstantValue(*I,true);
  endElement();
}

template<> void
XMLWriterImpl::WriterPass::put(const ConstantContinuation* i, bool nested)
{
  if (!nested) {
    startElement("constant");
    writeAttribute("id",i->getName());
    writeAttribute("type",i->getType()->getName());
    putDoc(i);
  }
  startElement("cont");
  for (ConstantContinuation::const_iterator I = i->begin(), E = i->end(); 
       I != E; ++I)
    putConstantValue(*I,true);
  endElement();
}

inline void
XMLWriterImpl::WriterPass::putConstantValue(const ConstantValue* V, bool nstd)
{
  switch (V->getID()) {
    case ConstantAnyID:          put(cast<ConstantAny>(V),nstd); break;
    case ConstantBooleanID:      put(cast<ConstantBoolean>(V),nstd); break;
    case ConstantCharacterID:    put(cast<ConstantCharacter>(V),nstd); break;
    case ConstantEnumeratorID:   put(cast<ConstantEnumerator>(V),nstd); break;
    case ConstantIntegerID:      put(cast<ConstantInteger>(V),nstd); break;
    case ConstantRealID:         put(cast<ConstantReal>(V),nstd); break;
    case ConstantStringID:       put(cast<ConstantString>(V),nstd); break;
    case ConstantPointerID:      put(cast<ConstantPointer>(V),nstd); break;
    case ConstantArrayID:        put(cast<ConstantArray>(V),nstd); break;
    case ConstantVectorID:       put(cast<ConstantVector>(V),nstd); break;
    case ConstantStructureID:    put(cast<ConstantStructure>(V),nstd); break;
    case ConstantContinuationID: put(cast<ConstantContinuation>(V),nstd); break;
    default:
      hlvmAssert(!"Invalid ConstantValue kind");
  }
}

template<> void
XMLWriterImpl::WriterPass::put(const Variable* v)
{
  startElement("variable");
  writeAttribute("id",v->getName());
  writeAttribute("type",v->getType()->getName());
  if (v->hasInitializer()) {
    Constant* C = llvm::cast<Constant>(v->getInitializer());
    writeAttribute("init",C->getName());
  }
  putDoc(v);
}

template<> void
XMLWriterImpl::WriterPass::put(const Function* f)
{
  startElement("function");
  writeAttribute("id",f->getName());
  writeAttribute("type",f->getSignature()->getName());
  writeAttribute("linkage",getLinkageKind(f->getLinkageKind()));
  putDoc(f);
}

template<> void 
XMLWriterImpl::WriterPass::put(const Program* p)
{
  startElement("program");
  writeAttribute("id",p->getName());
  putDoc(p);
}

template<> void 
XMLWriterImpl::WriterPass::put(const Block* b)
{
  startElement("block");
  if (!b->getLabel().empty())
    writeAttribute("label",b->getLabel());
  putDoc(b);
}

template<> void
XMLWriterImpl::WriterPass::put(const AutoVarOp* av)
{
  startElement("autovar");
  writeAttribute("id",av->getName());
  writeAttribute("type",av->getType()->getName());
  if (av->hasInitializer()) {
    Constant* C = llvm::cast<Constant>(av->getInitializer());
    writeAttribute("init",C->getName());
  }
  putDoc(av);
}

template<> void
XMLWriterImpl::WriterPass::put(const NegateOp* op)
{
  startElement("neg");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const ComplementOp* op)
{
  startElement("cmpl");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const PreIncrOp* op)
{
  startElement("preinc");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const PreDecrOp* op)
{
  startElement("predec");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const PostIncrOp* op)
{
  startElement("postinc");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const PostDecrOp* op)
{
  startElement("postdec");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const AddOp* op)
{
  startElement("add");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const SubtractOp* op)
{
  startElement("sub");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const MultiplyOp* op)
{
  startElement("mul");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const DivideOp* op)
{
  startElement("div");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const ModuloOp* op)
{
  startElement("mod");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const BAndOp* op)
{
  startElement("band");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const BOrOp* op)
{
  startElement("bor");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const BXorOp* op)
{
  startElement("bxor");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const BNorOp* op)
{
  startElement("bnor");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const NotOp* op)
{
  startElement("not");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const AndOp* op)
{
  startElement("and");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const OrOp* op)
{
  startElement("or");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const XorOp* op)
{
  startElement("xor");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const NorOp* op)
{
  startElement("nor");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const EqualityOp* op)
{
  startElement("eq");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const InequalityOp* op)
{
  startElement("ne");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const LessThanOp* op)
{
  startElement("lt");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const GreaterThanOp* op)
{
  startElement("gt");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const LessEqualOp* op)
{
  startElement("le");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const GreaterEqualOp* op)
{
  startElement("ge");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const SelectOp* op)
{
  startElement("select");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const SwitchOp* op) 
{
  startElement("switch");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const WhileOp* op) 
{
  startElement("while");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const UnlessOp* op) 
{
  startElement("unless");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const UntilOp* op) 
{
  startElement("until");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const LoopOp* op) 
{
  startElement("loop");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const BreakOp* op)
{
  startElement("break");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(const ContinueOp* op)
{
  startElement("continue");
  putDoc(op);
}

template<> void 
XMLWriterImpl::WriterPass::put(const ReturnOp* r)
{
  startElement("ret");
  putDoc(r);
}

template<> void 
XMLWriterImpl::WriterPass::put(const ResultOp* r)
{
  startElement("result");
  putDoc(r);
}

template<> void 
XMLWriterImpl::WriterPass::put(const CallOp* r)
{
  startElement("call");
  putDoc(r);
}

template<> void 
XMLWriterImpl::WriterPass::put(const StoreOp* r)
{
  startElement("store");
  putDoc(r);
}

template<> void 
XMLWriterImpl::WriterPass::put(const LoadOp* r)
{
  startElement("load");
  putDoc(r);
}

template<> void 
XMLWriterImpl::WriterPass::put(const ReferenceOp* r)
{
  startElement("ref");
  const Value* ref = r->getReferent();
  std::string name;
  if (isa<AutoVarOp>(ref))
    name = cast<AutoVarOp>(ref)->getName();
  else if (isa<Argument>(ref))
    name = cast<Argument>(ref)->getName();
  else if (isa<Constant>(ref))
    name = cast<Constant>(ref)->getName();
  else
    name = "oops";
  writeAttribute("id",name);
  putDoc(r);
}

template<> void 
XMLWriterImpl::WriterPass::put(const OpenOp* r)
{
  startElement("open");
  putDoc(r);
}

template<> void 
XMLWriterImpl::WriterPass::put(const WriteOp* r)
{
  startElement("write");
  putDoc(r);
}

template<> void 
XMLWriterImpl::WriterPass::put(const CloseOp* r)
{
  startElement("close");
  putDoc(r);
}

template<> void 
XMLWriterImpl::WriterPass::put(const Bundle* b)
{
  startElement("bundle");
  writeAttribute("id",b->getName());
  putDoc(b);
}

void
XMLWriterImpl::WriterPass::handle(Node* n,Pass::TraversalKinds mode)
{
  if (mode & Pass::PreOrderTraversal) {
    switch (n->getID()) 
    {
      case BundleID:               put(cast<Bundle>(n)); break;
      /* TYPES */
      case AnyTypeID:              put(cast<AnyType>(n)); break;
      case ArrayTypeID:            put(cast<ArrayType>(n)); break;
      case BooleanTypeID:          put(cast<BooleanType>(n)); break;
      case BufferTypeID:           put(cast<BufferType>(n)); break;
      case CharacterTypeID:        put(cast<CharacterType>(n)); break;
      case ContinuationTypeID:     put(cast<ContinuationType>(n)); break;
      case EnumerationTypeID:      put(cast<EnumerationType>(n)); break;
      case IntegerTypeID:          put(cast<IntegerType>(n)); break;
      case NamedTypeID:            put(cast<NamedType>(n)); break;
      case OpaqueTypeID:           put(cast<OpaqueType>(n)); break;
      case PointerTypeID:          put(cast<PointerType>(n)); break;
      case RangeTypeID:            put(cast<RangeType>(n)); break;
      case RealTypeID:             put(cast<RealType>(n)); break;
      case SignatureTypeID:        put(cast<SignatureType>(n)); break;
      case StreamTypeID:           put(cast<StreamType>(n)); break;
      case StringTypeID:           put(cast<StringType>(n)); break;
      case StructureTypeID:        put(cast<StructureType>(n)); break;
      case TextTypeID:             put(cast<TextType>(n)); break;
      case VectorTypeID:           put(cast<VectorType>(n)); break;
      /* CONSTANTS */
      case ConstantAnyID:          
      case ConstantBooleanID:      
      case ConstantCharacterID:    
      case ConstantEnumeratorID:   
      case ConstantIntegerID:      
      case ConstantRealID:         
      case ConstantStringID:       
      case ConstantPointerID:      
      case ConstantArrayID:        
      case ConstantVectorID:       
      case ConstantStructureID:    
      case ConstantContinuationID: 
        putConstantValue(cast<ConstantValue>(n),false); break;
      /* LINKABLES */
      case VariableID:             put(cast<Variable>(n)); break;
      case FunctionID:             put(cast<Function>(n)); break;
      case ProgramID:              put(cast<Program>(n)); break;
      case BlockID:                put(cast<Block>(n)); break;
      /* OPERATORS */
      case AutoVarOpID:            put(cast<AutoVarOp>(n)); break;
      case NegateOpID:             put(cast<NegateOp>(n)); break;
      case ComplementOpID:         put(cast<ComplementOp>(n)); break;
      case PreIncrOpID:            put(cast<PreIncrOp>(n)); break;
      case PreDecrOpID:            put(cast<PreDecrOp>(n)); break;
      case PostIncrOpID:           put(cast<PostIncrOp>(n)); break;
      case PostDecrOpID:           put(cast<PostDecrOp>(n)); break;
      case AddOpID:                put(cast<AddOp>(n)); break;
      case SubtractOpID:           put(cast<SubtractOp>(n)); break;
      case MultiplyOpID:           put(cast<MultiplyOp>(n)); break;
      case DivideOpID:             put(cast<DivideOp>(n)); break;
      case ModuloOpID:             put(cast<ModuloOp>(n)); break;
      case BAndOpID:               put(cast<BAndOp>(n)); break;
      case BOrOpID:                put(cast<BOrOp>(n)); break;
      case BXorOpID:               put(cast<BXorOp>(n)); break;
      case BNorOpID:               put(cast<BNorOp>(n)); break;
      case NotOpID:                put(cast<NotOp>(n)); break;
      case AndOpID:                put(cast<AndOp>(n)); break;
      case OrOpID:                 put(cast<OrOp>(n)); break;
      case XorOpID:                put(cast<XorOp>(n)); break;
      case NorOpID:                put(cast<NorOp>(n)); break;
      case EqualityOpID:           put(cast<EqualityOp>(n)); break;
      case InequalityOpID:         put(cast<InequalityOp>(n)); break;
      case LessThanOpID:           put(cast<LessThanOp>(n)); break;
      case GreaterThanOpID:        put(cast<GreaterThanOp>(n)); break;
      case LessEqualOpID:          put(cast<LessEqualOp>(n)); break;
      case GreaterEqualOpID:       put(cast<GreaterEqualOp>(n)); break;
      case SelectOpID:             put(cast<SelectOp>(n)); break;
      case SwitchOpID:             put(cast<SwitchOp>(n)); break;
      case WhileOpID:              put(cast<WhileOp>(n)); break;
      case UnlessOpID:             put(cast<UnlessOp>(n)); break;
      case UntilOpID:              put(cast<UntilOp>(n)); break;
      case LoopOpID:               put(cast<LoopOp>(n)); break;
      case BreakOpID:              put(cast<BreakOp>(n)); break;
      case ContinueOpID:           put(cast<ContinueOp>(n)); break;
      case ReturnOpID:             put(cast<ReturnOp>(n)); break;
      case ResultOpID:             put(cast<ResultOp>(n)); break;
      case CallOpID:               put(cast<CallOp>(n)); break;
      case StoreOpID:              put(cast<StoreOp>(n)); break;
      case LoadOpID:               put(cast<LoadOp>(n)); break;
      case ReferenceOpID:          put(cast<ReferenceOp>(n)); break;
      case OpenOpID:               put(cast<OpenOp>(n)); break;
      case CloseOpID:              put(cast<CloseOp>(n)); break;
      case WriteOpID:              put(cast<WriteOp>(n)); break;
      default:
        hlvmDeadCode("Unknown Type");
        break;
    }
  }
  if (mode & Pass::PostOrderTraversal) {
    // The types print their own end element because intrinsic types
    // don't print at all so we can't assume a startElement.
    switch (n->getID()) 
    {
      case AnyTypeID: 
      case ArrayTypeID:  
      case BooleanTypeID:
      case BufferTypeID:
      case CharacterTypeID:
      case EnumerationTypeID:
      case IntegerTypeID: 
      case NamedTypeID:
      case OpaqueTypeID:   
      case PointerTypeID: 
      case RangeTypeID:  
      case RealTypeID:      
      case StringTypeID:
      case StreamTypeID:
      case StructureTypeID:
      case SignatureTypeID:
      case TextTypeID:
      case VectorTypeID: 
        break;
      default:
        // For everything else, there was a startElement, so just end it now
        endElement();
    }
  }
}

void
XMLWriterImpl::write(AST* ast) 
{
  pass.node = ast;
  pass.putHeader();
  PassManager* PM = PassManager::create();
  PM->addPass(&pass);
  PM->runOn(ast);
  pass.putFooter();
}

}

XMLWriter* 
hlvm::XMLWriter::create(const char* fname)
{
  return new XMLWriterImpl(fname);
}

#ifdef HLVM_DEBUG
void 
hlvm::dump(Node*)
{
}
#endif

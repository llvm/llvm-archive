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
    inline void writeElement(const char* elem, const char* body)
      { xmlTextWriterWriteElement(writer,
          reinterpret_cast<const xmlChar*>(elem),
          reinterpret_cast<const xmlChar*>(body)); }
    inline void writeString(const std::string& str) ;

    inline void putHeader();
    inline void putFooter();
    inline void putDoc(Documentable* node);

    template<class NodeClass>
    inline void put(NodeClass* nc);

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
XMLWriterImpl::WriterPass::put(Documentation* b)
{
  startElement("doc");
  const std::string& data = b->getDoc();
  xmlTextWriterWriteRawLen(writer,
    reinterpret_cast<const xmlChar*>(data.c_str()),data.length());
  endElement();
}

inline void
XMLWriterImpl::WriterPass::putDoc(Documentable* node)
{
  Documentation* theDoc = node->getDoc();
  if (theDoc) {
    this->put<Documentation>(theDoc);
  }
}

template<> void 
XMLWriterImpl::WriterPass::put(AnyType* t)
{
  startElement("atom");
  writeAttribute("id",t->getName());
  putDoc(t);
  startElement("intrinsic");
  writeAttribute("is","any");
  endElement();
}

template<> void 
XMLWriterImpl::WriterPass::put(StringType* t)
{
  startElement("atom");
  writeAttribute("id",t->getName());
  putDoc(t);
  startElement("intrinsic");
  writeAttribute("is","string");
  endElement();
}

template<>void
XMLWriterImpl::WriterPass::put(BooleanType* t)
{
  startElement("atom");
  writeAttribute("id",t->getName());
  putDoc(t);
  startElement("intrinsic");
  writeAttribute("is","bool");
  endElement();
}

template<> void
XMLWriterImpl::WriterPass::put(CharacterType* t)
{
  startElement("atom");
  writeAttribute("id",t->getName());
  putDoc(t);
  startElement("intrinsic");
  writeAttribute("is","char");
  endElement();
}

template<> void
XMLWriterImpl::WriterPass::put(IntegerType* t)
{
  startElement("atom");
  writeAttribute("id",t->getName());
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
}

template<> void
XMLWriterImpl::WriterPass::put(RangeType* t)
{
  startElement("range");
  writeAttribute("id",t->getName());
  writeAttribute("min",t->getMin());
  writeAttribute("max",t->getMax());
  putDoc(t);
}

template<> void 
XMLWriterImpl::WriterPass::put(EnumerationType* t)
{
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
}

template<> void
XMLWriterImpl::WriterPass::put(RealType* t)
{
  startElement("atom");
  writeAttribute("id",t->getName());
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
}

template<> void
XMLWriterImpl::WriterPass::put(OctetType* t)
{
  startElement("atom");
  writeAttribute("id",t->getName());
  putDoc(t);
  startElement("intrinsic");
  writeAttribute("is","octet");
  endElement();
}

template<> void
XMLWriterImpl::WriterPass::put(OpaqueType* op)
{
  startElement("opaque");
  writeAttribute("id",op->getName());
  putDoc(op);
}

template<> void 
XMLWriterImpl::WriterPass::put(PointerType* t)
{
  startElement("pointer");
  writeAttribute("id", t->getName());
  writeAttribute("to", t->getElementType());
  putDoc(t);
}

template<> void 
XMLWriterImpl::WriterPass::put(ArrayType* t)
{
  startElement("array");
  writeAttribute("id", t->getName());
  writeAttribute("of", t->getElementType());
  writeAttribute("length", t->getMaxSize());
  putDoc(t);
}

template<> void 
XMLWriterImpl::WriterPass::put(VectorType* t)
{
  startElement("vector");
  writeAttribute("id", t->getName());
  writeAttribute("of", t->getElementType());
  writeAttribute("length", t->getSize());
  putDoc(t);
}

template<> void 
XMLWriterImpl::WriterPass::put(StructureType* t)
{
  startElement("structure");
  writeAttribute("id",t->getName());
  putDoc(t);
  for (StructureType::iterator I = t->begin(), E = t->end(); I != E; ++I) {
    startElement("field");
    Field* field = cast<Field>(*I);
    writeAttribute("id",field->getName());
    writeAttribute("type",field->getType());
    putDoc(field);
    endElement();
  }
}

template<> void 
XMLWriterImpl::WriterPass::put(SignatureType* t)
{
  startElement("signature");
  writeAttribute("id",t->getName());
  writeAttribute("result",t->getResultType());
  if (t->isVarArgs())
    writeAttribute("varargs","true");
  putDoc(t);
  for (SignatureType::iterator I = t->begin(), E = t->end(); I != E; ++I) {
    startElement("arg");
    Parameter* param = cast<Parameter>(*I);
    writeAttribute("id",param->getName());
    writeAttribute("type",param->getType());
    putDoc(param);
    endElement();
  }
}


template<> void
XMLWriterImpl::WriterPass::put(ConstantAny* i)
{
  startElement("constant");
  writeAttribute("id",i->getName());
  writeAttribute("type",i->getType()->getName());
}

template<> void 
XMLWriterImpl::WriterPass::put(ConstantBoolean* i)
{
  startElement("constant");
  writeAttribute("id",i->getName());
  writeAttribute("type",i->getType()->getName());
  if (i->getValue())
    startElement("true");
  else
    startElement("false");
  endElement();
}

template<> void
XMLWriterImpl::WriterPass::put(ConstantCharacter* i)
{
  startElement("constant");
  writeAttribute("id",i->getName());
  writeAttribute("type",i->getType()->getName());
}

template<> void
XMLWriterImpl::WriterPass::put(ConstantEnumerator* i)
{
  startElement("constant");
  writeAttribute("id",i->getName());
  writeAttribute("type",i->getType()->getName());
}

template<> void
XMLWriterImpl::WriterPass::put(ConstantOctet* i)
{
  startElement("constant");
  writeAttribute("id",i->getName());
  writeAttribute("type",i->getType()->getName());
}

template<> void 
XMLWriterImpl::WriterPass::put(ConstantInteger* i)
{
  startElement("constant");
  writeAttribute("id",i->getName());
  writeAttribute("type",i->getType()->getName());
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
XMLWriterImpl::WriterPass::put(ConstantReal* r)
{
  startElement("constant");
  writeAttribute("id",r->getName());
  writeAttribute("type",r->getType()->getName());
  startElement("dbl");
  writeString(r->getValue());
  endElement();
}

template<> void
XMLWriterImpl::WriterPass::put(ConstantString* t)
{
  startElement("constant");
  writeAttribute("id",t->getName());
  writeAttribute("type",t->getType()->getName());
  startElement("string");
  writeString(t->getValue());
  endElement();
}

template<> void
XMLWriterImpl::WriterPass::put(ConstantPointer* i)
{
  startElement("constant");
  writeAttribute("id",i->getName());
  writeAttribute("type",i->getType()->getName());
}

template<> void
XMLWriterImpl::WriterPass::put(ConstantArray* i)
{
  startElement("constant");
  writeAttribute("id",i->getName());
  writeAttribute("type",i->getType()->getName());
}

template<> void
XMLWriterImpl::WriterPass::put(ConstantVector* i)
{
  startElement("constant");
  writeAttribute("id",i->getName());
  writeAttribute("type",i->getType()->getName());
}

template<> void
XMLWriterImpl::WriterPass::put(ConstantStructure* i)
{
  startElement("constant");
  writeAttribute("id",i->getName());
  writeAttribute("type",i->getType()->getName());
}

template<> void
XMLWriterImpl::WriterPass::put(ConstantContinuation* i)
{
  startElement("constant");
  writeAttribute("id",i->getName());
  writeAttribute("type",i->getType()->getName());
}

template<> void
XMLWriterImpl::WriterPass::put(Variable* v)
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
XMLWriterImpl::WriterPass::put(Function* f)
{
  startElement("function");
  writeAttribute("id",f->getName());
  writeAttribute("type",f->getSignature()->getName());
  writeAttribute("linkage",getLinkageKind(f->getLinkageKind()));
  putDoc(f);
}

template<> void 
XMLWriterImpl::WriterPass::put(Program* p)
{
  startElement("program");
  writeAttribute("id",p->getName());
  putDoc(p);
}

template<> void 
XMLWriterImpl::WriterPass::put(Block* b)
{
  startElement("block");
  if (!b->getLabel().empty())
    writeAttribute("label",b->getLabel());
  putDoc(b);
}

template<> void
XMLWriterImpl::WriterPass::put(AutoVarOp* av)
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
XMLWriterImpl::WriterPass::put(NegateOp* op)
{
  startElement("neg");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(ComplementOp* op)
{
  startElement("cmpl");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(PreIncrOp* op)
{
  startElement("preinc");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(PreDecrOp* op)
{
  startElement("predec");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(PostIncrOp* op)
{
  startElement("postinc");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(PostDecrOp* op)
{
  startElement("postdec");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(AddOp* op)
{
  startElement("add");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(SubtractOp* op)
{
  startElement("sub");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(MultiplyOp* op)
{
  startElement("mul");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(DivideOp* op)
{
  startElement("div");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(ModuloOp* op)
{
  startElement("mod");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(BAndOp* op)
{
  startElement("band");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(BOrOp* op)
{
  startElement("bor");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(BXorOp* op)
{
  startElement("bxor");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(BNorOp* op)
{
  startElement("bnor");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(NotOp* op)
{
  startElement("not");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(AndOp* op)
{
  startElement("and");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(OrOp* op)
{
  startElement("or");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(XorOp* op)
{
  startElement("xor");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(NorOp* op)
{
  startElement("nor");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(EqualityOp* op)
{
  startElement("eq");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(InequalityOp* op)
{
  startElement("ne");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(LessThanOp* op)
{
  startElement("lt");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(GreaterThanOp* op)
{
  startElement("gt");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(LessEqualOp* op)
{
  startElement("le");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(GreaterEqualOp* op)
{
  startElement("ge");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(SelectOp* op)
{
  startElement("select");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(SwitchOp* op) 
{
  startElement("switch");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(WhileOp* op) 
{
  startElement("while");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(UnlessOp* op) 
{
  startElement("unless");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(UntilOp* op) 
{
  startElement("until");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(LoopOp* op) 
{
  startElement("loop");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(BreakOp* op)
{
  startElement("break");
  putDoc(op);
}

template<> void
XMLWriterImpl::WriterPass::put(ContinueOp* op)
{
  startElement("continue");
  putDoc(op);
}

template<> void 
XMLWriterImpl::WriterPass::put(ReturnOp* r)
{
  startElement("ret");
  putDoc(r);
}

template<> void 
XMLWriterImpl::WriterPass::put(ResultOp* r)
{
  startElement("result");
  putDoc(r);
}

template<> void 
XMLWriterImpl::WriterPass::put(CallOp* r)
{
  startElement("call");
  putDoc(r);
}

template<> void 
XMLWriterImpl::WriterPass::put(StoreOp* r)
{
  startElement("store");
  putDoc(r);
}

template<> void 
XMLWriterImpl::WriterPass::put(LoadOp* r)
{
  startElement("load");
  putDoc(r);
}

template<> void 
XMLWriterImpl::WriterPass::put(ReferenceOp* r)
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
XMLWriterImpl::WriterPass::put(OpenOp* r)
{
  startElement("open");
  putDoc(r);
}

template<> void 
XMLWriterImpl::WriterPass::put(WriteOp* r)
{
  startElement("write");
  putDoc(r);
}

template<> void 
XMLWriterImpl::WriterPass::put(CloseOp* r)
{
  startElement("close");
  putDoc(r);
}

template<> void 
XMLWriterImpl::WriterPass::put(Bundle* b)
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
      case AnyTypeID:              put(cast<AnyType>(n)); break;
      case StringTypeID:           put(cast<StringType>(n)); break;
      case BooleanTypeID:          put(cast<BooleanType>(n)); break;
      case BundleID:               put(cast<Bundle>(n)); break;
      case CharacterTypeID:        put(cast<CharacterType>(n)); break;
      case IntegerTypeID:          put(cast<IntegerType>(n)); break;
      case RangeTypeID:            put(cast<RangeType>(n)); break;
      case EnumerationTypeID:      put(cast<EnumerationType>(n)); break;
      case RealTypeID:             put(cast<RealType>(n)); break;
      case OctetTypeID:            put(cast<OctetType>(n)); break;
      case OpaqueTypeID:           put(cast<OpaqueType>(n)); break;
      case PointerTypeID:          put(cast<PointerType>(n)); break;
      case ArrayTypeID:            put(cast<ArrayType>(n)); break;
      case VectorTypeID:           put(cast<VectorType>(n)); break;
      case StructureTypeID:        put(cast<StructureType>(n)); break;
      case SignatureTypeID:        put(cast<SignatureType>(n)); break;
      case ConstantAnyID:          put(cast<ConstantAny>(n)); break;
      case ConstantBooleanID:      put(cast<ConstantBoolean>(n)); break;
      case ConstantCharacterID:    put(cast<ConstantCharacter>(n)); break;
      case ConstantEnumeratorID:   put(cast<ConstantEnumerator>(n)); break;
      case ConstantOctetID:        put(cast<ConstantOctet>(n)); break;
      case ConstantIntegerID:      put(cast<ConstantInteger>(n)); break;
      case ConstantRealID:         put(cast<ConstantReal>(n)); break;
      case ConstantStringID:       put(cast<ConstantString>(n)); break;
      case ConstantPointerID:      put(cast<ConstantPointer>(n)); break;
      case ConstantArrayID:        put(cast<ConstantArray>(n)); break;
      case ConstantVectorID:       put(cast<ConstantVector>(n)); break;
      case ConstantStructureID:    put(cast<ConstantStructure>(n)); break;
      case ConstantContinuationID: put(cast<ConstantContinuation>(n)); break;
      case VariableID:             put(cast<Variable>(n)); break;
      case FunctionID:             put(cast<Function>(n)); break;
      case ProgramID:              put(cast<Program>(n)); break;
      case BlockID:                put(cast<Block>(n)); break;
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
    endElement();
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

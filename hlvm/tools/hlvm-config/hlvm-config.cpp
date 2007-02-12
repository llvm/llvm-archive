//===-- hlvm-config Main Program --------------------------------*- C++ -*-===//
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
/// @file tools/hlvm-config/hlvm-config.cpp
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/05/23
/// @since 0.1.0
/// @brief Implements the main program for the hlvm-config tool
//===----------------------------------------------------------------------===//

#include <hlvm/AST/Node.h>
#include <hlvm/AST/AST.h>
#include <hlvm/AST/Arithmetic.h>
#include <hlvm/AST/Block.h>
#include <hlvm/AST/BooleanOps.h>
#include <hlvm/AST/Bundle.h>
#include <hlvm/AST/Constants.h>
#include <hlvm/AST/ContainerType.h>
#include <hlvm/AST/ControlFlow.h>
#include <hlvm/AST/Documentation.h>
#include <hlvm/AST/InputOutput.h>
#include <hlvm/AST/Linkables.h>
#include <hlvm/AST/MemoryOps.h>
#include <hlvm/AST/RealMath.h>
#include <hlvm/AST/RuntimeType.h>
#include <hlvm/AST/StringOps.h>
#include <hlvm/Base/Configuration.h>
#include <llvm/Support/CommandLine.h>
#include <iostream>
#include <iomanip>

using namespace llvm;
using namespace hlvm;

static llvm::cl::opt<bool> ShowNodeIds("node-ids", cl::init(false),
  cl::desc("Show the node ids"));

struct ClassInfo {
  const char* name;
  size_t size;
};

static ClassInfo NodeIDStrs[] =
{
  { "NoType", 0 },
  { "TreeTop", sizeof(AST) },
  { "Documentation", sizeof(Documentation) },
  { "NamedType", sizeof(NamedType) },
  { "Bundle", sizeof(Bundle) },
  { "Import", sizeof(Import) },
  { "AnyType", sizeof(AnyType) },
  { "BooleanType", sizeof(BooleanType) },
  { "CharacterType", sizeof(CharacterType) },
  { "EnumerationType", sizeof(EnumerationType) },
  { "IntegerType", sizeof(IntegerType) },
  { "OpaqueType", sizeof(OpaqueType) },
  { "RangeType", sizeof(RangeType) },
  { "RealType", sizeof(RealType) },
  { "RationalType", sizeof(RationalType) },
  { "StringType", sizeof(StringType) },
  { "PointerType", sizeof(PointerType) },
  { "ArrayType", sizeof(ArrayType) },
  { "VectorType", sizeof(VectorType) },
  { "StructureType", sizeof(StructureType) },
  { "SignatureType", sizeof(SignatureType) },
  { "ContinuationType", sizeof(ContinuationType) },
  { "BufferType", sizeof(BufferType) },
  { "StreamType", sizeof(StreamType) },
  { "TextType", sizeof(TextType) },
  { "Interface", 0 /*sizeof(Interface)*/ },
  { "Class", 0, /*sizeof(Class)*/ },
  { "Method", 0 /*sizeof(Method)*/ },
  { "Implements", 0 /*sizeof(Implements)*/ },
  { "Argument", sizeof(Argument) },
  { "ConstantAny", sizeof(ConstantAny) },
  { "ConstantBoolean", sizeof(ConstantBoolean) },
  { "ConstantCharacter", sizeof(ConstantCharacter) },
  { "ConstantEnumerator", sizeof(ConstantEnumerator) },
  { "ConstantInteger", sizeof(ConstantInteger) },
  { "ConstantReal", sizeof(ConstantReal) },
  { "ConstantString", sizeof(ConstantString) },
  { "ConstantPointer", sizeof(ConstantPointer) },
  { "ConstantArray", sizeof(ConstantArray) },
  { "ConstantVector", sizeof(ConstantVector) },
  { "ConstantStructure", sizeof(ConstantStructure) },
  { "ConstantContinuation", sizeof(ConstantContinuation) },
  { "ConstantExpression", sizeof(ConstantExpression) },
  { "Variable", sizeof(Variable) },
  { "Function", sizeof(Function) },
  { "Program", sizeof(Program) },
  { "Block", sizeof(Block) },
  { "BreakOp", sizeof(BreakOp) },
  { "ContinueOp", sizeof(ContinueOp) },
  { "ReturnOp", sizeof(ReturnOp) },
  { "GetOp", sizeof(GetOp) },
  { "ResultOp", sizeof(ResultOp) },
  { "ThrowOp", 0 /*sizeof(ThrowOp)*/ },
  { "NotOp", sizeof(NotOp) },
  { "NegateOp", sizeof(NegateOp) },
  { "ComplementOp", sizeof(ComplementOp) },
  { "PreIncrOp", sizeof(PreIncrOp) },
  { "PostIncrOp", sizeof(PostIncrOp) },
  { "PreDecrOp", sizeof(PreDecrOp) },
  { "PostDecrOp", sizeof(PostDecrOp) },
  { "SizeOfOp", sizeof(SizeOfOp) },
  { "ConvertOp", sizeof(ConvertOp) },
  { "IsPInfOp", sizeof(IsPInfOp) },
  { "IsNInfOp", sizeof(IsNInfOp) },
  { "IsNanOp", sizeof(IsNanOp) },
  { "TruncOp", sizeof(TruncOp) },
  { "RoundOp", sizeof(RoundOp) },
  { "FloorOp", sizeof(FloorOp) },
  { "CeilingOp", sizeof(CeilingOp) },
  { "LogEOp", sizeof(LogEOp) },
  { "Log2Op", sizeof(Log2Op) },
  { "Log10Op", sizeof(Log10Op) },
  { "SquareRootOp", sizeof(SquareRootOp) },
  { "CubeRootOp", sizeof(CubeRootOp) },
  { "FactorialOp", sizeof(FactorialOp) },
  { "LoadOp", sizeof(LoadOp) },
  { "AllocateOp", sizeof(AllocateOp) },
  { "DeallocateOp", sizeof(DeallocateOp) },
  { "AutoVarOp", sizeof(AutoVarOp) },
  { "TellOp", 0 /*sizeof(TellOp)*/ },
  { "CloseOp", sizeof(CloseOp) },
  { "LengthOp", sizeof(LengthOp) },
  { "AddOp", sizeof(AddOp) },
  { "SubtractOp", sizeof(SubtractOp) },
  { "MultiplyOp", sizeof(MultiplyOp) },
  { "DivideOp", sizeof(DivideOp) },
  { "ModuloOp", sizeof(ModuloOp) },
  { "BAndOp", sizeof(BAndOp) },
  { "BOrOp", sizeof(BOrOp) },
  { "BXorOp", sizeof(BXorOp) },
  { "BNorOp", sizeof(BNorOp) },
  { "AndOp", sizeof(AndOp) },
  { "OrOp", sizeof(OrOp) },
  { "NorOp", sizeof(NorOp) },
  { "XorOp", sizeof(XorOp) },
  { "LessThanOp", sizeof(LessThanOp) },
  { "GreaterThanOp", sizeof(GreaterThanOp) },
  { "LessEqualOp", sizeof(LessEqualOp) },
  { "GreaterEqualOp", sizeof(GreaterEqualOp) },
  { "EqualityOp", sizeof(EqualityOp) },
  { "InequalityOp", sizeof(InequalityOp) },
  { "PowerOp", sizeof(PowerOp) },
  { "RootOp", sizeof(RootOp) },
  { "GCDOp", sizeof(GCDOp) },
  { "LCMOp", sizeof(LCMOp) },
  { "ReallocateOp", 0 /*sizeof(ReallocateOp)*/ },
  { "StoreOp", sizeof(StoreOp) },
  { "GetIndexOp", sizeof(GetIndexOp) },
  { "GetFieldOp", sizeof(GetFieldOp) },
  { "WhileOp", sizeof(WhileOp) },
  { "UnlessOp", sizeof(UnlessOp) },
  { "UntilOp", sizeof(UntilOp) },
  { "OpenOp", sizeof(OpenOp) },
  { "ReadOp", sizeof(ReadOp) },
  { "WriteOp", sizeof(WriteOp) },
  { "CreateContOp", 0 /*sizeof(CreateContOp)*/ },
  { "SelectOp", sizeof(SelectOp) },
  { "StrInsertOp", sizeof(StrInsertOp) },
  { "StrEraseOp", sizeof(StrEraseOp) },
  { "StrReplaceOp", sizeof(StrReplaceOp) },
  { "StrConcatOp", sizeof(StrConcatOp) },
  { "PositionOp", 0 /*sizeof(PositionOp)*/ },
  { "LoopOp", sizeof(LoopOp) },
  { "CallOp", sizeof(CallOp) },
  { "InvokeOp", 0 /*sizeof(InvokeOp)*/ },
  { "DispatchOp", 0 /*sizeof(DispatchOp)*/ },
  { "CallWithContOp", 0 /*sizeof(CallWithContOp)*/ },
  { "SwitchOp", sizeof(SwitchOp) },
  { "**INVALID**", 0 },
  { "**INVALID**", 0 },
  { "**INVALID**", 0 },
  { "**INVALID**", 0 },
  { "**INVALID**", 0 },
  { "**INVALID**", 0 },
  { "**INVALID**", 0 },
  { "**INVALID**", 0 },
  { "**INVALID**", 0 },
  { "**INVALID**", 0 },
};

void showNodeIds()
{
  unsigned rowcount = 4;
  std::cout << "NodeID NodeSize Class Name\n";
  for (unsigned i = FirstNodeID; i <= LastNodeID; i++) {
    std::cout << std::setw(6) << i << " " << std::setw(8) << NodeIDStrs[i].size
      << " " << NodeIDStrs[i].name << "\n";
    rowcount++;
  }

  std::cout << "\n\nNumber of NodeIDs: " << NumNodeIDs << "\n";
  std::cout << "Nodes: " 
            << FirstNodeID << " -> "
            << LastNodeID << "\n";
  std::cout << "Documentables: " 
            << FirstDocumentableID << " -> "
            << LastDocumentableID << "\n";
  std::cout << "Types:" 
            << FirstTypeID << " -> " 
            << LastTypeID << "\n";
  std::cout << "ContainerTypes: " 
            << FirstContainerTypeID << " -> "
            << LastContainerTypeID << "\n";
  std::cout << "UniformContainerTypes: " 
            << FirstUniformContainerTypeID << " -> "
            << LastUniformContainerTypeID << "\n";
  std::cout << "DisparateContainerTypes: " 
            << FirstDisparateContainerTypeID << " -> "
            << LastDisparateContainerTypeID << "\n";
  std::cout << "RuntimeTypes: " 
            << FirstRuntimeTypeID << " -> "
            << LastRuntimeTypeID << "\n";
  std::cout << "Values: " 
            << FirstValueID << " -> " 
            << LastValueID << "\n";
  std::cout << "Constants: " 
            << FirstConstantID << " -> "
            << LastConstantID << "\n";
  std::cout << "ConstantValues: " 
            << FirstConstantValueID << " -> "
            << LastConstantValueID << "\n";
  std::cout << "ConstantAggregates: " 
            << FirstConstantAggregateID << " -> "
            << LastConstantAggregateID << "\n";
  std::cout << "Linkables: " 
            << FirstLinkableID << " -> "
            << LastLinkableID << "\n";
  std::cout << "Operators: " 
            << FirstOperatorID << " -> "
            << LastOperatorID << "\n";
  std::cout << "NilaryOperators: " 
            << FirstNilaryOperatorID << " -> "
            << LastNilaryOperatorID << "\n";
  std::cout << "UnaryOperators: " 
            << FirstUnaryOperatorID << " -> "
            << LastUnaryOperatorID << "\n";
  std::cout << "BinaryOperators: " 
            << FirstBinaryOperatorID << " -> "
            << LastBinaryOperatorID << "\n";
  std::cout << "TernaryOperators: " 
            << FirstTernaryOperatorID << " -> "
            << LastTernaryOperatorID << "\n";
  std::cout << "MultiOperators: " 
            << FirstMultiOperatorID << " -> "
            << LastMultiOperatorID << "\n";
  if (sizeof(NodeIDStrs)/sizeof(NodeIDStrs[0]) != NumNodeIDs+10)
    std::cout << "\n**##!! NodeIDStrs Out Of Date !!##**\n\n";
}

void printVersion() 
{
  std::cout << HLVM_Version << "\n";
}

static llvm::cl::opt<bool> ConfigTime("config-time", cl::init(false),
  cl::desc("Show when HLVM was configured"));
static llvm::cl::opt<bool> Copyright("copyright", cl::init(false),
  cl::desc("Show when HLVM copyright notice"));
static llvm::cl::opt<bool> Maintainer("maintainer", cl::init(false),
  cl::desc("Show the maintainer of this HLVM release"));

int 
main(int argc, char**argv) 
{
  cl::SetVersionPrinter(printVersion);
  cl::ParseCommandLineOptions(argc, argv, 
    "hlvm-config: HLVM Configuration Utility\n");

  if (ShowNodeIds)
    showNodeIds();
  if (ConfigTime)
    std::cout << HLVM_ConfigTime << "\n";
  if (Copyright)
    std::cout << HLVM_Copyright << "\n";
  if (Maintainer)
    std::cout << HLVM_Maintainer << "\n";

  return 0;
}

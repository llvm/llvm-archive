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
#include <hlvm/Base/Configuration.h>
#include <llvm/Support/CommandLine.h>
#include <iostream>

using namespace llvm;
using namespace hlvm;

static llvm::cl::opt<bool> ShowNodeIds("node-ids", cl::init(false),
  cl::desc("Show the node ids"));

static const char* NodeIDStrs[] =
{
  "NoType",
  "TreeTop",
  "Documentation",
  "NamedType",
  "Bundle",
  "Import",
  "AnyType",
  "BooleanType",
  "CharacterType",
  "EnumerationType",
  "IntegerType",
  "OpaqueType",
  "RangeType",
  "RealType",
  "RationalType",
  "StringType",
  "PointerType",
  "ArrayType",
  "VectorType",
  "StructureType",
  "SignatureType",
  "ContinuationType",
  "BufferType",
  "StreamType",
  "TextType",
  "Interface",
  "Class",
  "Method",
  "Implements",
  "Argument",
  "ConstantAny",
  "ConstantBoolean",
  "ConstantCharacter",
  "ConstantEnumerator",
  "ConstantInteger",
  "ConstantReal",
  "ConstantString",
  "ConstantPointer",
  "ConstantArray",
  "ConstantVector",
  "ConstantStructure",
  "ConstantContinuation",
  "ConstantExpression",
  "Variable",
  "Function",
  "Program",
  "Block",
  "BreakOp",
  "ContinueOp",
  "ReturnOp",
  "PInfOp",
  "NInfOp",
  "NaNOp",
  "ReferenceOp",
  "ResultOp",
  "ThrowOp",
  "NotOp",
  "NegateOp",
  "ComplementOp",
  "PreIncrOp",
  "PostIncrOp",
  "PreDecrOp",
  "PostDecrOp",
  "SizeOf",
  "IsPInfOp",
  "IsNInfOp",
  "IsNanOp",
  "TruncOp",
  "RoundOp",
  "FloorOp",
  "CeilingOp",
  "LogEOp",
  "Log2Op",
  "Log10Op",
  "SquareRootOp",
  "CubeRootOp",
  "FactorialOp",
  "LoadOp",
  "AllocateOp",
  "DeallocateOp",
  "AutoVarOp",
  "TellOp",
  "CloseOp",
  "LengthOp",
  "AddOp",
  "SubtractOp",
  "MultiplyOp",
  "DivideOp",
  "ModuloOp",
  "BAndOp",
  "BOrOp",
  "BXorOp",
  "BNorOp",
  "AndOp",
  "OrOp",
  "NorOp",
  "XorOp",
  "LessThanOp",
  "GreaterThanOp",
  "LessEqualOp",
  "GreaterEqualOp",
  "EqualityOp",
  "InequalityOp",
  "PowerOp",
  "RootOp",
  "GCDOp",
  "LCMOp",
  "ReallocateOp",
  "StoreOp",
  "WhileOp",
  "UnlessOp",
  "UntilOp",
  "OpenOp",
  "ReadOp",
  "WriteOp",
  "CreateContOp",
  "SelectOp",
  "StrInsertOp",
  "StrEraseOp",
  "StrReplaceOp",
  "PositionOp",
  "LoopOp",
  "CallOp",
  "InvokeOp",
  "DispatchOp",
  "CallWithContOp",
  "IndexOp",
  "SwitchOp",
  "**INVALID**",
  "**INVALID**",
  "**INVALID**",
  "**INVALID**",
  "**INVALID**",
  "**INVALID**",
  "**INVALID**",
  "**INVALID**",
  "**INVALID**",
  "**INVALID**",
};

void showNodeIds()
{
  unsigned rowcount = 4;
  for (unsigned i = FirstNodeID; i <= LastNodeID; i++) {
    if (rowcount % 4 == 0)
      std::cout << "\n";
    std::cout << "  " << i << ":" << NodeIDStrs[i];
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

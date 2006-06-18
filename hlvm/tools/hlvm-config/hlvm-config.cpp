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
  "Documentable",
  "Bundle",
  "Import",
  "VoidType",
  "BooleanType",
  "CharacterType",
  "OctetType",
  "UInt8Type",
  "UInt16Type",
  "UInt32Type",
  "UInt64Type",
  "UInt128Type",
  "SInt8Type",
  "SInt16Type",
  "SInt32Type",
  "SInt64Type",
  "SInt128Type",
  "Float32Type",
  "Float44Type",
  "Float64Type",
  "Float80Type",
  "Float128Type",
  "AnyType",
  "StringType",
  "IntegerType",
  "RangeType",
  "EnumerationType",
  "RealType",
  "RationalType",
  "OpaqueType",
  "AliasType",
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
  "ConstantBoolean",
  "ConstantInteger",
  "ConstantReal",
  "ConstantString",
  "ConstantAggregate",
  "ConstantExpression",
  "SizeOf",
  "Variable",
  "Function",
  "Program",
  "Block",
  "BreakOp",
  "ContinueOp",
  "PInfOp",
  "NInfOp",
  "NaNOp",
  "ReferenceOp",
  "ConstantReferenceOp",
  "NoOperator",
  "ReturnOp",
  "ThrowOp",
  "NotOp",
  "NegateOp",
  "ComplementOp",
  "PreIncrOp",
  "PostIncrOp",
  "PreDecrOp",
  "PostDecrOp",
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
};


int 
main(int argc, char**argv) 
{
  cl::ParseCommandLineOptions(argc, argv, 
    "hlvm-config: HLVM Configuration Utility\n");

  if (ShowNodeIds)
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
    std::cout << "PrimitiveTypes: " 
              << FirstPrimitiveTypeID << " -> " 
              << LastPrimitiveTypeID << "\n";
    std::cout << "SimpleTypeID: " 
              << FirstSimpleTypeID << " -> " 
              << LastSimpleTypeID << "\n";
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
    std::cout << "LinkageItems: " 
              << FirstLinkageItemID << " -> "
              << LastLinkageItemID << "\n";
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
    if (sizeof(NodeIDStrs)/sizeof(NodeIDStrs[0]) != NumNodeIDs)
      std::cout << "\n**##!! NodeIDStrs Out Of Date !!##**\n";

  }
  return 0;
}

//===-- LLVM Code Generation Interface --------------------------*- C++ -*-===//
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
/// @file hlvm/CodeGen/LLVMGenerator.h
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/05/26
/// @since 0.1.0
/// @brief Declares the interface for generating code with LLVM
//===----------------------------------------------------------------------===//

#ifndef HLVM_CODEGEN_LLVM_LLVMGENERATOR_H
#define HLVM_CODEGEN_LLVM_LLVMGENERATOR_H

#include <ostream>

namespace llvm {
  class Module;
}

namespace hlvm 
{
  class AST;

  /// Convert an Abstract Syntax Tree into LLVM bytecode written on the output 
  /// stream.
  bool generateBytecode(AST* input, std::ostream& output, std::string& ErrMsg);

  /// Convert an Abstract Syntax Tree into LLVM assembly written on the output
  /// stream.
  bool generateAssembly(AST* input, std::ostream& output, std::string& ErrMsg);

  /// Convert an Abstract Syntax Tree into an LLVM Module
  llvm::Module* generateModule(AST* input, std::string& ErrMsg );

} // hlvm
#endif

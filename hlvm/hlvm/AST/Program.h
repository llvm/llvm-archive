//===-- AST Program Class ---------------------------------------*- C++ -*-===//
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
/// @file hlvm/AST/Program.h
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::Program
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_PROGRAM_H
#define HLVM_AST_PROGRAM_H

#include <hlvm/AST/Function.h>

namespace hlvm 
{

class Block; // Forward declare
class SignatureType;  // Forward declare

/// This class represents a Program in the HLVM Abstract Syntax Tree.  
/// A Program is a function with a specific set of arguments. It represents
/// a starting point for any execution. To be executable, a Bundle must have
/// at least one Program node in it. The Program node is simply introduced
/// to ensure the signature of the function is correct and to serve as a way
/// to identify Program's quickly.
/// @brief HLVM AST Function Node
class Program : public Function
{
  /// @name Constructors
  /// @{
  public:
    Program() : Function(ProgramID) 
    { setSignature(SignatureTy); setLinkageKind(ExternalLinkage); }
    virtual ~Program();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const Program*) { return true; }
    static inline bool classof(const Function*F) { return F->isProgram(); }
    static inline bool classof(const Node* N) { return N->isProgram(); }

  /// @}
  /// @name Data
  /// @{
  private:
    static SignatureType* SignatureTy; ///< The signature for programs
    LinkageItem::setLinkageKind;
  /// @}
  friend class AST;
};

} // hlvm
#endif

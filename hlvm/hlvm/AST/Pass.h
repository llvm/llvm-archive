//===-- AST Pass Classes ----------------------------------------*- C++ -*-===//
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
/// @file hlvm/AST/Pass.h
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/18
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::Pass
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_PASS_H
#define HLVM_AST_PASS_H

namespace hlvm 
{

class Block;
class Bundle;
class Function;
class Node;
class Program;
class Operator;
class Type;
/// This class provides an abstract interface to Pass execution. This class
/// is meant to be subclassed and the various "handle" methods overriden to
/// gain access to the information in the AST.
/// @brief HLVM AST Abstract Pass
class Pass 
{
  /// @name Types
  /// @{
  public:
    /// Or these together and pass to the constructor to indicate which
    /// kinds of things you are interested in.
    enum PassInterest {
      All_Interest = 0,           ///< Pass is interested in everything
      Block_Interest = 1,         ///< Pass is interested in Blocks
      Operator_Interest = 2,      ///< Pass is interested in Operators
      Function_Interest = 4,      ///< Pass is interested in Functions
      Program_Interest = 8,       ///< Pass is interested in Programs
      Type_Interest = 16          ///< Pass is interested in Types
    };
  /// @}
  /// @name Constructors
  /// @{
  protected:
    Pass(int interest);
  public:
    virtual ~Pass();

  /// @}
  /// @name Handlers
  /// @{
  public:
    /// Handle any kind of node. Subclasses should override this; the default
    /// implementation does nothing. This handler is only called if the
    /// interest is set to 0 (interested in everything). It is left to the
    /// subclass to disambiguate the Node.
    virtual void handle(Node* n);

  /// @}
  /// @name Invocators
  /// @{
  public:
    /// Run the pass with the default interest given during construction.
    void run();

    /// Run the pass with the interest specified
    void runWithInterest(int interest);

  /// @}
  /// @name Data
  /// @{
  protected:
    int interest;
  /// @}
};

} // hlvm
#endif

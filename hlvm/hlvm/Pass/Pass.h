//===-- Pass Interface Class ------------------------------------*- C++ -*-===//
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
/// @file hlvm/Pass/Pass.h
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/05/18
/// @since 0.1.0
/// @brief Declares the class hlvm::Pass::Pass
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_PASS_H
#define HLVM_AST_PASS_H

namespace hlvm 
{
class Node;
class AST;

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
    enum InterestKinds {
      All_Interest = 0,           ///< Pass is interested in everything
      Block_Interest = 1,         ///< Pass is interested in Blocks
      Operator_Interest = 2,      ///< Pass is interested in Operators
      Function_Interest = 4,      ///< Pass is interested in Functions
      Program_Interest = 8,       ///< Pass is interested in Programs
      Type_Interest = 16,         ///< Pass is interested in Types
      Variable_Interest = 32      ///< Pass is interested in Variables
    };

    /// Specifies the kinds of traversals that a pass can request. The
    /// PassManager always walks the tree in a depth-first search (DFS) and it
    /// can call the pass either when it arrives at the node (preorder) on the
    /// way down, or when it leaves the node (postorder) on the way up. A pass
    /// can also specify that it wants both pre-order and post-order traversal.
    enum TraversalKinds {
      NoTraversal = 0,             ///< Pass doesn't want to be called!
      PreOrderTraversal = 1,       ///< Call pass on the way down the DFS walk
      PostOrderTraversal = 2,      ///< Call pass on the way up the DFS walk
      PreAndPostOrderTraversal = 3 ///< Call pass on both the way down and up
    };

  /// @}
  /// @name Constructors
  /// @{
  protected:
    Pass(int i, TraversalKinds m) : interest_(i), mode_(m), passed_(true) {}
  public:
    virtual ~Pass();

    /// Instantiate the standard passes
    static Pass* new_ValidatePass();
    static Pass* new_ResolveTypesPass();

  /// @}
  /// @name Handlers
  /// @{
  public:
    /// Handle initialization. This is called before any other handle method
    /// is called. Default implementation does nothing
    virtual void handleInitialize();

    /// Handle any kind of node. Subclasses should override this; the default
    /// implementation does nothing. This handler is only called if the
    /// interest is set to 0 (interested in everything). It is left to the
    /// subclass to disambiguate the Node.
    virtual void handle(Node* n, TraversalKinds mode) = 0;

    /// Handle termination. This is called after all other handle methods
    /// are called. Default implementation does nothing
    virtual void handleTerminate();

  /// @}
  /// @name Accessors
  /// @{
  public:
    int mode()     const { return mode_; }
    int interest() const { return interest_; }
    int passed()   const { return passed_; }

  /// @}
  /// @name Data
  /// @{
  private:
    int interest_;
    TraversalKinds mode_;
  protected:
    bool passed_;
  /// @}
};

class PassManager 
{
  protected:
    PassManager() {}
  public:
    virtual ~PassManager();
    static  PassManager* create();
    virtual void addPass(Pass* p) = 0;
    virtual void runOn(AST* tree) = 0;
};

bool validate(AST* tree);
} // hlvm
#endif

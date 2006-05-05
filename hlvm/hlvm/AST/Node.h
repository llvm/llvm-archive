//
// Copyright (C) 2006 HLVM Group. All Rights Reserved.
//
// This program is open source software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License (GPL) as published by
// the Free Software Foundation; either version 2 of the License, or (at your
// option) any later version. You should have received a copy of the GPL in a
// file named COPYING that was included with this program; if not, you can
// obtain a copy of the license through the Internet at http://www.fsf.org/
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
////////////////////////////////////////////////////////////////////////////////
/// @file hlvm/AST/Node.h
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::Node
////////////////////////////////////////////////////////////////////////////////

#ifndef HLVM_AST_NODE_H
#define HLVM_AST_NODE_H

#include <llvm/Support/Casting.h>
#include <vector>
#include <string>

/// This namespace is for all HLVM software. It ensures that HLVM software does
/// not collide with any other software. Hopefully HLVM is not a namespace used
/// elsewhere. 
namespace hlvm
{
/// This namespace contains all the AST (Abstract Syntax Tree) module code. All
/// node types of the AST are declared in this namespace.
namespace AST
{
  /// This enumeration is used to identify a specific type
  enum NodeIDs {
    // Types
    VoidTypeID = 0,     ///< The Void Type
    IntegerTypeID,      ///< The Integer Type
    RangeTypeID,        ///< The Range Type
    RealTypeID,         ///< The Real Number Type
    PointerTypeID,      ///< The Pointer Type
    ArrayTypeID,        ///< The Array Type
    VectorTypeID,       ///< The Vector Type
    StructureTypeID,    ///< The Structure Type
    SignatureTypeID,    ///< The Function Signature Type

    // Containers
    BundleID,           ///< The Bundle Node
    FunctionID,         ///< The Function Node

    // Declarations
    VariableID,         ///< The Variable Node

    // Enumeration Limits
    NumNodeIDs,         ///< The number of type identifiers in the enum
    FirstPrimitiveTypeID = VoidTypeID,
    LastPrimitiveTypeID  = RealTypeID,
    FirstContainerTypeID = PointerTypeID,
    LastContainerTypeID  = SignatureTypeID
  };

  class Type;

  /// A NamedType is simply a pair involving a name and a pointer to a Type.
  /// This is so frequently needed, it is declared here for convenience.
  typedef std::pair<std::string,Type*> NamedType;

  /// This class is the base class of HLVM Abstract Syntax Tree (AST). All 
  /// other AST nodes are subclasses of this class.
  /// @brief Abstract base class of all HLVM AST node classes
  class Node
  {
    /// @name Constructors
    /// @{
    public:
      Node(NodeIDs id, Node* parent = 0, const std::string& name = "") 
        : id_(id), parent_(parent), name_(name), kids_() {}
      virtual ~Node();
#ifndef _NDEBUG
      virtual void dump() const;
#endif

    /// @}
    /// @name Accessors
    /// @{
    public:
      inline bool isType() const { 
        return id_ <= FirstPrimitiveTypeID && id_ >= LastContainerTypeID;
      }
      inline bool isBundle() const { return id_ == BundleID; }
      inline bool isFunction() const { return id_ == FunctionID; }
      inline bool isVariable() const { return id_ == VariableID; }
      static inline bool classof(const Node*) { return true; }

    /// @}
    /// @name Data
    /// @{
    protected:
      NodeIDs id_;              ///< Identification of the node kind.
      Node* parent_;            ///< The node that owns this node.
      std::string name_;        ///< The name of this node.
      std::vector<Node> kids_;  ///< The vector of children nodes.
    /// @}
  };
} // AST
} // hlvm
#endif

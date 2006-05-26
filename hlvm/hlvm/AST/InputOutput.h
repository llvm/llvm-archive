//===-- AST Input/Output Operators ------------------------------*- C++ -*-===//
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
/// @file hlvm/AST/InputOutput.h
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/05/25
/// @since 0.1.0
/// @brief Declares the AST Input/Output classes (Open, Close, Write, Read,etc.)
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_INPUTOUTPUT_H
#define HLVM_AST_INPUTOUTPUT_H

#include <hlvm/AST/Operator.h>

namespace hlvm 
{

/// This node type represents a stream open operation. Streams can be files,
/// sockets, or any other kind of sequential or random access media. The URL
/// provided to the operator as its first operand determines what will get 
/// opened and how.
/// @brief HLVM AST Open Stream Operator
class OpenOp : public UnaryOperator
{
  /// @name Constructors
  /// @{
  public:
    static OpenOp* create();

  protected:
    OpenOp() : UnaryOperator(OpenOpID)  {}
    virtual ~OpenOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const OpenOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(OpenOpID); }

  /// @}
  friend class AST;
};

/// This node type represents a stream close operation. The operand must be a
/// value returned by the stream open operation. The associated stream is 
/// flushed of data and closed permanently.
/// @brief HLVM AST Close Stream Operator
class CloseOp : public UnaryOperator
{
  /// @name Constructors
  /// @{
  public:
    static CloseOp* create();

  protected:
    CloseOp() : UnaryOperator(CloseOpID)  {}
    virtual ~CloseOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const CloseOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(CloseOpID); }

  /// @}
  friend class AST;
};

/// This node type represents a stream write operation. There are three 
/// operands: [1] the stream to write, [2] the data buffer to write, [3] the
/// length of the data buffer.
/// @brief HLVM AST Write Stream Operator
class WriteOp : public TernaryOperator
{
  /// @name Constructors
  /// @{
  public:
    static WriteOp* create();

  protected:
    WriteOp() : TernaryOperator(WriteOpID)  {}
    virtual ~WriteOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const WriteOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(WriteOpID); }

  /// @}
  friend class AST;
};

} // hlvm 
#endif

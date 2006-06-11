//===-- AST Input/Output Operators Interface --------------------*- C++ -*-===//
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

/// This class provides an Abstract Syntax Tree node that represents a stream 
/// open operation. Streams can be files, sockets, or any other kind of 
/// sequential media. This operator takes one operand which is a URI indicating
/// what should be opened. The URI specifies the resource and any parameters to
/// indicate how the resource should be opened. The operator returns a value of
/// type StreamType which is a built-in runtime defined opaque type.
/// @see CloseOp
/// @see StreamType
/// @brief AST Open Stream Operator
class OpenOp : public UnaryOperator
{
  /// @name Constructors
  /// @{
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

/// This class provides an Abstract Syntax Tree node that represents a stream 
/// close operation. This operator takes one operand which must be the stream
/// to be closed. The operand's type is StreamType which must have been 
/// previously returned by the OpenOp operator. The operator causes all 
/// buffers written to the stream to be flushed. After the operator completes, 
/// the stream will no longer be available for input/output operations. This 
/// operator returns a value of type VoidType.
/// @see OpenOp
/// @see StreamType
/// @see VoidType
/// @brief AST Close Stream Operator
class CloseOp : public UnaryOperator
{
  /// @name Constructors
  /// @{
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

/// This class provides an Abstract Syntax Tree node that represents a stream 
/// read operation. There are two operands: [1] the stream from which data will
/// be read and [2] the BufferType or TextType into which the data should be
/// placed. The first operand must be of type StreamType and previously opened
/// with the OpenOp operator. The second operand may be of BufferType or 
/// TextType. The value of the second operand will be replaced with the data
/// read. The previous value of the second operand, if any, is consequently
/// discarded by this operation.
/// @see OpenOp
/// @see StreamType
/// @see BufferType
/// @see TextType
/// @brief AST Stream Write Operator
class ReadOp : public BinaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    ReadOp() : BinaryOperator(ReadOpID)  {}
    virtual ~ReadOp();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const ReadOp*) { return true; }
    static inline bool classof(const Node* N) { return N->is(ReadOpID); }

  /// @}
  friend class AST;
};

/// This class provides an Asbstract Syntax Tree node that represents a stream 
/// write operation. There are two operands: [1] the stream to write, and [2]
/// the data buffer or text to write. The second operands can be either a
/// BufferType object or a TextType object. The entire buffer or text value is
/// written to the stream. The result of this operator is an integer value of
/// type u64 which indicates the number of bytes actually written to the stream.
/// @see StreamType
/// @see BufferType
/// @see TextType
/// @brief AST Stream Write Operator
class WriteOp : public BinaryOperator
{
  /// @name Constructors
  /// @{
  protected:
    WriteOp() : BinaryOperator(WriteOpID)  {}
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

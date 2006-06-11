//===-- AST Runtime Type Interfaces -----------------------------*- C++ -*-===//
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
/// @file hlvm/AST/RuntimeType.h   
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares types for the objects that the runtime manipulates.
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_RUNTIMETYPE_H  
#define HLVM_AST_RUNTIMETYPE_H  

#include <hlvm/AST/Type.h>

namespace hlvm 
{

/// This class provides an Abstract Syntax Tree base class for the various types
/// that are manipulated by the HLVM runtime.  In general, a RuntimeType is
/// simply a pointer (handle) to some internal data structure of the runtime.
/// The subclasses of RuntimeType exist to differentiate between the different
/// kinds of objects the runtime manipulates.  The Runtime then is able to 
/// define what the actualy types are without breaking compatibility across
/// releases. Runtime types are disginguished by their names.
class RuntimeType : public Type
{
  /// @name Constructors
  /// @{
  protected:
    RuntimeType(NodeIDs id, const std::string& n) : Type(id) { setName(n); }
    virtual ~RuntimeType();

  /// @}
  /// @name Accessors
  /// @{
  public:
    virtual const char* getPrimitiveName() const; // asserting override
    // Methods to support type inquiry via isa, cast, dyn_cast
    static inline bool classof(const RuntimeType*) { return true; }
    static inline bool classof(const Type* T) { return T->isRuntimeType(); }
  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a Text Type
/// object. TextType objects are unicode strings of either variable or constant
/// value. Internall, the encoding of a TextType string is UTF-8. However, the
/// string can be converted to any of a number of Unicode encodings.
/// @brief AST Unicode Text Type
class TextType : public RuntimeType
{
  /// @name Constructors
  /// @{
  protected:
    TextType() : RuntimeType(TextTypeID,"text") {}
    virtual ~TextType();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const TextType*) { return true; }
    static inline bool classof(const Node* N) 
      { return N->is(TextTypeID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a stream 
/// type. A StreamType is used for input and output. The StreamType represents
/// a handle that the HLVM runtime uses to identify the source of the program's
/// input or the destination of a program's output.
/// @brief AST Input Output Stream Type
class StreamType : public RuntimeType
{
  /// @name Constructors
  /// @{
  protected:
    StreamType() : RuntimeType(StreamTypeID,"hlvm_stream") {}
    virtual ~StreamType();
  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const StreamType*) { return true; }
    static inline bool classof(const Node* N) 
      { return N->is(StreamTypeID); }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a data 
/// buffer in the HLVM runtime. A buffer is logically the combination of an 
/// area of memory and an integer length for the size of that memory area.
/// @brief AST Buffer Type
class BufferType : public RuntimeType
{
  /// @name Constructors
  /// @{
  protected:
    BufferType() : RuntimeType(BufferTypeID,"hlvm_buffer") {}
    virtual ~BufferType();
  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const BufferType*) { return true; }
    static inline bool classof(const Node* N) 
      { return N->is(BufferTypeID); }

  /// @}
  friend class AST;
};

} // hlvm
#endif

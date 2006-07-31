//===-- AST Linkables Interface ---------------------------------*- C++ -*-===//
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
/// @file hlvm/AST/Linkables.h
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/06/10
/// @since 0.1.0
/// @brief Declares the interface to all subclasses of Linkable
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_LINKABLE_H
#define HLVM_AST_LINKABLE_H

#include <hlvm/AST/Constants.h>
#include <hlvm/AST/ContainerType.h>
#include <hlvm/AST/Block.h>

namespace hlvm 
{

/// This enumeration is used to specify the kinds of linkage that are
/// permitted for a Linkable.
/// @brief Enumeration of ways to link bundles
enum LinkageKinds {
  ExternalLinkage   = 1, ///< Externally visible item
  LinkOnceLinkage   = 2, ///< Keep one copy of item when linking (inline)
  WeakLinkage       = 3, ///< Keep one copy of item when linking (weak)
  AppendingLinkage  = 4, ///< Append item to an array of similar items
  InternalLinkage   = 5  ///< Rename collisions when linking (static funcs)
};

/// This class provides an Abstract Syntax Tree node that represents an item
/// which can be linked with other Bundles. Linkabe is an abstract base 
/// class and cannot be instantiated. All Linkables are Constant values
/// because they represents a runtime value that is a constant address. The
/// value pointed to by the Linkable may be mutable or immutable depending
/// on its type and options.  As the name suggests, Linkables participate
/// in linkage. A Bundle referring to a name in another Bundle will only link
/// with a Linkable and nothing else. There are several ways in which 
/// Linkables can be linked together, specified by the LinkageKinds value.
/// @see LinkageKinds
/// @see Bundle
/// @see Constant
/// @brief AST Bundle Node
class Linkable : public Constant
{
  /// @name Constructors
  /// @{
  protected:
    Linkable( NodeIDs id ) : Constant(id) { setLinkageKind(InternalLinkage);}
    virtual ~Linkable();

  /// @}
  /// @name Accessors
  /// @{
  public:
    inline LinkageKinds getLinkageKind() const { 
      return LinkageKinds(flags & 0x0007); }
    static inline bool classof(const Linkable*) { return true; }
    static inline bool classof(const Node* N) { return N->isLinkable(); }

  /// @}
  /// @name Mutators
  /// @{
    void setLinkageKind(LinkageKinds k) { 
      flags &= 0xFFF8; flags |= uint16_t(k); 
    }

  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a 
/// global Variable.  A Variable can only be declared as a component of a 
/// Bundle. It is visible throughout the Bundle that declares it and may 
/// be a candidate for linkage with other Bundles. A Variable is a storage 
/// location, with an address, of a specific type. Global variables may have
/// a constant value in which case HLVM will ensure that the value of the
/// global variable is immutable. Variables can be of any type.
/// @see Linkable
/// @see Bundle
/// @brief AST Variable Node
class Variable : public Linkable
{
  /// @name Constructors
  /// @{
  protected:
    Variable() : Linkable(VariableID), init(0) {}
    virtual ~Variable();

  /// @}
  /// @name Accessors
  /// @{
  public:
    bool isConstant() const { return flags & 0x0008; }
    Constant* getInitializer() const { return init; }
    bool hasInitializer() const { return init != 0; }
    bool isZeroInitialized() const { return init == 0; }
    static inline bool classof(const Variable*) { return true; }
    static inline bool classof(const Node* N) { return N->is(VariableID); }

  /// @}
  /// @name Mutators
  /// @{
  public:
    void setIsConstant(bool v) { flags |= 0x0008; }
    void setInitializer(Constant* C) { init = C; }

  /// @}
  /// @name Data
  /// @{
  protected:
    Constant* init;
  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents an argument
/// to a Function. An Argument is simply a Value that has a name. Arguments can
/// be used within a function's blocks to access the value of the argument.
/// @see Value
/// @see Function
/// @brief AST Argument Node
class Argument : public Value
{
  /// @name Constructors
  /// @{
  protected:
    Argument() : Value(ArgumentID), name() {}
    virtual ~Argument();

  /// @}
  /// @name Accessors
  /// @{
  public:
    const std::string& getName() const { return name; }
    /// Return the 1-based index of this in this function it belongs to. 
    /// If this is not an argument of any function, returns 0;
    unsigned getArgNum() const;
    static inline bool classof(const Argument*) { return true; }
    static inline bool classof(const Node* N) { return N->is(ArgumentID); }

  /// @}
  /// @name Mutators
  /// @{
  public:
    void setName(const std::string& nm) { name = nm; }

  /// @}
  /// @name Data
  /// @{
  protected:
    std::string name;
  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a Function.
/// A Function is a callable block of code that accepts parameters and 
/// returns a result.  This is the basic unit of code in HLVM. A Function
/// has a name, a set of formal arguments, a return type, and, optionally, a 
/// Block of code to execute. The name of a function is used for linking 
/// purposes. The formal arguments and return type are encapsulated in the
/// Function's associated SignatureType. If a Block is associated with the
/// Function then the function is defined and the Block defines the computation
/// the Function provides. If a Block is not associated with the Function, then
/// the function is undefined and serves as a reference to a function in another
/// Bundle.
/// @see Block
/// @see Bundle
/// @see SignatureType
/// @brief AST Function Node
class Function : public Linkable
{
  /// @name Types
  /// @{
  public:
    typedef std::vector<Argument*> ArgumentList;
    typedef ArgumentList::iterator iterator;
    typedef ArgumentList::const_iterator const_iterator;

  /// @}
  /// @name Constructors
  /// @{
  protected:
    Function(NodeIDs id = FunctionID) : Linkable(id), block(0), args() {}
    virtual ~Function();

  /// @}
  /// @name Accessors
  /// @{
  public:
    bool hasBlock() const { return block != 0; }
    Block* getBlock() const { return block; }
    const SignatureType* getSignature() const 
      { return static_cast<const SignatureType*>(type); }
    const Type* getResultType() const 
      { return getSignature()->getResultType();}
    Argument* getArgument(const std::string& name) const;
    Argument* getArgument(unsigned argnum) const;

    /// Return the 1-based index of the \p arg in this function. If \p arg
    /// is not an argument of this function, returns 0;
    unsigned getArgNum(const Argument* arg) const;

    static inline bool classof(const Function*) { return true; }
    static inline bool classof(const Node* N) { return N->isFunction(); }

  /// @}
  /// @name Argument Iteration
  /// @{
  public:
    iterator         begin()       { return args.begin(); }
    const_iterator   begin() const { return args.begin(); }
    iterator         end  ()       { return args.end();   }
    const_iterator   end  () const { return args.end();   }
    size_t           size () const { return args.size();  }
    bool             empty() const { return args.empty(); }
    Argument*        front()       { return args.front(); }
    const Argument*  front() const { return args.front(); }
    Argument*        back()        { return args.back();  }
    const Argument*  back()  const { return args.back();  }

  /// @}
  /// @name Mutators
  /// @{
  public:
    virtual void insertChild(Node* kid);
    virtual void removeChild(Node* kid);
    void setBlock(Block* blk) { blk->setParent(this); }
    void addArgument(Argument* arg) { args.push_back(arg); }
    virtual void resolveTypeTo(const Type* from, const Type* to);

  /// @}
  /// @name Data
  /// @{
  protected:
    Block * block;                   ///< The code block to be executed
    ArgumentList args;
  /// @}
  friend class AST;
};

/// This class provides an Abstract Syntax Tree node that represents a Program 
/// in HLVM. A Program is an entry point for running HLVM programs. It is 
/// simply a Function with a specific signature. It represents the first
/// function to be executed by the runtime after option processing.  To be
/// executable, a Bundle must have at least one Program node in it. 
/// The Program node exists to simply ensure that the signature of the function
/// is correct and to serve as a way to identify Program's quickly.
/// @see Function
/// @see Bundle
/// @see SignatureType
/// @brief AST Program Node
class Program : public Function
{
  /// @name Constructors
  /// @{
  protected:
    Program() : Function(ProgramID) {}
    virtual ~Program();

  /// @}
  /// @name Accessors
  /// @{
  public:
    static inline bool classof(const Program*) { return true; }
    static inline bool classof(const Node* N) { return N->is(ProgramID); }

  /// @}
  /// @name Data
  /// @{
  private:
    Linkable::setLinkageKind;
  /// @}
  friend class AST;
};

} // end hlvm namespace
#endif

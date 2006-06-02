//===-- AST Locator Class ---------------------------------------*- C++ -*-===//
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
/// @file hlvm/AST/Locator.h
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::AST::Locator and friends
//===----------------------------------------------------------------------===//

#ifndef HLVM_AST_LOCATOR_H
#define HLVM_AST_LOCATOR_H

#include <hlvm/AST/URI.h>
#include <string>

namespace hlvm
{

/// This class is used to hold a source code location as a filename, line
/// number and column number. This is used for generating error messages and
/// for debugging support.
/// @brief Source location holder class.
class Locator
{
  /// @name Constructors
  protected:
    Locator() : SubclassID(0) {}
  /// @name Accessors
  /// @{
  public:
    virtual void getReference(std::string& ref) const = 0;
    virtual bool equals(const Locator& that) const = 0;
    bool operator==(const Locator& that) { return this->equals(that); }
    unsigned short id() const { return SubclassID; }
  /// @}
  protected:
    unsigned short SubclassID;
};

class URILocator : public Locator
{
  /// @name Constructors
  /// @{
  public:
    URILocator(const URI* u) : Locator(), uri(u) { SubclassID = 1; }

  /// @}
  /// @name Accessors
  /// @{
  public:
    virtual void getReference(std::string& ref) const;
    virtual bool equals(const Locator& that) const;

  /// @}
  /// @name Data
  /// @{
  protected:
    const URI* uri;
  /// @}
};

class LineLocator : public URILocator
{
  /// @name Constructors
  /// @{
  public:
    LineLocator(const URI* u, uint32_t l) : URILocator(u), line(l) {
      SubclassID = 2; 
    }

  /// @}
  /// @name Accessors
  /// @{
  public:
    virtual void getReference(std::string& ref) const;
    virtual bool equals(const Locator& that) const;

  /// @}
  /// @name Data
  /// @{
  protected:
    uint32_t line;           ///< Line number of source location
  /// @}
};

class LineColumnLocator : public LineLocator
{
  /// @name Constructors
  /// @{
  public:
    LineColumnLocator(const URI* u, uint32_t l, uint32_t c) 
      : LineLocator(u,l), col(c) { SubclassID = 3; }

  /// @}
  /// @name Accessors
  /// @{
  public:
    virtual void getReference(std::string& ref) const;
    virtual bool equals(const Locator& that) const;

  /// @}
  /// @name Data
  /// @{
  protected:
    uint32_t col;            ///< Column number of source location
  /// @}
};

class RangeLocator : public LineColumnLocator
{
  /// @name Constructors
  /// @{
  public:
    RangeLocator(const URI* u, uint32_t l, uint32_t c, uint32_t l2, uint32_t c2)
      : LineColumnLocator(u,l,c), line2(l2), col2(c2) { SubclassID = 4; }

  /// @}
  /// @name Accessors
  /// @{
  public:
    virtual void getReference(std::string& ref) const;
    virtual bool equals(const Locator& that) const;

  /// @}
  /// @name Data
  /// @{
  protected:
    uint32_t line2;           ///< Column number of source location
    uint32_t col2;            ///< Column number of source location
  /// @}
};

} // hlvm
#endif

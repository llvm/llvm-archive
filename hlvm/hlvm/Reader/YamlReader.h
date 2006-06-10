//===-- AST Yaml Reader Interface -------------------------------*- C++ -*-===//
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
/// @file hlvm/Reader/YamlReader.h
/// @author Reid Spencer <rspencer@reidspencer.com> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::YamlReader.h
////////////////////////////////////////////////////////////////////////////////

#ifndef HLVM_YAML_READER_YAMLREADER_H
#define HLVM_YAML_READER_YAMLREADER_H

#include <llvm/System/Path.h>

namespace hlvm {

  /// This class provides an interface to reading HLVM's AST via a Yaml
  /// document.
  /// @brief Interface to Reading Yaml AST documents.
  class YamlReader
  {
    /// @name Constructors
    /// @{
    public:
      /// The constructor static method to create a YamlReader. This creates
      /// the correct sublcass of YamlReader which is an implementation detail.
      static YamlReader* create();
    protected:
      YamlReader() {}

    /// @}
    /// @name Accessors
    /// @{
    public:

    /// @}
    /// @name Mutators
    /// @{
    public:
      void parse(const llvm::sys::Path& path);

    /// @}
    /// @name Data
    /// @{
    protected:
    /// @}
  };

} // hlvm
#endif

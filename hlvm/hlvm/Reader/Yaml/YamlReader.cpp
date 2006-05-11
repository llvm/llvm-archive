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
/// @file hlvm/Reader/Yaml/YamlReader.cpp
/// @author Reid Spencer <reid@hlvm.org> (original author)
/// @date 2006/05/04
/// @since 0.1.0
/// @brief Declares the class hlvm::YamlReader.cpp
////////////////////////////////////////////////////////////////////////////////

#include <hlvm/Reader/Yaml/YamlReader.h>
#include <llvm/System/MappedFile.h>
#include <syck.h>

namespace {

class YamlReaderImpl : public hlvm::YamlReader {
  public:
    YamlReaderImpl() {
      parser_ = syck_new_parser();
      syck_parser_handler(parser_, SyckNodeHandler(NodeHandler));
      syck_parser_error_handler(parser_, SyckErrorHandler(ErrorHandler));
      syck_parser_bad_anchor_handler(parser_, 
        SyckBadAnchorHandler(BadAnchorHandler));
    }

    static SYMID NodeHandler(SyckParser*, SyckNode* )
    {
      return 0;
    }

    static void ErrorHandler(SyckParser*, char*)
    {
    }

    static SyckNode* BadAnchorHandler(SyckParser*, char*)
    {
      return 0;
    }

    void parse(const llvm::sys::Path& path) {
      llvm::sys::MappedFile mapfile(path);
      mapfile.map();
      syck_parser_str(parser_, mapfile.charBase(), mapfile.size(),0);
      root_ = syck_parse(parser_);
    }

  private:
    SyckParser* parser_;
    SYMID root_;
};

} // anonymous

hlvm::YamlReader* hlvm::YamlReader::create() {
  return new YamlReaderImpl();
}

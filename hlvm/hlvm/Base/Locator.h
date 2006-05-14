#ifndef HLVM_BASE_LOCATOR_H
#define HLVM_BASE_LOCATOR_H

#include <llvm/ADT/StringExtras.h>

namespace hlvm {

  /// This class provides some data to locate a point in an XML file. It
  /// consists of a public id, system id (filename), line number and column
  /// number.
  /// @author Reid Spencer <rspencer@reidspencer.com>
  /// @date 2006/05/12
  /// @since 0.1.0
  /// @brief XML Locator
  class Locator
  {
  private:
    std::string pubid;
    std::string sysid;
    uint32_t line;	///< Line number of location
    uint32_t col;	///< Column number of location

  public:
    Locator() : pubid(), sysid(), line(0), col(0) {}
    Locator(
      std::string& p, 
      std::string& s, 
      uint32_t l, 
      uint32_t c = 0 
    ) 
      : pubid(p)
      , sysid(s)
      , line(l)
      , col(c)
    {}

    void set(
      const std::string& p = "", 
      const std::string& s = "", 
      uint32_t l = 0, 
      uint32_t c = 0 
    )
    {
      pubid = p; sysid = s; line = l; col = c;
    }
    void clear() { line = col = 0; pubid.clear(); sysid.clear(); }
    uint32_t getLine() const { return line; }
    void setLine(uint32_t l) { line = l; }
    uint32_t getColumn() const { return col; }
    void setColumn(uint32_t c ) { col = c; }
    const std::string& getPublicId() const { return pubid; } 
    const std::string& getSystemId() const { return sysid; }
    std::string toString() const
    {
      std::string result = getSystemId() + ": " + llvm::utostr(line);
      if (col)
        result += ": " + llvm::utostr(col);
      return result;
    }
  };
}

#endif

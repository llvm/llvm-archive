#include "HTMLMarkup.h"
#include <iostream>

class SimpleHTML : public HTMLMarkup {
private:
  std::ostream &os;

public:
  SimpleHTML(std::ostream &o) : os(o) {}

  void printHeader() { os << "<html><body><tt>\n"; }
  void printFooter() { os << "</tt></body></html>"; }

  void typeBegin() { os << "<font color=\"green\"><tt>"; }
  void typeEnd()   { os << "</tt></font>"; }

  void printKeyword(const std::string &s) { 
    os << "<font color=\"blue\"><tt><b>" << s
       << "</b></tt></font>";
  }

  void printBB(const std::string &s) { 
    os << "<font color=\"#cc0000\"><tt><b>" << s
       << ":</b></tt></font><br>";
  }
  
  void instrBegin() { os << "<tt>"; }
  void instrEnd()   { os << "</tt><br>\n"; }
};

HTMLMarkup *createSimpleHTMLMarkup(std::ostream &os) {
  return new SimpleHTML(os);
}

//----------------------------------------------------------------------------//

class CSSHTML : public HTMLMarkup {
private:
  std::ostream &os;

public:
  CSSHTML(std::ostream &o) : os(o) {}

  void printHeader() { os << "<html><body>\n"; }
  void printFooter() { os << "</body></html>"; }

  void typeBegin() { os << "<span class=\"llvm_type\">"; }
  void typeEnd()   { os << "</span>"; }

  void printKeyword(const std::string &s) {
    os << "<span class=\"llvm_keyword\">" << s
       << "</span>";
  }

  void printBB(const std::string &s) { 
    os << "<span class=\"llvm_bb\">" << s
       << ":</span><br>";
  }
  
  void instrBegin() { os << "<span class=\"llvm_instr\">"; }
  void instrEnd()   { os << "</span><br>\n"; }
};

HTMLMarkup *createCSSHTMLMarkup(std::ostream &os) {
  return new CSSHTML(os);
}

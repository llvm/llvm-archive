// Generic abstract HTML markup engine

#ifndef HTMLMARKUP_H
#define HTMLMARKUP_H

#include <ostream>
#include <string>

class HTMLMarkup {

 public:  
  virtual void printHeader() {};
  virtual void printFooter() {};

  virtual void typeBegin() = 0;
  virtual void typeEnd() = 0;

  virtual void printKeyword(const std::string &s) = 0;
  virtual void printBB(const std::string &s) = 0;
  
  virtual void instrBegin() = 0;
  virtual void instrEnd() = 0;
};


HTMLMarkup *createSimpleHTMLMarkup(std::ostream &os);

HTMLMarkup *createCSSHTMLMarkup(std::ostream &os);


#endif

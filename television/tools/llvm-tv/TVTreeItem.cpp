#include "TVTreeItem.h"
#include "llvm/Module.h"
#include "llvm/Type.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Assembly/CachedWriter.h"
#include "llvm/Assembly/Writer.h"
#include <wx/treectrl.h>
#include <cstdlib>
#include <sstream>
using namespace llvm;

static inline void htmlHeader(std::ostream &os) {
  os << "<html><body><tt>\n";
}

static inline void htmlFooter(std::ostream &os) {
  os << "</tt></body></html>";
}

static inline void htmlBB(std::ostream &os, const BasicBlock *BB) {
  os << "<font color=\"#cc0000\"><tt><b>" << BB->getName() 
     << ":</b></tt></font><br>";
}

static inline void htmlType(CachedWriter &cw, const Type* type) {
  cw << "<font color=\"green\"><b>" << type << "</b></font>";
}

static inline std::ostream&
wrapType(std::ostream &os, const std::string &word) {
  return os << "<font color=\"green\"><b>" << word << "</b></font>";
}

static inline std::ostream&
wrapKeyword(std::ostream &os, const std::string &word) {
  return os << "<font color=\"navy\"><b>" << word << "</b></font>";
}

static inline std::ostream&
wrapConstant(std::ostream &os, const std::string &c) {
  return os << "<font color=\"#770077\">" << c << "</font>";
}

// LLVM types
static const char* types[] = { 
  "void", "bool", "sbyte", "ubyte", "short", "ushort", "int", "uint",
  "long", "ulong", "float", "double", "type", "label", "opaque"
};

// LLVM keywords
static const char* keywords[] = { 
  // Arithmetic instructions
  "add", "sub", "mul", "div", "rem", "and", "or", "xor",
  "setne", "seteq", "setlt", "setgt", "setle", "setge",
  // Misc
  "phi", "call", "cast", "to", "select", "shl", "shr", "vaarg", "vanext",
  // Control flow
  "ret", "br", "switch", "invoke", "unwind",
  // Memory
  "malloc", "alloca", "free", "load", "store", "getelementptr",
  // Language constants/keywords
  "begin", "end", "true", "false", "zeroinitializer",
  "declare", "global", "constant", "const",
  "internal", "uninitialized", "external", "implementation",
  "linkonce", "weak", "appending",
  "null", "to", "except", "target", "endian", "pointersize",
  "big", "little", "volatile"
};


// Just use ostream to output instead of assembling into one string
std::ostream& stylizeTypesAndKeywords(std::ostream &os, std::string &str) {
  if (str == "") return os;

  // Prettify the instruction for HTML view
  for (unsigned i = 0; i != str.size(); ++i)
    if (str[i] == '\n') {                          // \n => <br>
      str[i] = '<';
      std::string br = "br>";
      str.insert(str.begin()+i+1, br.begin(), br.end());
      i += br.size();
    } else if (str[i] == '\t') {                   // \t => &nbsp;
      str[i] = '&';
      std::string nbsp = "nbsp; &nbsp; ";
      str.insert(str.begin()+i+1, nbsp.begin(), nbsp.end());
      i += nbsp.size();
    } else if (str[i] == ';') {                    // Delete comments!
      unsigned Idx = str.find('\n', i+1);          // Find end of line
      str.erase(str.begin()+i, str.begin()+Idx);
      --i;
    }

  os << "<tt>";

  // Tokenize and process
  unsigned prev = 0;
  bool done = false;
  for (unsigned i = 0, e = str.size(); i != e; ++i) {
    if (str[i] == ' ') {
      std::string token = str.substr(prev, i-prev);
      prev = i+1;
      done = false;
        
      // Wrap keywords
      for (unsigned k = 0, ke = sizeof(keywords)/sizeof(char*); k != ke; ++k)
        if (token == keywords[k]) {
          wrapKeyword(os, token);
          done = true;
          break;
        }

      if (done) { os << ' '; continue; }

      // Wrap types
      for (unsigned t = 0, te = sizeof(types)/sizeof(char*); t != te; ++t) {
        std::string type(types[t]);
        if (token.substr(0, type.size()) == type) {
          wrapType(os, token);
          done = true;
          break;
        }
      }

      if (done) { os << ' '; continue; }

      // Wrap constants
      if (strtol(token.c_str(), 0, 0))
        wrapConstant(os, token);
      else
        os << token;

      os << " ";
    }
  }
  // tack on the last segment
  return os << str.substr(prev, str.size()-prev) << "</tt>";
}

void TVTreeItemData::printFunctionHeader(Function *F) {
  std::ostream &os = cw.getStream();

  // print out function return type, name, and arguments
  os << "<tt>";
  if (F->isExternal ())
    wrapKeyword(os, "declare ");

  htmlType(cw, F->getReturnType());

  os << " " << F->getName() << "(";
  for (Function::aiterator arg = F->abegin(), ae = F->aend(); arg != ae; ++arg){
    htmlType(cw, arg->getType());
    os << " " << arg->getName();
    Function::aiterator next = arg;
    ++next;
    if (next != F->aend()) 
      os << ", ";
  }

  if (F->getFunctionType()->isVarArg()) {
    if (F->getFunctionType()->getNumParams()) os << ", ";
    os << "...";  // Output varargs portion of signature!
  }

  os << ")";
}

void TVTreeItemData::printFunction(Function *F) {
  printFunctionHeader(F);
  std::ostream &os = cw.getStream();

  if (!F->isExternal ())
    os << " {<br>";
  os << "</tt>";

  for (Function::iterator BB = F->begin(), BBe = F->end(); BB != BBe; ++BB) {
    htmlBB(os, BB);
    for (BasicBlock::iterator I = BB->begin(), Ie = BB->end(); I != Ie; ++I) {
      std::ostringstream oss;
      cw.setStream(oss);
      cw << &*I;
      std::string InstrVal = oss.str();
      stylizeTypesAndKeywords(os, InstrVal);
    }
  }
  cw.setStream(os);

  if (!F->isExternal ())
    os << "<tt>}</tt>";
  os << "<br>";
}

void TVTreeItemData::printModule(Module *M) {
  std::ostream &os = cw.getStream();
  htmlHeader(os);

  // Display target size (bits), endianness types
  std::ostringstream oss;
  oss << "target endian = "
      << (M->getEndianness() ? "little" : "big") << " \n ";
  oss << "target pointersize = "
      << (M->getPointerSize() ? "32" : "64") << "\n<br>";

  // Display globals
  for (Module::giterator G = M->gbegin(), Ge = M->gend(); G != Ge; ++G) {
    G->print(oss);
  }
  std::string str = oss.str();
  stylizeTypesAndKeywords(os, str);
  os << "<br>";

  // Display functions
  for (Module::iterator F = M->begin(), Fe = M->end(); F != Fe; ++F) {
    printFunctionHeader(F);
    os << "<br>";
  }

  htmlFooter(os);
}

void TVTreeModuleItem::print(std::ostream &os) {
  myModule->print(os);
}

void TVTreeModuleItem::printHTML(std::ostream &os) {
  if (myModule) {
    cw.setStream(os);
    cw.setModule(myModule);
    cw << CachedWriter::SymTypeOn;
    printModule(myModule);
  }
}


void TVTreeFunctionItem::print(std::ostream &os) { 
  myFunc->print(os);
}

void TVTreeFunctionItem::printHTML(std::ostream &os) {
  if (myFunc) {
    cw.setStream(os);
    cw.setModule(myFunc->getParent());
    cw << CachedWriter::SymTypeOn;
    htmlHeader(os);
    printFunction(myFunc); 
    htmlFooter(os);
  }
}


Module* TVTreeFunctionItem::getModule() {
  return myFunc ? myFunc->getParent() : 0; 
}

#include "TVTreeItem.h"
#include "llvm/Module.h"
#include "llvm/Type.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Assembly/Writer.h"
#include "wx/treectrl.h"
#include <cstdlib>
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

static inline void htmlType(std::ostream &os, const Type* type,
                            const Module *M) {
  os << "<font color=\"green\"><b>";
  WriteTypeSymbolic (os, type, M);
  os << "</b></font>";
}

static inline std::string wrapType(const std::string &word) {
  return ("<font color=\"green\"><b>" + word + "</b></font>");
}

static inline std::string wrapKeyword(const std::string &word) {
  return ("<font color=\"navy\"><b>" + word + "</b></font>");
}

static inline std::string wrapConstant(const std::string &c) {
  return ("<font color=\"#770077\">" + c + "</font>");
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
  "begin", "end", "true", "false",
  "declare", "global", "constant", "const",
  "internal", "uninitialized", "external", "implementation",
  "linkonce", "weak", "appending",
  "null", "to", "except", "target", "endian", "pointersize",
  "big", "little", "volatile"
};


static inline std::string stylizeTypesAndKeywords(std::string &str) {
  if (str == "") return "";

  // Tokenize
  std::vector<std::string> tokens;
  unsigned prev = 0;
  for (unsigned i = 0, e = str.size(); i != e; ++i) {
    if (str[i] == ' ') {
      tokens.push_back(str.substr(prev, i-prev));
      prev = i+1;
    }
  }
  // tack on the last segment
  tokens.push_back(str.substr(prev, str.size()-prev));

#if 0
  std::cerr << "\n";
  for (unsigned i=0, e = tokens.size(); i!=e; ++i)
    std::cerr << "[" << tokens[i] << "], ";
  std::cerr << "\n";
#endif
  
  for (unsigned i = 0, e = tokens.size(); i != e; ++i)
  {
    // "Wrap" keywords
    for (unsigned k = 0, ke = sizeof(keywords)/sizeof(char*); k != ke; ++k)
      if (tokens[i] == keywords[k]) {
        tokens[i] = wrapKeyword(tokens[i]);
        break;
      }

    // "Wrap" types
    for (unsigned t = 0, te = sizeof(types)/sizeof(char*); t != te; ++t) {
      std::string type(types[t]);
      if (tokens[i].substr(0, type.size()) == type) {
        tokens[i] = wrapType(tokens[i]);
        break;
      }
    }

    // "Wrap" constants
    if (strtol(tokens[i].c_str(), 0, 0))
      tokens[i] = wrapConstant(tokens[i]);

  }

#if 0
  std::cerr << "\n";
  for (unsigned i=0, e = tokens.size(); i!=e; ++i)
    std::cerr << "[" << tokens[i] << "], ";
  std::cerr << "\n";
#endif

  // Assemble back into one
  std::string styled = "";
  for (unsigned i = 0, e = tokens.size(); i != e; ++i)
    styled += tokens[i] + " ";
  return styled;
}

void TVTreeItemData::printFunction(Function *F, std::ostream &os) {
  // print out function return type, name, and arguments
  os << "<tt>";
  if (F->isExternal ())
    os << "declare ";
  htmlType(os, F->getReturnType(), F->getParent ());
  os << " " << F->getName() << "(";
  for (Function::aiterator arg = F->abegin(), ae = F->aend(); arg != ae; ++arg){
    htmlType(os, arg->getType(), F->getParent ());
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
  if (!F->isExternal ())
    os << " {<br>";
  os << "</tt>";

  for (Function::iterator BB = F->begin(), BBe = F->end(); BB != BBe; ++BB) {
    htmlBB(os, BB);
    for (BasicBlock::iterator I = BB->begin(), Ie = BB->end(); I != Ie; ++I) {
      std::ostringstream oss;
      I->print(oss);
      std::string InstrVal = oss.str();

      // Prettify the instruction for HTML view
      for (unsigned i = 0; i != InstrVal.length(); ++i)
        if (InstrVal[i] == '\n') {                          // \n => <br>
          InstrVal[i] = '<';
          std::string br = "br>";
          InstrVal.insert(InstrVal.begin()+i+1, br.begin(), br.end());
          i += br.size();
        } else if (InstrVal[i] == '\t') {                   // \t => &nbsp;
          InstrVal[i] = '&';
          std::string nbsp = "nbsp; &nbsp; ";
          InstrVal.insert(InstrVal.begin()+i+1, nbsp.begin(), nbsp.end());
          i += nbsp.size();
        } else if (InstrVal[i] == ';') {                    // Delete comments!
          unsigned Idx = InstrVal.find('\n', i+1);          // Find end of line
          InstrVal.erase(InstrVal.begin()+i, InstrVal.begin()+Idx);
          --i;
        }

      os << "<tt>" << stylizeTypesAndKeywords(InstrVal) << "</tt>";
    }
  }

  if (!F->isExternal ())
    os << "<tt>}</tt><br>";
}

void TVTreeItemData::printModule(Module *M, std::ostream &os) {
  // display target, endianness types

  // display all functions
  for (Module::iterator F = M->begin(), Fe = M->end(); F != Fe; ++F) {
    printFunction(F, os);
    os << "<br>";
  }
}

void TVTreeModuleItem::print(std::ostream &os) {
  htmlHeader(os);
  myModule->print(os); 
  htmlFooter(os);
}

void TVTreeFunctionItem::print(std::ostream &os) { 
  htmlHeader(os);
  myFunc->print(os);
  htmlFooter(os);
}

Module* TVTreeFunctionItem::getModule() {
  return myFunc ? myFunc->getParent() : 0; 
}

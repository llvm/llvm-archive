//===- GraphPrinters.cpp - DOT printers for various graph types -----------===//
// 
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
//
// This file defines several printers for various different types of graphs used
// by the LLVM infrastructure.  It uses the generic graph interface to convert
// the graph into a .dot graph.  These graphs can then be processed with the
// "dot" tool to convert them to postscript or some other suitable format.
//
//===----------------------------------------------------------------------===//

#include "llvm/Function.h"
#include "llvm/Pass.h"
#include "llvm/Value.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/DataStructure.h"
#include "llvm/Analysis/DSGraph.h"
#include "Support/GraphWriter.h"
#include <fstream>
using namespace llvm;

template<typename GraphType>
static void WriteGraphToFile(std::ostream &O, const std::string &GraphName,
                             const GraphType &GT) {
  std::string Filename = GraphName + ".dot";
  O << "Writing '" << Filename << "'...";
  std::ofstream F(Filename.c_str());
  
  if (F.good())
    WriteGraph(F, GT);
  else
    O << "  error opening file for writing!";
  O << "\n";
}


//===----------------------------------------------------------------------===//
//                              Call Graph Printer
//===----------------------------------------------------------------------===//

namespace llvm {
  template<>
  struct DOTGraphTraits<CallGraph*> : public DefaultDOTGraphTraits {
    static std::string getGraphName(CallGraph *F) {
      return "Call Graph";
    }
    
    static std::string getNodeLabel(CallGraphNode *Node, CallGraph *Graph) {
      if (Node->getFunction())
        return ((Value*)Node->getFunction())->getName();
      else
        return "Indirect call node";
    }
  };
}

namespace {
  struct CallGraphPrinter : public Pass {
    virtual bool run(Module &M) {
      WriteGraphToFile(std::cerr, "callgraph", &getAnalysis<CallGraph>());
      return false;
    }

    void print(std::ostream &OS) const {}
    
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<CallGraph>();
      AU.setPreservesAll();
    }
  };

  RegisterAnalysis<CallGraphPrinter> P2("print-callgraph",
                                        "Print Call Graph to 'dot' file");
}

//===----------------------------------------------------------------------===//
//                     BU DataStructures Graph Printer
//===----------------------------------------------------------------------===//

#if 0
namespace llvm {
  template<>
  struct DOTGraphTraits<DSGraph*> : public DefaultDOTGraphTraits {
    static std::string getGraphName(DSGraph *G) {
      return "BU DataStructures Graph";
    }
    
    static std::string getNodeLabel(DSNode *Node, DSGraph *G) {
      if (Node->getFunction())
        return ((Value*)Node->getFunction())->getName();
      else
        return "Indirect call node";
    }
  };
}
#endif

namespace {
  struct BUModulePrinter : public Pass {

    virtual bool run(Module &M) {
      BUDataStructures *BU = &getAnalysis<BUDataStructures>();
      std::string File = "buds.dot";
      std::ofstream of(File.c_str());
      if (of.good()) {
        BU->getGlobalsGraph().print(of);
        of.close();
      } else
        std::cerr << "Error writing to " << File << "!\n";
      return false;
    }

    void print(std::ostream &os) const {}

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<BUDataStructures>();
      AU.setPreservesAll();
    }
  };

  struct BUFunctionPrinter : public FunctionPass {
  
    virtual bool runOnFunction(Function &F) {
      BUDataStructures *BU = &getAnalysis<BUDataStructures>();
      std::string File = "buds." + F.getName() + ".dot";
      std::ofstream of(File.c_str());
      if (of.good()) {
        if (BU->hasGraph(F)) {
          BU->getDSGraph(F).print(of);
          of.close();
        } else
          std::cerr << "No BU DSGraph for: " << F.getName() << "\n";
      } else
        std::cerr << "Error writing to " << File << "!\n";
      return false;
    }

    void print(std::ostream &os) const {}

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<BUDataStructures>();
      AU.setPreservesAll();
    }
  };
}


//===----------------------------------------------------------------------===//
//                     TD DataStructures Graph Printer
//===----------------------------------------------------------------------===//

namespace {
  struct TDGraphPrinter : public Pass {
    TDDataStructures *TD;

    TDGraphPrinter() : TD(0) {}

    virtual bool run(Module &M) {
      TD = &getAnalysis<TDDataStructures>();
      return false;
    }

    void print(std::ostream &os) const {}

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<TDDataStructures>();
      AU.setPreservesAll();
    }
  };
}

//===----------------------------------------------------------------------===//
//                   Local DataStructures Graph Printer
//===----------------------------------------------------------------------===//

namespace {
  struct LocalGraphPrinter : public Pass {
    LocalDataStructures *L;

    LocalGraphPrinter() : L(0) {}

    virtual bool run(Module &M) {
      L = &getAnalysis<LocalDataStructures>();
      return false;
    }

    void print(std::ostream &os) const {}
    void printFunction(std::ostream &os, Function *F) const {}

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<LocalDataStructures>();
      AU.setPreservesAll();
    }
  };



}

namespace llvm {

Pass *createCallGraphPrinterPass () { return new CallGraphPrinter(); }

Pass *createBUDSModulePrinterPass () { return new BUModulePrinter(); }

FunctionPass *createBUDSFunctionPrinterPass () {
  return new BUFunctionPrinter();
}

Pass *createTDDSPrinterPass () { return new TDGraphPrinter(); }

Pass *createLocalDSPrinterPass () { return new LocalGraphPrinter(); }

} // end namespace llvm

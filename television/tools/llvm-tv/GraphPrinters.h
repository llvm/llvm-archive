//===- GraphPrinters.h - DOT printers for various graph types ---*- C++ -*-===//
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

#ifndef GRAPHPRINTERS_H
#define GRAPHPRINTERS_H

#include <iosfwd>
#include "llvm/Pass.h"
using namespace llvm;

namespace llvm {

Pass *createCallGraphPrinterPass ();

} // end namespace llvm

#endif // GRAPHPRINTERS_H

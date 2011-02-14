/*
 * Formatting.h - various utils for printing LLVM objects.
 */

#ifndef LLBROWSE_FORMATTING_H
#define LLBROWSE_FORMATTING_H

#ifndef CONFIG_H
#include "config.h"
#endif

#ifndef LLVM_ADT_STRINGREF_H
#include "llvm/ADT/StringRef.h"
#endif

#ifndef LLVM_SUPPORT_RAW_OSTREAM_H
#include "llvm/Support/raw_ostream.h"
#endif

#ifndef _WX_WX_H_
#include "wx/wx.h"
#endif

#ifndef _WX_WXSTRINGH__
#include "wx/string.h"
#endif

#ifndef _WX_TXTSTREAM_H_
#include "wx/txtstrm.h"
#endif

#ifndef _STDINT_H
#include <stdint.h>
#endif

namespace llvm {
class Constant;
class Module;
class Type;
class Value;
}

/// Pretty-print an LLVM type expression to a stream.
/// 'maxDepth' limits expansion of complex types.
///
void printType(wxTextOutputStream& out, const llvm::Module* module,
    const llvm::Type* type, uint32_t maxDepth = -1);

/// Pretty-print an LLVM type expression to a stream, with the outermost
/// type dealiased (print the definition instead of the type name.)
/// 'maxDepth' limits expansion of complex types.
///
void printTypeExpansion(wxTextOutputStream& out, const llvm::Module* module,
    const llvm::Type* type, uint32_t maxDepth = -1);

/// Pretty-print an LLVM constant expression to a stream.
/// 'maxDepth' limits expansion of complex expressions.
///
void printConstant(wxTextOutputStream& out, const llvm::Module* module,
    const llvm::Constant* val, uint32_t maxDepth = -1);

/// Pretty-print list of constants separated by commas.
/// 'maxDepth' limits expansion of complex expressions.
///
void printConstantList(wxTextOutputStream& out, const llvm::Module* module,
    const llvm::Constant* parent, uint32_t maxDepth = -1);

/// Pretty-print an LLVM type metadata node.
/// 'maxDepth' limits expansion of complex nodes.
///
void printMetadata(wxTextOutputStream& out, const llvm::Value* nodeVal,
    uint32_t maxDepth = -1);

/// Conversion from LLVM StringRef to wxString.
///
inline wxString toWxStr(const llvm::StringRef& ref) {
  return wxString::From8BitData(ref.data(), ref.size());
}

#endif // LLBROWSE_FORMATTING_H

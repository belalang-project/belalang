#ifndef BELALANG_DIAG_DIAG_H_
#define BELALANG_DIAG_DIAG_H_

#include "llvm/ADT/StringRef.h"
#include <cstddef>
#include <string>
#include <vector>

namespace belalang {
namespace diag {

// -----------------------------------------------------------------------------
// Diagnostics Information
// -----------------------------------------------------------------------------

enum class Severity {
  Error,
  Warning,
  Note,
  Help,
};

struct Label {
  size_t spanStart;
  size_t spanEnd;

  std::string message;
  bool isPrimary;

  static Label primary(size_t start, size_t end, std::string message) {
    return {start, end, std::move(message), true};
  }

  static Label secondary(size_t start, size_t end, std::string message) {
    return {start, end, std::move(message), false};
  }
};

struct Diagnostic {
  Severity severity;
  std::string message;
  std::vector<Label> labels;

  static Diagnostic error(std::string message) {
    return {Severity::Error, std::move(message), {}};
  }

  Diagnostic &withLabel(Label label) {
    labels.push_back(std::move(label));
    return *this;
  }
};

// -----------------------------------------------------------------------------
// Diagnostics Engine
// -----------------------------------------------------------------------------

class DiagnosticEngine {
  llvm::StringRef sourceText;
  llvm::StringRef sourceFile;
  bool useColor;

public:
  DiagnosticEngine(llvm::StringRef sourceText, llvm::StringRef sourceFile,
                   bool useColor = false)
      : sourceText(sourceText), sourceFile(sourceFile), useColor(useColor) {}

  void print(const Diagnostic &diag) const;
};

} // namespace diag
} // namespace belalang

#endif // BELALANG_DIAG_DIAG_H_

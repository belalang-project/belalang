#include "belalang/Diag/Diag.h"
#include <iostream>

extern "C" {

struct FFILabel {
  size_t span_start;
  size_t span_end;
  const char *message;
  bool is_primary;
};

struct FFIDiagnostic {
  uint8_t severity;
  const char *message;
  const FFILabel *labels;
  size_t labels_len;
};

void belalang_print_diagnostic(const FFIDiagnostic *diag,
                               const char *source_text, size_t source_text_len,
                               const char *source_file, size_t source_file_len,
                               bool use_color);
}

namespace belalang {
namespace diag {

void DiagnosticEngine::print(const Diagnostic &diag) const {
  // Translate labels to FFI labels.
  std::vector<FFILabel> ffiLabels;
  ffiLabels.reserve(diag.labels.size());
  for (const auto &label : diag.labels) {
    ffiLabels.push_back({label.spanStart, label.spanEnd, label.message.c_str(),
                         label.isPrimary});
  }

  // Translate diagnostic itself to FFI diagnostic.
  FFIDiagnostic ffiDiag;
  ffiDiag.severity = static_cast<uint8_t>(diag.severity);
  ffiDiag.message = diag.message.c_str();
  ffiDiag.labels = ffiLabels.data();
  ffiDiag.labels_len = ffiLabels.size();

  // Emit the diagnostic.
  //
  // We rely on the Rust side of things to print the diagnostic. I believe this
  // is a simpler approach than to return a string and print it out in the C++
  // side.
  belalang_print_diagnostic(
      &ffiDiag, reinterpret_cast<const char *>(sourceText.data()),
      sourceText.size(), reinterpret_cast<const char *>(sourceFile.data()),
      sourceFile.size(), useColor);
}

} // namespace diag
} // namespace belalang

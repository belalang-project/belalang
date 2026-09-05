#include "belalang/Diag/Diag.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"

#include <algorithm>

namespace {

llvm::SourceMgr::DiagKind toLLVMSeverity(belalang::diag::Severity severity) {
  switch (severity) {
  case belalang::diag::Severity::Error:
    return llvm::SourceMgr::DK_Error;
  case belalang::diag::Severity::Warning:
    return llvm::SourceMgr::DK_Warning;
  case belalang::diag::Severity::Note:
  case belalang::diag::Severity::Help:
    return llvm::SourceMgr::DK_Note;
  }

  llvm_unreachable("unknown diagnostic severity");
}

} // namespace

namespace belalang {
namespace diag {

void DiagnosticEngine::print(const Diagnostic &diag) const {
  llvm::SourceMgr sourceMgr;
  sourceMgr.AddNewSourceBuffer(
      llvm::MemoryBuffer::getMemBuffer(sourceText, sourceFile, false),
      llvm::SMLoc());

  const auto kind = toLLVMSeverity(diag.severity);
  if (diag.labels.empty() || sourceText.empty()) {
    llvm::SMDiagnostic(sourceFile, kind, diag.message)
        .print(nullptr, llvm::errs(), useColor);
    return;
  }

  auto anchor = std::find_if(diag.labels.begin(), diag.labels.end(),
                             [](const Label &label) {
                               return label.isPrimary;
                             });
  if (anchor == diag.labels.end())
    anchor = diag.labels.begin();

  const char *sourceStart = sourceText.data();
  auto toRange = [&](const Label &label) {
    const size_t start = std::min(label.spanStart, sourceText.size());
    const size_t end = std::clamp(label.spanEnd, start, sourceText.size());
    return llvm::SMRange(llvm::SMLoc::getFromPointer(sourceStart + start),
                         llvm::SMLoc::getFromPointer(sourceStart + end));
  };

  std::vector<llvm::SMRange> ranges;
  ranges.reserve(diag.labels.size());
  for (const Label &label : diag.labels)
    ranges.push_back(toRange(label));

  std::string message = diag.message;
  if (!anchor->message.empty() && anchor->message != diag.message) {
    message += ": ";
    message += anchor->message;
  }

  const llvm::SMRange anchorRange = toRange(*anchor);
  sourceMgr.PrintMessage(llvm::errs(), anchorRange.Start, kind, message, ranges,
                         {}, useColor);

  for (auto label = diag.labels.begin(); label != diag.labels.end(); ++label) {
    if (label == anchor || label->message.empty())
      continue;

    const bool duplicate = std::any_of(
        diag.labels.begin(), label, [&](const Label &previous) {
          return previous.spanStart == label->spanStart &&
                 previous.spanEnd == label->spanEnd &&
                 previous.message == label->message;
        });
    if (duplicate)
      continue;

    const llvm::SMRange range = toRange(*label);
    sourceMgr.PrintMessage(llvm::errs(), range.Start, llvm::SourceMgr::DK_Note,
                           label->message, {range}, {}, useColor);
  }
}

} // namespace diag
} // namespace belalang

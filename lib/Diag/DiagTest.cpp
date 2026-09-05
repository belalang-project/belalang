#include "belalang/Diag/Diag.h"

#include "gtest/gtest.h"

#include <string>
#include <utility>

namespace belalang::diag {
namespace {

std::string render(const Diagnostic &diag, llvm::StringRef source = "abc\n") {
  testing::internal::CaptureStderr();
  DiagnosticEngine(source, "test.bel").print(diag);
  return testing::internal::GetCapturedStderr();
}

TEST(DiagnosticEngineTest, RendersAndMergesPrimaryLabel) {
  Diagnostic diag = Diagnostic::error("mismatched type");
  diag.withLabel(Label::primary(1, 3, "expected int"));

  EXPECT_EQ(render(diag),
            "test.bel:1:2: error: mismatched type: expected int\n"
            "abc\n"
            " ^~\n");
}

TEST(DiagnosticEngineTest, SuppressesDuplicatePrimaryMessage) {
  Diagnostic diag = Diagnostic::error("bad token");
  diag.withLabel(Label::primary(0, 1, "bad token"));

  EXPECT_EQ(render(diag),
            "test.bel:1:1: error: bad token\n"
            "abc\n"
            "^\n");
}

TEST(DiagnosticEngineTest, UsesFirstLabelWhenThereIsNoPrimary) {
  Diagnostic diag = Diagnostic::error("bad operands");
  diag.withLabel(Label::secondary(0, 1, "left operand"))
      .withLabel(Label::secondary(2, 3, "right operand"));

  EXPECT_EQ(render(diag),
            "test.bel:1:1: error: bad operands: left operand\n"
            "abc\n"
            "^ ~\n"
            "test.bel:1:3: note: right operand\n"
            "abc\n"
            "  ^\n");
}

TEST(DiagnosticEngineTest, RendersSecondaryLabelOnAnotherLineAsNote) {
  Diagnostic diag = Diagnostic::error("bad operands");
  diag.withLabel(Label::primary(0, 1, "bad operands"))
      .withLabel(Label::secondary(5, 7, "second operand"));

  EXPECT_EQ(render(diag, "abc\ndef\n"),
            "test.bel:1:1: error: bad operands\n"
            "abc\n"
            "^\n"
            "test.bel:2:2: note: second operand\n"
            "def\n"
            " ^~\n");
}

TEST(DiagnosticEngineTest, ClampsInvalidSpans) {
  Diagnostic diag = Diagnostic::error("past end");
  diag.withLabel(Label::primary(100, 50, "past end"));

  EXPECT_EQ(render(diag, "abc"),
            "test.bel:1:4: error: past end\n"
            "abc\n"
            "   ^\n");
}

TEST(DiagnosticEngineTest, RendersLocationFreeDiagnostic) {
  EXPECT_EQ(render(Diagnostic::error("failed"), ""),
            "test.bel: error: failed\n");
}

TEST(DiagnosticEngineTest, MapsWarningNoteAndHelpSeverities) {
  for (const auto &[severity, expected] : {
           std::pair{Severity::Warning, "warning"},
           std::pair{Severity::Note, "note"},
           std::pair{Severity::Help, "note"},
       }) {
    Diagnostic diag{severity, "message", {Label::primary(0, 1, "message")}};
    EXPECT_EQ(render(diag), std::string("test.bel:1:1: ") + expected +
                                ": message\nabc\n^\n");
  }
}

} // namespace
} // namespace belalang::diag

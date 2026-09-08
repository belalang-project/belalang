#include <optional>
#include <string_view>

#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/raw_ostream.h"

#include "Belalang.h"

constexpr std::string_view help = R"(belalang

Usage: belalang [OPTIONS] <COMMAND> <PATH>

Commands:
  build    Compile a .bel file
  run      Run a .bel file
  version  Print version information
  help     Print help message
)";

namespace belalang {
namespace cmd {

llvm::Expected<Path> createTemporaryDirectory(llvm::StringRef prefix) {
  Path directory;
  if (std::error_code ec = llvm::sys::fs::createUniqueDirectory(prefix,
                                                                directory))
    return llvm::errorCodeToError(ec);

  return directory;
}

void removeTemporaryDirectory(llvm::StringRef directory) {
  if (std::error_code ec = llvm::sys::fs::remove_directories(directory, false))
    term::warning() << "could not remove temporary directory: " << ec.message()
                    << "\n";
}

Path pathInDirectory(llvm::StringRef directory, llvm::StringRef filename) {
  Path path(directory);
  llvm::sys::path::append(path, filename);
  return path;
}

Path executablePathForSource(llvm::StringRef source) {
  llvm::StringRef stem = llvm::sys::path::stem(source);
  return Path(stem.empty() ? "a.out" : stem);
}

llvm::Expected<int> link(const BelalangCtx &ctx, llvm::StringRef objectFile,
                         llvm::StringRef executable) {
  llvm::ErrorOr<std::string> compilerPath = llvm::sys::findProgramByName(
      ctx.cc_cmd);
  if (!compilerPath)
    return llvm::createStringError(compilerPath.getError(),
                                   "could not find compiler '%s'",
                                   ctx.cc_cmd.c_str());

  std::string libraryArgument = "-L" + ctx.brt_dir;
  Path stackmapsPath(ctx.brt_dir);
  llvm::sys::path::append(stackmapsPath, "llvm_stackmaps.ld");
  std::string stackmapsArgument = "-Wl,-T," + stackmapsPath.str().str();

  llvm::SmallVector<llvm::StringRef, 8> arguments = {
      *compilerPath,     "-no-pie", objectFile, libraryArgument,
      stackmapsArgument, "-lbrt",   "-o",       executable,
  };

  std::string errorMessage;
  bool executionFailed = false;
  int exitCode = llvm::sys::ExecuteAndWait(*compilerPath, arguments,
                                           std::nullopt, {}, 0, 0,
                                           &errorMessage, &executionFailed);
  if (executionFailed)
    return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "could not execute compiler '%s': %s",
                                   ctx.cc_cmd.c_str(), errorMessage.c_str());

  return exitCode;
}

llvm::Expected<int> execute(llvm::StringRef executable) {
  llvm::SmallVector<llvm::StringRef, 1> arguments = {executable};

  std::string errorMessage;
  bool executionFailed = false;
  int exitCode = llvm::sys::ExecuteAndWait(executable, arguments, std::nullopt,
                                           {}, 0, 0, &errorMessage,
                                           &executionFailed);
  if (executionFailed)
    return llvm::createStringError(
        llvm::inconvertibleErrorCode(), "could not execute '%s': %s",
        executable.str().c_str(), errorMessage.c_str());

  return exitCode;
}

namespace term {

llvm::raw_ostream &error() {
  llvm::raw_ostream &os = llvm::errs();
  os.changeColor(llvm::raw_ostream::RED, true);
  os << "error: ";
  os.resetColor();
  return os;
}

llvm::raw_ostream &warning() {
  llvm::raw_ostream &os = llvm::errs();
  os.changeColor(llvm::raw_ostream::YELLOW, true);
  os << "warning: ";
  os.resetColor();
  return os;
}

llvm::raw_ostream &hint() {
  llvm::raw_ostream &os = llvm::errs();
  os.changeColor(llvm::raw_ostream::CYAN, true);
  os << "hint: ";
  os.resetColor();
  return os;
}

} // namespace term
} // namespace cmd
} // namespace belalang

int main(int argc, char **argv) {
  muopt::Parser parser(argc, argv);

  std::optional<std::string> command;

  auto arg = parser.next();
  if (arg.has_value() && arg->is_plain())
    command = arg->as_str();

  if (!command.has_value()) {
    llvm::errs() << help;
    return 1;
  }

  if (*command == "help") {
    llvm::outs() << help;
    return 0;
  }

  std::string brt = llvm::sys::Process::GetEnv("BRT_DIR").value_or(
      "/usr/local/lib");
  std::string cc = llvm::sys::Process::GetEnv("CC").value_or("cc");

  const belalang::cmd::BelalangCtx ctx{cc, brt};

  if (*command == "build")
    return belalang::cmd::build(parser, ctx);

  if (*command == "version")
    return belalang::cmd::version();

  if (*command == "run") {
    return belalang::cmd::run(parser, ctx);
  }

  belalang::cmd::term::error() << "unknown command: " << *command << "\n";
  belalang::cmd::term::hint() << "available commands are build, run, help\n";
  return 1;
}

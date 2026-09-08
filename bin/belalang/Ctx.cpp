#include "Ctx.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/raw_ostream.h"
#include <optional>

#include "Term.h"

namespace belalang {
namespace cmd {

llvm::Expected<Path> createTemporaryDirectory(llvm::StringRef prefix) {
  Path directory;
  if (std::error_code ec =
          llvm::sys::fs::createUniqueDirectory(prefix, directory))
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
  llvm::ErrorOr<std::string> compilerPath = llvm::sys::findProgramByName(ctx.cc_cmd);
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
      stackmapsArgument, "-lbrt",   "-o",      executable,
  };

  std::string errorMessage;
  bool executionFailed = false;
  int exitCode = llvm::sys::ExecuteAndWait(
      *compilerPath, arguments, std::nullopt, {}, 0, 0, &errorMessage,
      &executionFailed);
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
  int exitCode = llvm::sys::ExecuteAndWait(
      executable, arguments, std::nullopt, {}, 0, 0, &errorMessage,
      &executionFailed);
  if (executionFailed)
    return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "could not execute '%s': %s",
                                   executable.str().c_str(),
                                   errorMessage.c_str());

  return exitCode;
}

} // namespace cmd
} // namespace belalang

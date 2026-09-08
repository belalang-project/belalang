#include <optional>
#include <string_view>

#include "llvm/Support/Process.h"
#include "llvm/Support/raw_ostream.h"

#include "Cmds.h"
#include "Ctx.h"

constexpr std::string_view help = R"(belalang

Usage: belalang [OPTIONS] <COMMAND> <PATH>

Commands:
  build    Compile a .bel file
  run      Run a .bel file
  version  Print version information
  help     Print help message
)";

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

  llvm::errs() << "error: unknown command: " << *command << "\n";
  llvm::errs() << "hint: available commands are build, run, help\n";
  return 1;
}

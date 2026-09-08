#include <iostream>
#include <optional>
#include <string_view>

#include "llvm/Support/Process.h"

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
    std::cerr << help;
    return 1;
  }

  if (*command == "help") {
    std::cerr << help;
    return 0;
  }

  std::string brt_path = llvm::sys::Process::GetEnv("BRT_DIR").value_or(
      "/usr/local/lib");
  std::string cc_cmd = llvm::sys::Process::GetEnv("CC").value_or("cc");

  belalang::cmd::BelalangCtx ctx{cc_cmd, brt_path};

  if (*command == "build")
    return belalang::cmd::build(parser, ctx);

  if (*command == "version")
    return belalang::cmd::version();

  if (*command == "run") {
    return belalang::cmd::run(parser, ctx);
  }

  std::cerr << "error: unknown command: " << *command << "\n";
  std::cerr << "hint: available commands are build, run, help\n";
  return 1;
}

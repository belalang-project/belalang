#include <iostream>
#include <optional>
#include <string_view>

#include "Cmds.h"

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

  if (*command == "build")
    return belalang::cmd::build(parser);

  if (*command == "version")
    return belalang::cmd::version();

  if (*command == "run") {
    std::cerr << "running\n";
    return 0;
  }

  std::cerr << "error: unknown command: " << *command << "\n";
  std::cerr << "hint: available commands are build, run, help\n";
  return 1;
}

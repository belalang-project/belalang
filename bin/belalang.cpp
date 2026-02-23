#include <CLI/CLI.hpp>
#include <iostream>

void setup_compile_command(CLI::App &app) {
  auto *cmd = app.add_subcommand("compile", "compile source to bytecode");

  cmd->callback([cmd] { std::cout << "Compiled!\n"; });
}

void setup_dis_command(CLI::App &app) {
  auto *cmd = app.add_subcommand("dis", "disassemble bytecode to source");

  cmd->callback([cmd] { std::cout << "Disassembled!\n"; });
}

int main(int argc, char **argv) {
  CLI::App app{"Belalang"};

  setup_compile_command(app);
  setup_dis_command(app);

  app.require_subcommand(1);

  CLI11_PARSE(app, argc, argv);
  return 0;
}

// RUN: %bir-opt --split-input-file --verify-roundtrip %s | %FileCheck %s

bir.func @main() -> !bir.int {
  %0 = bir.scope {
    %1 = bir.constant #bir.int<42> : !bir.int
    bir.yield %1 : !bir.int
  } : !bir.int
  bir.return %0 : !bir.int
}

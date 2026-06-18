// RUN: %bir-opt --split-input-file --verify-roundtrip %s | %FileCheck %s

// CHECK: bir.func
bir.func @main() -> !bir.int {
  %0 = bir.var.declare "x" : !bir.ref<!bir.int>
  %1 = bir.constant #bir.int<12> : !bir.int
  bir.var.store %1 to %0 : !bir.int to !bir.ref<!bir.int>
  %2 = bir.var.load %0 : (!bir.ref<!bir.int>) -> !bir.int
  bir.return %2 : !bir.int
}

// -----

// CHECK: bir.func
bir.func @main() -> !bir.int {
  // x := 12
  %0 = bir.var.declare "x" : !bir.ref<!bir.int>
  %1 = bir.constant #bir.int<12> : !bir.int
  bir.var.store %1 to %0 : !bir.int to !bir.ref<!bir.int>

  // return x + 1
  %2 = bir.constant #bir.int<1> : !bir.int
  %3 = bir.var.load %0 : (!bir.ref<!bir.int>) -> !bir.int
  %4 = bir.add %3, %2 : (!bir.int, !bir.int) -> !bir.int
  bir.return %4 : !bir.int
}

// -----

// CHECK: bir.func
bir.func @main() -> !bir.string {
  // x := "hello"
  %0 = bir.constant #bir.string<"hello"> : !bir.string
  %1 = bir.var.declare "x" :!bir.ref<!bir.string>
  bir.var.store %0 to %1 : !bir.string to !bir.ref<!bir.string>

  // return x
  %2 = bir.var.load %1 : (!bir.ref<!bir.string>) -> !bir.string
  bir.return %2 : !bir.string
}

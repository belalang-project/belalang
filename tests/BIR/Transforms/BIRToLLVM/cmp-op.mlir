// RUN: %bir-opt --split-input-file --bir-lowering-pipeline --convert-bir-to-llvm %s | %FileCheck %s

bir.func @main() {
  // CHECK: %[[C0:.*]] = llvm.mlir.constant(0 : i64) : i64
  // CHECK: %[[C1:.*]] = llvm.mlir.constant(1 : i64) : i64

  %0 = bir.constant #bir.int<0> : !bir.int
  %1 = bir.constant #bir.int<1> : !bir.int

  // CHECK: llvm.icmp "slt" %[[C0]], %[[C1]] : i64
  // CHECK: llvm.icmp "sle" %[[C0]], %[[C1]] : i64
  // CHECK: llvm.icmp "sgt" %[[C0]], %[[C1]] : i64
  // CHECK: llvm.icmp "sge" %[[C0]], %[[C1]] : i64
  // CHECK: llvm.icmp "eq" %[[C0]], %[[C1]] : i64
  // CHECK: llvm.icmp "ne" %[[C0]], %[[C1]] : i64

  %2 = bir.cmp lt %0, %1 : !bir.int
  bir.print %2 : !bir.bool

  %3 = bir.cmp le %0, %1 : !bir.int
  bir.print %3 : !bir.bool

  %4 = bir.cmp gt %0, %1 : !bir.int
  bir.print %4 : !bir.bool

  %5 = bir.cmp ge %0, %1 : !bir.int
  bir.print %5 : !bir.bool

  %6 = bir.cmp eq %0, %1 : !bir.int
  bir.print %6 : !bir.bool

  %7 = bir.cmp ne %0, %1 : !bir.int
  bir.print %7 : !bir.bool

  bir.return
}

// -----

bir.func @main() {
  // CHECK: %[[C0:.*]] = llvm.mlir.constant(0.000000e+00 : f64) : f64
  // CHECK: %[[C1:.*]] = llvm.mlir.constant(1.000000e+00 : f64) : f64

  %0 = bir.constant #bir.float<0.00> : !bir.float
  %1 = bir.constant #bir.float<1.00> : !bir.float

  // CHECK: llvm.fcmp "olt" %[[C0]], %[[C1]] : f64
  // CHECK: llvm.fcmp "ole" %[[C0]], %[[C1]] : f64
  // CHECK: llvm.fcmp "ogt" %[[C0]], %[[C1]] : f64
  // CHECK: llvm.fcmp "oge" %[[C0]], %[[C1]] : f64
  // CHECK: llvm.fcmp "oeq" %[[C0]], %[[C1]] : f64
  // CHECK: llvm.fcmp "one" %[[C0]], %[[C1]] : f64

  %2 = bir.cmp lt %0, %1 : !bir.float
  bir.print %2 : !bir.bool

  %3 = bir.cmp le %0, %1 : !bir.float
  bir.print %3 : !bir.bool

  %4 = bir.cmp gt %0, %1 : !bir.float
  bir.print %4 : !bir.bool

  %5 = bir.cmp ge %0, %1 : !bir.float
  bir.print %5 : !bir.bool

  %6 = bir.cmp eq %0, %1 : !bir.float
  bir.print %6 : !bir.bool

  %7 = bir.cmp ne %0, %1 : !bir.float
  bir.print %7 : !bir.bool

  bir.return
}


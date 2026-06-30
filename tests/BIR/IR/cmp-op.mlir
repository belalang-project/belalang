// RUN: %bir-opt --split-input-file --verify-roundtrip --verify-diagnostics %s | %FileCheck %s

// CHECK-LABEL: bir.func @main()
// CHECK-NEXT: %[[C0:.*]] = bir.constant #bir.int<0> : !bir.int
// CHECK-NEXT: %[[C1:.*]] = bir.constant #bir.int<1> : !bir.int
// CHECK-NEXT: bir.cmp lt %[[C0]], %[[C1]] : !bir.int
// CHECK-NEXT: bir.cmp le %[[C0]], %[[C1]] : !bir.int
// CHECK-NEXT: bir.cmp gt %[[C0]], %[[C1]] : !bir.int
// CHECK-NEXT: bir.cmp ge %[[C0]], %[[C1]] : !bir.int
// CHECK-NEXT: bir.cmp eq %[[C0]], %[[C1]] : !bir.int
// CHECK-NEXT: bir.cmp ne %[[C0]], %[[C1]] : !bir.int

bir.func @main() {
  %0 = bir.constant #bir.int<0> : !bir.int
  %1 = bir.constant #bir.int<1> : !bir.int

  %2 = bir.cmp lt %0, %1 : !bir.int
  %3 = bir.cmp le %0, %1 : !bir.int
  %4 = bir.cmp gt %0, %1 : !bir.int
  %5 = bir.cmp ge %0, %1 : !bir.int
  %6 = bir.cmp eq %0, %1 : !bir.int
  %7 = bir.cmp ne %0, %1 : !bir.int

  bir.return
}

// -----

// CHECK-LABEL: bir.func @main()
// CHECK-NEXT: %[[C0:.*]] = bir.constant #bir.float<0.000000e+00> : !bir.float
// CHECK-NEXT: %[[C1:.*]] = bir.constant #bir.float<1.000000e+00> : !bir.float
// CHECK-NEXT: bir.cmp lt %[[C0]], %[[C1]] : !bir.float
// CHECK-NEXT: bir.cmp le %[[C0]], %[[C1]] : !bir.float
// CHECK-NEXT: bir.cmp gt %[[C0]], %[[C1]] : !bir.float
// CHECK-NEXT: bir.cmp ge %[[C0]], %[[C1]] : !bir.float
// CHECK-NEXT: bir.cmp eq %[[C0]], %[[C1]] : !bir.float
// CHECK-NEXT: bir.cmp ne %[[C0]], %[[C1]] : !bir.float

bir.func @main() {
  %0 = bir.constant #bir.float<0.00> : !bir.float
  %1 = bir.constant #bir.float<1.00> : !bir.float

  %2 = bir.cmp lt %0, %1 : !bir.float
  %3 = bir.cmp le %0, %1 : !bir.float
  %4 = bir.cmp gt %0, %1 : !bir.float
  %5 = bir.cmp ge %0, %1 : !bir.float
  %6 = bir.cmp eq %0, %1 : !bir.float
  %7 = bir.cmp ne %0, %1 : !bir.float

  bir.return
}

// -----

bir.func @main() {
  // expected-note@+1 {{prior use here}}
  %0 = bir.constant #bir.float<0.00> : !bir.float
  %1 = bir.constant #bir.int<1> : !bir.int

  // expected-error@+1 {{expects different type than prior uses: '!bir.int' vs '!bir.float'}}
  %2 = bir.cmp lt %1, %0 : !bir.int

  bir.return
}

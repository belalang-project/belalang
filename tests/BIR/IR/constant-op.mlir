// RUN: %bir-opt --split-input-file --verify-roundtrip --verify-diagnostics %s | %FileCheck %s

bir.func @intint() {
  // CHECK: %[[C0:.*]] = bir.constant #bir.int<4> : !bir.int
  %0 = bir.constant #bir.int<4> : !bir.int
  bir.return
}

// -----

bir.func @floatfloat() {
  // CHECK: %[[C0:.*]] = bir.constant #bir.float<4.000000e+00> : !bir.float
  %0 = bir.constant #bir.float<4.00> : !bir.float
  bir.return
}

// -----

bir.func @stringstring() {
  // CHECK: %[[C0:.*]] = bir.constant #bir.string<"hello"> : !bir.string
  %0 = bir.constant #bir.string<"hello"> : !bir.string
  bir.return
}

// -----

bir.func @intfloat() {
  // expected-error@+1 {{type and attribute mismatch}}
  %0 = bir.constant #bir.int<4> : !bir.float
  bir.return
}

// -----

bir.func @floatint() {
  // expected-error@+1 {{type and attribute mismatch}}
  %0 = bir.constant #bir.float<4.00> : !bir.int
  bir.return
}

// -----

bir.func @stringint() {
  // expected-error@+1 {{type and attribute mismatch}}
  %0 = bir.constant #bir.string<"hello"> : !bir.int
  bir.return
}

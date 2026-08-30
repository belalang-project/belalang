// RUN: %not %bir-opt --verify-diagnostics %s 2>&1 | %FileCheck %s

// CHECK: error: 'bir.func' op symbol declaration cannot have public visibility
bir.func @public_declaration()

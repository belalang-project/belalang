// RUN: %bir-opt --cse --split-input-file %s | %FileCheck %s

// CHECK-LABEL: func.func @merge_loads
// CHECK-SAME:      (%[[PTR:.*]]: !gc.ptr<i64>)
// CHECK-NEXT:    %[[VAL:.*]] = gc.load %[[PTR]] : !gc.ptr<i64>
// CHECK-NEXT:    return %[[VAL]], %[[VAL]] : i64, i64

func.func @merge_loads(%ptr : !gc.ptr<i64>) -> (i64, i64) {
  %first = gc.load %ptr : !gc.ptr<i64>
  %second = gc.load %ptr : !gc.ptr<i64>
  return %first, %second : i64, i64
}

// -----

// CHECK-LABEL: func.func @store_blocks_load_cse
// CHECK-SAME:      (%[[PTR:.*]]: !gc.ptr<i64>, %[[NEW_VAL:.*]]: i64)
// CHECK-NEXT:    %[[BEFORE:.*]] = gc.load %[[PTR]] : !gc.ptr<i64>
// CHECK-NEXT:    gc.store %[[NEW_VAL]], %[[PTR]] : !gc.ptr<i64>
// CHECK-NEXT:    %[[AFTER:.*]] = gc.load %[[PTR]] : !gc.ptr<i64>
// CHECK-NEXT:    return %[[BEFORE]], %[[AFTER]] : i64, i64

func.func @store_blocks_load_cse(%ptr : !gc.ptr<i64>, %new_value : i64) -> (i64, i64) {
  %before = gc.load %ptr : !gc.ptr<i64>
  gc.store %new_value, %ptr : !gc.ptr<i64>
  %after = gc.load %ptr : !gc.ptr<i64>
  return %before, %after : i64, i64
}

// -----

// CHECK-LABEL: func.func @call_blocks_load_cse
// CHECK-SAME:      (%[[PTR:.*]]: !gc.ptr<i64>)
// CHECK-NEXT:    %[[BEFORE:.*]] = gc.load %[[PTR]] : !gc.ptr<i64>
// CHECK-NEXT:    gc.call @unknown() : () -> ()
// CHECK-NEXT:    %[[AFTER:.*]] = gc.load %[[PTR]] : !gc.ptr<i64>
// CHECK-NEXT:    return %[[BEFORE]], %[[AFTER]] : i64, i64

func.func @call_blocks_load_cse(%ptr : !gc.ptr<i64>) -> (i64, i64) {
  %before = gc.load %ptr : !gc.ptr<i64>
  gc.call @unknown() : () -> ()
  %after = gc.load %ptr : !gc.ptr<i64>
  return %before, %after : i64, i64
}

// -----

// CHECK-LABEL: func.func @remove_dead_allocations
// CHECK-NEXT:    return
func.func @remove_dead_allocations() {
  %heap = gc.alloc : !gc.ptr<i64>
  %stack = gc.alloca : !gc.ptr<i64>
  return
}

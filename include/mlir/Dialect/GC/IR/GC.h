#ifndef MLIR_DIALECT_GC_IR_GC_H_
#define MLIR_DIALECT_GC_IR_GC_H_

#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/Types.h"
#include "mlir/Interfaces/CallInterfaces.h"
#include "mlir/Interfaces/DataLayoutInterfaces.h"
#include "mlir/Interfaces/MemorySlotInterfaces.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/TypeSwitch.h"

#include "mlir/Dialect/GC/IR/GCDialect.h.inc"

#define GET_TYPEDEF_CLASSES
#include "mlir/Dialect/GC/IR/GCTypes.h.inc"

#define GET_OP_CLASSES
#include "mlir/Dialect/GC/IR/GCOps.h.inc"

namespace mlir::gc {

inline constexpr llvm::StringLiteral kRuntimeAllocAttrName = "gc.runtime.alloc";
inline constexpr llvm::StringLiteral
    kRuntimePushRootsAttrName = "gc.runtime.push_roots";
inline constexpr llvm::StringLiteral
    kRuntimePopRootsAttrName = "gc.runtime.pop_roots";

} // namespace mlir::gc

#endif // MLIR_DIALECT_GC_IR_GC_H_

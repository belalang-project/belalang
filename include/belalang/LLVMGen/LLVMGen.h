#ifndef BELALANG_LLVMGEN_LLVMGEN_H_
#define BELALANG_LLVMGEN_LLVMGEN_H_

#include "mlir/IR/BuiltinOps.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <cstdint>
#include <memory>

#include "rust/cxx.h"

namespace belalang {
namespace llvmgen {
class LLVMGen;
} // namespace llvmgen
} // namespace belalang

#include "llvmgen/lib.rs.h"

namespace belalang {
namespace llvmgen {

class LLVMGen {
public:
  LLVMGen(mlir::ModuleOp *module);
  ~LLVMGen() = default;

  rust::String dump_to_string() const;
  rust::String compile_object_file(rust::String out,
                                   SanitizerKind sanitizer) const;

private:
  llvm::LLVMContext llvmCtx;
  std::unique_ptr<llvm::Module> llvmModule;
};

/// Creates an LLVMGen instance. `module_ptr` should point to the lowered
/// `mlir::ModuleOp` containing bir operations.
std::unique_ptr<LLVMGen> create_llvmgen(uintptr_t module_ptr);

} // namespace llvmgen
} // namespace belalang

#endif // BELALANG_LLVMGEN_LLVMGEN_H_

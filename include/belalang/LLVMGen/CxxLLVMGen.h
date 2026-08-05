#ifndef BELALANG_LLVMGEN_CXXLLVMGEN_H_
#define BELALANG_LLVMGEN_CXXLLVMGEN_H_

#include "mlir/IR/BuiltinOps.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <cstdint>
#include <memory>

#include "rust/cxx.h"

namespace belalang {
namespace llvmgen {
class CxxLLVMGen;
} // namespace llvmgen
} // namespace belalang

#include "llvmgen/lib.rs.h"

namespace belalang {
namespace llvmgen {

class CxxLLVMGen {
public:
  CxxLLVMGen(mlir::ModuleOp *module);
  ~CxxLLVMGen() = default;

  rust::String dump_to_string() const;
  rust::String compile_object_file(rust::String out,
                                   SanitizerKind sanitizer) const;

private:
  llvm::LLVMContext llvmCtx;
  std::unique_ptr<llvm::Module> llvmModule;
};

/// Creates an CxxLLVMGen instance. `module_ptr` should point to the lowered
/// `mlir::ModuleOp` containing bir operations.
std::unique_ptr<CxxLLVMGen> create_llvmgen(uintptr_t module_ptr);

} // namespace llvmgen
} // namespace belalang

#endif // BELALANG_LLVMGEN_CXXLLVMGEN_H_

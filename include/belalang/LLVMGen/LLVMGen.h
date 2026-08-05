#ifndef BELALANG_LLVMGEN_LLVMGEN_H_
#define BELALANG_LLVMGEN_LLVMGEN_H_

#include "mlir/IR/BuiltinOps.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <cstdint>
#include <memory>

namespace belalang {
namespace llvmgen {

enum class SanitizerKind {
  None,
  Thread,
};

class LLVMGen {
public:
  LLVMGen(uintptr_t);
  ~LLVMGen() = default;

  std::string dumpToString() const;
  void compileObjFile(std::string outfile, SanitizerKind san) const;

private:
  llvm::LLVMContext llvmCtx;
  std::unique_ptr<llvm::Module> llvmModule;
};

} // namespace llvmgen
} // namespace belalang

#endif // BELALANG_LLVMGEN_LLVMGEN_H_

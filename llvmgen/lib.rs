use cxx::UniquePtr;

#[cxx::bridge(namespace = "belalang::llvmgen")]
mod ffi {
    #[derive(Debug, Clone, Copy, PartialEq, Eq)]
    #[repr(u8)]
    enum SanitizerKind {
        None,
        Thread,
    }

    unsafe extern "C++" {
        include!("belalang/LLVMGen/CxxLLVMGen.h");

        type CxxLLVMGen;

        fn create_llvmgen(module_ptr: usize) -> UniquePtr<CxxLLVMGen>;
        fn dump_to_string(self: &CxxLLVMGen) -> String;
        fn compile_object_file(self: &CxxLLVMGen, out: String, sanitizer: SanitizerKind) -> String;
    }
}

pub use ffi::SanitizerKind;

pub struct LLVMGen {
    inner: UniquePtr<ffi::CxxLLVMGen>,
}

impl LLVMGen {
    pub fn new(module_ptr: usize) -> Self {
        Self {
            inner: ffi::create_llvmgen(module_ptr),
        }
    }

    pub fn dump_to_string(&self) -> String {
        self.inner.dump_to_string()
    }

    pub fn compile_object_file(&self, out: String, sanitizer: SanitizerKind) -> String {
        self.inner.compile_object_file(out, sanitizer)
    }
}

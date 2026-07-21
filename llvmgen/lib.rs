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
        include!("belalang/LLVMGen/LLVMGen.h");

        type LLVMGen;

        fn create_llvmgen(module_ptr: usize) -> UniquePtr<LLVMGen>;
        fn dump_to_string(self: &LLVMGen) -> String;
        fn compile_object_file(self: &LLVMGen, out: String, sanitizer: SanitizerKind) -> String;
    }
}

pub use ffi::SanitizerKind;

pub struct LLVMGen {
    inner: UniquePtr<ffi::LLVMGen>,
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

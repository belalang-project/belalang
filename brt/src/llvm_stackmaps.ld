SECTIONS {
  .llvm_stackmaps : ALIGN(8) {
    __start_llvm_stackmaps = .;
    KEEP(*(.llvm_stackmaps))
    __stop_llvm_stackmaps = .;
  }
}
INSERT AFTER .rodata;

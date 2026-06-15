#[unsafe(no_mangle)]
pub extern "C" fn brt_print_int(v: i32) {
    println!("{}", v)
}

#[unsafe(no_mangle)]
pub extern "C" fn brt_print_float(v: f32) {
    println!("{}", v)
}

#[unsafe(no_mangle)]
pub extern "C" fn brt_mmtk_init() {
    brt_core::mmtk::init();
}

#[unsafe(no_mangle)]
pub extern "C" fn brt_mmtk_alloc(size: usize) -> *mut u8 {
    brt_core::mmtk::alloc(size)
}

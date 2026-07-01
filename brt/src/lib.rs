use libc::{
    c_void,
    size_t,
};

unsafe extern "C" {
    fn GC_init();
    fn GC_malloc(size: size_t) -> *mut c_void;
}

#[unsafe(no_mangle)]
pub extern "C" fn brt_print_int(v: i64) {
    println!("{}", v)
}

#[unsafe(no_mangle)]
pub extern "C" fn brt_print_float(v: f64) {
    println!("{}", v)
}

#[unsafe(no_mangle)]
pub extern "C" fn brt_print_string(v: brt_core::string::BrString) {
    v.print();
}

#[unsafe(no_mangle)]
pub extern "C" fn brt_print_bool(v: bool) {
    println!("{}", v)
}

#[unsafe(no_mangle)]
pub extern "C" fn brt_mmtk_init() {
    unsafe {
        GC_init();
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn brt_mmtk_alloc(size: usize) -> *mut u8 {
    unsafe { GC_malloc(size) }.cast()
}

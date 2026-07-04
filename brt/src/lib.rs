use libc::{
    c_void,
    size_t,
};

unsafe extern "C" {
    fn GC_init();
    fn GC_malloc(size: size_t) -> *mut c_void;
}

#[repr(C)]
pub struct BrString {
    pub ptr: *const u8,
    pub len: u64,
}

impl BrString {
    pub fn print(&self) {
        unsafe {
            let bytes = std::slice::from_raw_parts(self.ptr, self.len as usize);
            if let Ok(s) = std::str::from_utf8(bytes) {
                println!("{}", s);
            } else {
                eprintln!("invalid");
            }
        }
    }
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
pub extern "C" fn brt_print_string(v: BrString) {
    v.print();
}

#[unsafe(no_mangle)]
pub extern "C" fn brt_print_bool(v: bool) {
    println!("{}", v)
}

#[unsafe(no_mangle)]
pub extern "C" fn brt_init() {
    unsafe {
        GC_init();
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn brt_gc_alloc(size: usize) -> *mut u8 {
    unsafe { GC_malloc(size) }.cast()
}

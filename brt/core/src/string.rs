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

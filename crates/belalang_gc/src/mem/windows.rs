use std::ptr;

use windows_sys::Win32::System::Memory::{
    MEM_COMMIT,
    MEM_RESERVE,
    PAGE_READWRITE,
    VirtualAlloc,
};

pub unsafe fn alloc_space(len: usize) -> ptr::NonNull<libc::c_void> {
    let addr = ptr::null_mut();
    let flags = MEM_COMMIT | MEM_RESERVE;
    let prot = PAGE_READWRITE;

    let raw_ptr = unsafe { VirtualAlloc(addr, len, flags, prot) };

    assert!(
        !raw_ptr.is_null(),
        "VirtualAlloc failed: {}",
        std::io::Error::last_os_error()
    );

    unsafe { ptr::NonNull::new_unchecked(raw_ptr) }
}

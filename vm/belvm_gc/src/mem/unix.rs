use std::ptr;

pub unsafe fn alloc_space(len: usize) -> ptr::NonNull<libc::c_void> {
    let addr = ptr::null_mut();
    let prot = libc::PROT_READ | libc::PROT_WRITE;
    let flags = libc::MAP_ANONYMOUS | libc::MAP_PRIVATE;
    let fd = -1;
    let offset = 0;

    let raw_ptr = unsafe { libc::mmap(addr, len, prot, flags, fd, offset) };

    if raw_ptr == libc::MAP_FAILED {
        panic!("mmap failed: {}", std::io::Error::last_os_error());
    }

    unsafe { ptr::NonNull::new_unchecked(raw_ptr) }
}

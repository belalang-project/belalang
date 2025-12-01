use std::{
    io,
    ptr,
};

use super::{
    StackMemory,
    StackValue,
};

pub(super) fn new_stack_memory(size: usize) -> io::Result<StackMemory> {
    unsafe {
        let page_size = libc::sysconf(libc::_SC_PAGESIZE);
        if page_size < 0 {
            return Err(io::Error::last_os_error());
        }
        let page_size = page_size as usize;

        let guard_size = page_size;
        let total_size = size + guard_size;

        let ptr = libc::mmap(
            ptr::null_mut(),
            total_size,
            libc::PROT_READ | libc::PROT_WRITE,
            libc::MAP_PRIVATE | libc::MAP_ANONYMOUS,
            -1, // no fd
            0,  // no offset
        );

        if ptr == libc::MAP_FAILED {
            return Err(io::Error::last_os_error());
        }

        if libc::mprotect(ptr, guard_size, libc::PROT_NONE) != 0 {
            let err = io::Error::last_os_error();
            libc::munmap(ptr, total_size);
            return Err(err);
        }

        let ptr = ptr.cast::<u8>();
        let stack_top = ptr.add(total_size).cast::<StackValue>();
        let stack_limit = ptr.add(page_size).cast::<StackValue>();

        Ok(StackMemory {
            mmap_ptr: ptr.cast::<StackValue>(),
            mmap_size: total_size,
            sp: stack_top,
            stack_top,
            stack_limit,
        })
    }
}

pub(super) fn drop_stack_memory(stack: &mut StackMemory) {
    unsafe {
        assert_eq!(
            libc::munmap(stack.mmap_ptr.cast::<libc::c_void>(), stack.mmap_size),
            0,
            "munmap failed"
        );
    }
}

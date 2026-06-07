use std::{
    io,
    mem,
    ptr,
};

use windows_sys::Win32::System::{
    Memory::{
        MEM_COMMIT,
        MEM_RELEASE,
        MEM_RESERVE,
        PAGE_NOACCESS,
        PAGE_READWRITE,
        VirtualAlloc,
        VirtualFree,
        VirtualProtect,
    },
    SystemInformation::{
        GetSystemInfo,
        SYSTEM_INFO,
    },
};

use super::{
    StackMemory,
    StackValue,
};

pub(super) fn new_stack_memory(size: usize) -> io::Result<StackMemory> {
    unsafe {
        let mut sys_info: SYSTEM_INFO = mem::zeroed();
        GetSystemInfo(&mut sys_info);
        let page_size = sys_info.dwPageSize as usize;

        let guard_size = page_size;
        let total_size = size + guard_size;

        let ptr = VirtualAlloc(ptr::null_mut(), total_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

        if ptr.is_null() {
            return Err(io::Error::last_os_error());
        }

        let mut old_protect = 0;

        if VirtualProtect(ptr, guard_size, PAGE_NOACCESS, &mut old_protect) == 0 {
            let err = io::Error::last_os_error();
            VirtualFree(ptr, 0, MEM_RELEASE);
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
        assert_ne!(
            VirtualFree(stack.mmap_ptr.cast(), 0, MEM_RELEASE),
            0,
            "VirtualFree failed"
        );
    }
}

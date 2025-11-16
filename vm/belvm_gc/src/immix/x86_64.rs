use std::{
    alloc::{
        Layout,
        alloc,
    },
    arch::asm,
    ptr,
};

#[thread_local]
static mut LOW_WATER_MARK: usize = 0;

const REGISTERS_COUNT: usize = 16;

pub unsafe fn get_stack_ptr() -> usize {
    let sp: usize;

    unsafe {
        asm!("mov {}, rsp", out(reg) sp, options(nostack));
    }

    sp
}

pub fn get_registers_count() -> i32 {
    REGISTERS_COUNT as i32
}

pub unsafe fn get_registers() -> ptr::NonNull<libc::c_void> {
    let layout = Layout::array::<usize>(REGISTERS_COUNT).expect("Could not create register array layout");

    let ptr = unsafe { alloc(layout) as *mut libc::c_void };

    unsafe {
        asm!(
            "mov [{ptr} + 0*8], rax",
            "mov [{ptr} + 1*8], rbx",
            "mov [{ptr} + 2*8], rcx",
            "mov [{ptr} + 3*8], rdx",
            "mov [{ptr} + 4*8], rbp",
            "mov [{ptr} + 5*8], rsp",
            "mov [{ptr} + 6*8], rsi",
            "mov [{ptr} + 7*8], rdi",
            "mov [{ptr} + 8*8], r8",
            "mov [{ptr} + 9*8], r9",
            "mov [{ptr} + 10*8], r10",
            "mov [{ptr} + 11*8], r11",
            "mov [{ptr} + 12*8], r12",
            "mov [{ptr} + 13*8], r13",
            "mov [{ptr} + 14*8], r14",
            "mov [{ptr} + 15*8], r15",
            ptr = in(reg) ptr,
            options(nostack)
        );
    }

    unsafe { ptr::NonNull::new_unchecked(ptr) }
}

pub unsafe fn set_low_water_mark() {
    unsafe {
        let rsp = get_stack_ptr();
        LOW_WATER_MARK = rsp;
    }
}

pub unsafe fn get_low_water_mark() -> usize {
    unsafe { LOW_WATER_MARK }
}

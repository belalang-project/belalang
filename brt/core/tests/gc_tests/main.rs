use std::mem;

use brt_core::gc::{
    gc_init,
    new_mutator,
};

#[test]
#[cfg(target_arch = "x86_64")]
fn simple() {
    #[repr(C)]
    struct Test(i32, i32);

    let t = Test(1, 2);

    gc_init(1 << 20, 1 << 20, 8);

    let mut mutator = new_mutator();
    let ptr = mutator
        .alloc_from_global(mem::size_of::<Test>(), mem::align_of::<Test>())
        .cast::<Test>()
        .as_ptr();

    unsafe { ptr.write(t) };
}

#[test]
fn mmtk_simple_alloc() {
    brt_core::mmtk::init();

    let ptr = brt_core::mmtk::alloc(64);
    assert!(!ptr.is_null());

    unsafe {
        std::ptr::write(ptr.cast::<i32>(), 42);
        assert_eq!(std::ptr::read(ptr.cast::<i32>()), 42);
    }
}

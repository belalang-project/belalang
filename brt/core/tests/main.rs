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

#[test]
fn mmtk_small_alloc() {
    brt_core::mmtk::init();

    // should be rounded up internallly
    let ptr = brt_core::mmtk::alloc(4);
    assert!(!ptr.is_null());

    unsafe {
        std::ptr::write(ptr.cast::<i32>(), 99);
        assert_eq!(std::ptr::read(ptr.cast::<i32>()), 99);
    }
}

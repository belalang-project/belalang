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

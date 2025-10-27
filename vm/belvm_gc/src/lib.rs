use std::cell::RefCell;

pub mod errors;
pub mod gc;
pub mod immix;

thread_local! {
    static HEAP: RefCell<gc::GcHeap> = RefCell::new(gc::GcHeap::default());
}

pub fn with_heap<F, R>(f: F) -> R
where
    F: FnOnce(&mut gc::GcHeap) -> R,
{
    HEAP.with(|h| f(&mut h.borrow_mut()))
}

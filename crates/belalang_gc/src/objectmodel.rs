use std::sync::atomic::{
    AtomicUsize,
    Ordering,
};

pub static MARK_STATE: AtomicUsize = AtomicUsize::new(0);

pub fn init() {
    MARK_STATE.store(1, Ordering::SeqCst);
}

pub fn flip_mark_state() {
    let mark_state = MARK_STATE.load(Ordering::SeqCst);
    if mark_state == 0 {
        MARK_STATE.store(1, Ordering::SeqCst);
    } else {
        MARK_STATE.store(0, Ordering::SeqCst);
    }
}

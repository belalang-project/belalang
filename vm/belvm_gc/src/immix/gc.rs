use crate::immix::mutator::mutators;

pub fn trigger_gc() {
    for m in mutators().write().unwrap().iter_mut() {
        if let Some(m) = m {
            m.set_take_yield(true);
        }
    }
}

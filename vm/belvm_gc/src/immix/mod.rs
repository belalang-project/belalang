use std::sync::{
    Arc,
    OnceLock,
    RwLock,
    atomic::{
        AtomicUsize,
        Ordering,
    },
};

use crate::immix::space::IxSpace;

pub mod freelist;
pub mod gc;
pub mod mutator;
pub mod objectmodel;
pub mod space;

#[cfg(target_arch = "x86_64")]
mod x86_64;

pub fn gc() -> &'static RwLock<Option<Gc>> {
    static GC: OnceLock<RwLock<Option<Gc>>> = OnceLock::new();
    GC.get_or_init(|| RwLock::new(None))
}

pub struct Gc {
    ix_space: Arc<IxSpace>,
}

pub const ALIGNMENT_VALUE: u8 = 1;
pub const IMMIX_SPACE_RATIO: f64 = 1.0 - LO_SPACE_RATIO;
pub const LO_SPACE_RATIO: f64 = 0.2;
pub const DEFAULT_HEAP_SIZE: usize = 500 << 20;

pub static IMMIX_SPACE_SIZE: AtomicUsize = AtomicUsize::new((DEFAULT_HEAP_SIZE as f64 * IMMIX_SPACE_RATIO) as usize);
pub static LO_SPACE_SIZE: AtomicUsize = AtomicUsize::new((DEFAULT_HEAP_SIZE as f64 * LO_SPACE_RATIO) as usize);

pub fn gc_init(ix_size: usize, lo_space: usize, n_gcthreads: usize) {
    IMMIX_SPACE_SIZE.store(ix_size, Ordering::SeqCst);
    LO_SPACE_SIZE.store(lo_space, Ordering::SeqCst);

    let ix_space = Arc::new(IxSpace::new(ix_size));
    gc::init(n_gcthreads, ix_space.clone());

    let mut gc_writer = gc().write().unwrap();
    *gc_writer = Some(Gc { ix_space });
    drop(gc_writer);

    objectmodel::init();
}

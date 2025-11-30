use std::sync::{
    Arc,
    OnceLock,
    RwLock,
    atomic::{
        AtomicIsize,
        AtomicUsize,
        Ordering,
    },
};

use crate::{
    mutator::{
        IxMutatorLocal,
        mutators,
    },
    space::IxSpace,
};

pub const LOG_POINTER_SIZE: usize = 3;
pub const POINTER_SIZE: usize = 1 << LOG_POINTER_SIZE;

static CONTROLLER: AtomicIsize = AtomicIsize::new(0);
static NO_CONTROLLER: isize = -1;

pub static GC_THREADS: AtomicUsize = AtomicUsize::new(0);

fn gc_context() -> &'static RwLock<GcContext> {
    static GC_CONTEXT: OnceLock<RwLock<GcContext>> = OnceLock::new();
    GC_CONTEXT.get_or_init(|| RwLock::new(GcContext { immix_space: None }))
}

pub struct GcContext {
    immix_space: Option<Arc<IxSpace>>,
}

pub fn init(n_gcthreads: usize, immix_space: Arc<IxSpace>) {
    GC_THREADS.store(n_gcthreads, Ordering::SeqCst);
    CONTROLLER.store(NO_CONTROLLER, Ordering::SeqCst);

    let mut ctx = gc_context().write().unwrap();
    ctx.immix_space = Some(immix_space);
    drop(ctx);
}

pub fn trigger_gc() {
    for m in mutators().write().unwrap().iter_mut().flatten() {
        m.set_take_yield(true);
    }
}

#[allow(unused_variables)]
pub fn sync_barrier(mutator: &mut IxMutatorLocal) {
    let controller_id = CONTROLLER
        .compare_exchange(-1, mutator.id() as isize, Ordering::SeqCst, Ordering::SeqCst)
        .unwrap();

    mutator.prepare_for_gc();
}

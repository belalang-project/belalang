use std::sync::{
    Arc,
    OnceLock,
    RwLock,
    atomic::{
        AtomicBool,
        Ordering,
    },
};

use crate::immix::space::IxSpace;

type Mutator = RwLock<Vec<Option<Arc<IxMutatorGlobal>>>>;
const MAX_MUTATORS: usize = 1024;

fn mutators() -> &'static Mutator {
    static MUTATORS: OnceLock<Mutator> = OnceLock::new();
    MUTATORS.get_or_init(|| {
        let mut mutators = Vec::with_capacity(MAX_MUTATORS);
        for _ in 0..MAX_MUTATORS {
            mutators.push(None);
        }
        RwLock::new(mutators)
    })
}

fn n_mutators() -> &'static RwLock<usize> {
    static N_MUTATORS: OnceLock<RwLock<usize>> = OnceLock::new();
    N_MUTATORS.get_or_init(|| RwLock::new(0))
}

pub struct IxMutatorLocal {
    id: usize,
    global: Arc<IxMutatorGlobal>,
    space: Arc<IxSpace>,
}

impl IxMutatorLocal {
    pub fn new(space: Arc<IxSpace>) -> Self {
        let global = Arc::new(IxMutatorGlobal::new());
        let mut id_lock = n_mutators().write().unwrap();

        let mut mutators_lock = mutators().write().unwrap();
        mutators_lock.remove(*id_lock);
        mutators_lock.insert(*id_lock, Some(global.clone()));
        drop(mutators_lock);

        let mutator = Self {
            id: *id_lock,
            global,
            space,
        };

        *id_lock += 1;

        mutator
    }
}

pub struct IxMutatorGlobal {
    take_yield: AtomicBool,
    still_blocked: AtomicBool,
}

impl IxMutatorGlobal {
    pub fn new() -> Self {
        Self {
            take_yield: AtomicBool::new(false),
            still_blocked: AtomicBool::new(false),
        }
    }

    #[inline(always)]
    pub fn is_still_blocked(&self) -> bool {
        self.still_blocked.load(Ordering::SeqCst)
    }

    #[inline(always)]
    pub fn set_still_blocked(&self, val: bool) {
        self.still_blocked.store(val, Ordering::SeqCst);
    }

    #[inline(always)]
    pub fn take_yield(&self) -> bool {
        self.take_yield.load(Ordering::SeqCst)
    }

    #[inline(always)]
    pub fn set_take_yield(&self, val: bool) {
        self.still_blocked.store(val, Ordering::SeqCst);
    }
}

use std::{
    ptr,
    sync::{
        Arc,
        OnceLock,
        RwLock,
        atomic::{
            AtomicBool,
            Ordering,
        },
    },
};

use crate::space::{
    IxBlock,
    IxSpace,
    LINES_IN_BLOCK,
    LOG_BYTES_IN_LINE,
    LineMark,
};

type Mutator = RwLock<Vec<Option<Arc<IxMutatorGlobal>>>>;
const MAX_MUTATORS: usize = 1024;

pub(crate) fn mutators() -> &'static Mutator {
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
    cursor: ptr::NonNull<libc::c_void>,
    limit: ptr::NonNull<libc::c_void>,
    line: usize,
    global: Arc<IxMutatorGlobal>,
    space: Arc<IxSpace>,
    block: Option<Box<IxBlock>>,
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
            cursor: ptr::NonNull::dangling(),
            limit: ptr::NonNull::dangling(),
            line: LINES_IN_BLOCK,
            global,
            space,
            block: None,
        };

        *id_lock += 1;

        mutator
    }

    pub fn alloc(&mut self, size: usize, align: usize) -> ptr::NonNull<libc::c_void> {
        let (start, end) = unsafe {
            let offset = self.cursor.align_offset(align);
            let start = self.cursor.add(offset);
            (start, start.add(size))
        };

        if end > self.limit {
            self.try_alloc_from_local(size, align)
        } else {
            self.cursor = end;
            start
        }
    }

    pub fn try_alloc_from_local(&mut self, size: usize, align: usize) -> ptr::NonNull<libc::c_void> {
        if self.line < LINES_IN_BLOCK {
            let opt_next_available_line = {
                let curr_line = self.line;
                self.block().get_next_available_line(curr_line)
            };

            if let Some(next_available_line) = opt_next_available_line {
                let end_line = self.block().get_next_unavailable_line(next_available_line);

                unsafe {
                    self.cursor = self.block().start().add(next_available_line << LOG_BYTES_IN_LINE);
                    self.limit = self.block().start().add(end_line << LOG_BYTES_IN_LINE);
                    self.line = end_line;

                    let offset = self.limit.offset_from_unsigned(self.cursor);
                    ptr::write_bytes(self.cursor.as_ptr(), 0, offset);
                }

                for line in next_available_line..end_line {
                    self.block().line_mark_table_mut().set(line, LineMark::FreshAlloc);
                }

                return self.alloc(size, align);
            }
        }

        self.alloc_from_global(size, align)
    }

    pub fn alloc_from_global(&mut self, size: usize, align: usize) -> ptr::NonNull<libc::c_void> {
        self.return_block();

        loop {
            self.yieldpoint();

            let new_block = self.space.get_next_usable_block();

            match new_block {
                Some(b) => {
                    self.block = Some(b);
                    self.cursor = self.block().start();
                    self.limit = self.block().start();
                    self.line = 0;

                    return self.alloc(size, align);
                },
                None => continue,
            }
        }
    }

    pub fn id(&self) -> usize {
        self.id
    }

    fn block(&mut self) -> &mut IxBlock {
        self.block.as_mut().unwrap()
    }

    pub fn prepare_for_gc(&mut self) {
        self.return_block();
    }

    fn return_block(&mut self) {
        if self.block.is_some() {
            self.space.return_used_block(self.block.take().unwrap());
        }
    }

    pub fn yieldpoint(&mut self) {
        if self.global.take_yield() {
            self.yieldpoint_slow();
        }
    }

    pub fn yieldpoint_slow(&mut self) {
        todo!("sync_barrier")
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

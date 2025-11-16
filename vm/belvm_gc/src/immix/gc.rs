use std::{
    ptr,
    sync::{
        Arc,
        OnceLock,
        RwLock,
        atomic::{
            AtomicIsize,
            Ordering,
        },
    },
};

use crate::immix::{
    mutator::{
        IxMutatorLocal,
        mutators,
    },
    space::IxSpace,
    x86_64::{
        self,
        get_registers,
        get_registers_count,
    },
};

pub const LOG_POINTER_SIZE: usize = 3;
pub const POINTER_SIZE: usize = 1 << LOG_POINTER_SIZE;

static CONTROLLER: AtomicIsize = AtomicIsize::new(0);

fn gc_context() -> &'static RwLock<GcContext> {
    static GC_CONTEXT: OnceLock<RwLock<GcContext>> = OnceLock::new();
    GC_CONTEXT.get_or_init(|| RwLock::new(GcContext { immix_space: None }))
}

pub struct GcContext {
    immix_space: Option<Arc<IxSpace>>,
}

pub fn trigger_gc() {
    for m in mutators().write().unwrap().iter_mut() {
        if let Some(m) = m {
            m.set_take_yield(true);
        }
    }
}

pub fn stack_scan() -> Vec<ptr::NonNull<libc::c_void>> {
    let stack_ptr = unsafe {
        let sp_usize = x86_64::get_stack_ptr();
        let sp_ptr = sp_usize as *mut libc::c_void;
        ptr::NonNull::new_unchecked(sp_ptr)
    };
    let low_water_mark = unsafe {
        let sp_usize = x86_64::get_low_water_mark();
        let sp_ptr = sp_usize as *mut libc::c_void;
        ptr::NonNull::new_unchecked(sp_ptr)
    };

    let mut cursor = stack_ptr;
    let mut result = Vec::new();

    let ctx = gc_context().read().unwrap();
    let ix_space = ctx.immix_space.as_ref().unwrap();

    while cursor < low_water_mark {
        let value = cursor;

        if is_valid_object(value, ix_space.start(), ix_space.end()) {
            result.push(value);
        }

        cursor = unsafe { cursor.add(POINTER_SIZE) };
    }

    let reg_count = get_registers_count();
    let regs = unsafe { get_registers() };

    for i in 0..reg_count {
        let value = unsafe { regs.offset(i as isize) };

        if is_valid_object(value, ix_space.start(), ix_space.end()) {
            result.push(value);
        }
    }

    return result;

    fn is_valid_object(
        addr: ptr::NonNull<libc::c_void>,
        start: ptr::NonNull<libc::c_void>,
        end: ptr::NonNull<libc::c_void>,
    ) -> bool {
        if addr >= end || addr < start {
            return false;
        }

        true
    }
}

pub fn sync_barrier(mutator: &mut IxMutatorLocal) {
    let controller_id = CONTROLLER
        .compare_exchange(-1, mutator.id() as isize, Ordering::SeqCst, Ordering::SeqCst)
        .unwrap();

    mutator.prepare_for_gc();

    let mut thread_roots = stack_scan();
    todo!("Append thread_roots to roots");
}

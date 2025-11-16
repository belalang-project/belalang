use std::{
    collections::LinkedList,
    ptr,
    sync::Mutex,
};

use super::gc;

pub const LOG_BYTES_IN_LINE: usize = 8;
pub const LOG_BYTES_IN_BLOCK: usize = 16;

pub const BYTES_IN_LINE: usize = 1 << LOG_BYTES_IN_LINE;
pub const BYTES_IN_BLOCK: usize = 1 << LOG_BYTES_IN_BLOCK;
pub const LINES_IN_BLOCK: usize = 1 << (LOG_BYTES_IN_BLOCK - LOG_BYTES_IN_LINE);

#[derive(PartialEq, Eq, Debug, Copy, Clone)]
pub enum LineMark {
    Free,
    Live,
    FreshAlloc,
    ConservLive,
    PrevLive,
}

#[derive(PartialEq, Eq, Debug, Copy, Clone)]
pub enum BlockMark {
    Usable,
    Full,
}

#[derive(Clone)]
pub struct LineMarkTable {
    space_start: ptr::NonNull<libc::c_void>,
    ptr: *mut LineMark,
    len: usize,
}

impl LineMarkTable {
    pub fn new(space_start: ptr::NonNull<libc::c_void>, space_end: ptr::NonNull<libc::c_void>) -> Self {
        let offset = unsafe { space_end.offset_from(space_start) };
        let len = offset.unsigned_abs() / BYTES_IN_LINE;

        let line_mark_table = unsafe {
            let size: libc::size_t = std::mem::size_of::<LineMark>() * len;
            libc::malloc(size) as *mut LineMark
        };

        let mut cursor = line_mark_table;
        for _ in 0..len {
            unsafe { *cursor = LineMark::Free };
            cursor = unsafe { cursor.add(1) };
        }

        Self {
            space_start,
            ptr: line_mark_table,
            len,
        }
    }

    pub fn take_slice(&mut self, start: usize, len: usize) -> LineMarkTableSlice {
        let ptr = unsafe { self.ptr.offset(start as isize) };
        LineMarkTableSlice { ptr, len }
    }

    #[inline(always)]
    pub fn get(&self, index: usize) -> LineMark {
        unsafe { *self.ptr.add(index) }
    }
}

#[derive(Clone)]
pub struct LineMarkTableSlice {
    ptr: *mut LineMark,
    len: usize,
}

impl LineMarkTableSlice {
    #[inline(always)]
    pub fn get(&self, index: usize) -> LineMark {
        unsafe { *self.ptr.add(index) }
    }

    #[inline(always)]
    pub fn set(&mut self, index: usize, value: LineMark) {
        unsafe { *self.ptr.add(index) = value };
    }
}

pub struct IxSpace {
    mem_start: ptr::NonNull<libc::c_void>,
    mem_end: ptr::NonNull<libc::c_void>,
    blocks_usable: Mutex<LinkedList<Box<IxBlock>>>,
    blocks_used: Mutex<LinkedList<Box<IxBlock>>>,
    line_mark_table: LineMarkTable,
}

unsafe impl Send for IxSpace {}
unsafe impl Sync for IxSpace {}

impl IxSpace {
    pub fn new(len: usize) -> Self {
        let addr = ptr::null_mut();
        let prot = libc::PROT_READ | libc::PROT_WRITE;
        let flags = libc::MAP_ANONYMOUS;
        let fd = -1;
        let offset = 0;

        let (mem_start, mem_end) = unsafe {
            let raw_ptr = libc::mmap(addr, len, prot, flags, fd, offset);
            let start = ptr::NonNull::new_unchecked(raw_ptr);
            let end = start.add(len);
            (start, end)
        };

        let line_mark_table = LineMarkTable::new(mem_start, mem_end);

        let mut space = Self {
            mem_start,
            mem_end,
            blocks_usable: Mutex::new(LinkedList::new()),
            blocks_used: Mutex::new(LinkedList::new()),
            line_mark_table,
        };

        let mut blocks_usable = space.blocks_usable.lock().unwrap();
        let mut block_start = mem_start.clone();
        let mut line = 0;
        while unsafe { block_start.add(BYTES_IN_BLOCK) } <= mem_end {
            let block = IxBlock {
                state: BlockMark::Usable,
                start: block_start,
                line_mark_table: space.line_mark_table.take_slice(line, LINES_IN_BLOCK),
            };
            blocks_usable.push_back(Box::new(block));
            block_start = unsafe { block_start.add(BYTES_IN_BLOCK) };
            line += LINES_IN_BLOCK;
        }
        drop(blocks_usable);

        space
    }

    pub fn return_used_block(&self, old: Box<IxBlock>) {
        self.blocks_used.lock().unwrap().push_front(old);
    }

    pub fn get_next_usable_block(&self) -> Option<Box<IxBlock>> {
        let res_new_block = self.blocks_usable.lock().unwrap().pop_front();

        if res_new_block.is_none() {
            gc::trigger_gc();
            None
        } else {
            res_new_block
        }
    }

    pub fn start(&self) -> ptr::NonNull<libc::c_void> {
        self.mem_start
    }

    pub fn end(&self) -> ptr::NonNull<libc::c_void> {
        self.mem_end
    }
}

pub struct IxBlock {
    state: BlockMark,
    start: ptr::NonNull<libc::c_void>,
    line_mark_table: LineMarkTableSlice,
}

impl IxBlock {
    pub fn get_next_available_line(&self, curr_line: usize) -> Option<usize> {
        let mut i = curr_line;
        while i < self.line_mark_table.len {
            if let LineMark::Free = self.line_mark_table.get(i) {
                return Some(i);
            }
            i += 1;
        }
        None
    }

    pub fn get_next_unavailable_line(&self, curr_line: usize) -> usize {
        let mut i = curr_line;
        while i < self.line_mark_table.len {
            match self.line_mark_table.get(i) {
                LineMark::Free => i += 1,
                _ => return i,
            }
        }
        i
    }

    pub fn line_mark_table_mut(&mut self) -> &mut LineMarkTableSlice {
        &mut self.line_mark_table
    }

    pub fn start(&self) -> ptr::NonNull<libc::c_void> {
        self.start
    }
}

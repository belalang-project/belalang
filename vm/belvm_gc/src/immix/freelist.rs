use std::{
    alloc::{
        self,
        Layout,
    },
    collections::LinkedList,
    ptr,
    sync::{
        Arc,
        RwLock,
    },
};

use crate::immix::{
    gc,
    mutator::IxMutatorLocal,
};

pub struct FreeListSpace {
    current_nodes: LinkedList<Box<FreeListNode>>,
    node_id: usize,
    size: usize,
    used_bytes: usize,
}

impl FreeListSpace {
    pub fn new(size: usize) -> Self {
        Self {
            current_nodes: LinkedList::new(),
            node_id: 0,
            size,
            used_bytes: 0,
        }
    }

    pub fn alloc(&mut self, size: usize, align: usize) -> Option<ptr::NonNull<libc::c_void>> {
        if self.used_bytes + size > self.size {
            None
        } else {
            let layout = Layout::from_size_align(size, align).ok()?;

            let ptr = unsafe {
                let ptr = alloc::alloc(layout) as *mut libc::c_void;
                ptr::NonNull::new_unchecked(ptr)
            };

            self.current_nodes.push_front(Box::new(FreeListNode {
                id: self.node_id,
                start: ptr,
                layout,
                size,
                mark: NodeMark::FreshAlloc,
            }));

            self.node_id += 1;
            self.used_bytes += size;

            Some(ptr)
        }
    }

    pub fn sweep(&mut self) {
        let (new_nodes, new_used_bytes) = {
            let mut ret = LinkedList::new();
            let nodes = &mut self.current_nodes;
            let mut used_bytes = 0;

            while !nodes.is_empty() {
                let mut node = nodes.pop_front().unwrap();
                match node.mark {
                    NodeMark::Live => {
                        node.set_mark(NodeMark::PrevLive);
                        used_bytes += node.size;
                        ret.push_back(node);
                    },
                    NodeMark::PrevLive | NodeMark::FreshAlloc => {
                        unsafe { alloc::dealloc(node.start.as_ptr() as *mut u8, node.layout) };
                    },
                }
            }

            (ret, used_bytes)
        };

        self.current_nodes = new_nodes;
        self.used_bytes = new_used_bytes;
    }

    pub fn current_nodes(&self) -> &LinkedList<Box<FreeListNode>> {
        &self.current_nodes
    }

    pub fn current_nodes_mut(&mut self) -> &mut LinkedList<Box<FreeListNode>> {
        &mut self.current_nodes
    }
}

pub struct FreeListNode {
    id: usize,
    start: ptr::NonNull<libc::c_void>,
    layout: Layout,
    size: usize,
    mark: NodeMark,
}

impl FreeListNode {
    pub fn set_mark(&mut self, mark: NodeMark) {
        self.mark = mark;
    }
}

#[derive(PartialEq, Eq, Copy, Clone, Debug)]
pub enum NodeMark {
    FreshAlloc,
    PrevLive,
    Live,
}

pub fn alloc_large(
    size: usize,
    align: usize,
    mutator: &mut IxMutatorLocal,
    space: Arc<RwLock<FreeListSpace>>,
) -> ptr::NonNull<libc::c_void> {
    loop {
        mutator.yieldpoint();

        let addr = {
            let mut lo_space_lock = space.write().unwrap();
            lo_space_lock.alloc(size, align)
        };

        match addr {
            Some(addr) => {
                return addr;
            },
            None => {
                gc::trigger_gc();
            },
        }
    }
}

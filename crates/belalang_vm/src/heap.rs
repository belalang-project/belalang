use std::ptr;

use crate::objectmodel::{
    ObjectMethod,
    ObjectModel,
};

pub struct HeapObject {
    marked: bool,
    next: *mut HeapObject,
    pub value: Box<dyn ObjectModel>,
    pub methods: Vec<ObjectMethod>,
}

pub struct HeapMemory {
    head: *mut HeapObject,
    pub n_objects: i32,
}

impl Default for HeapMemory {
    fn default() -> Self {
        Self::new()
    }
}

impl HeapMemory {
    pub fn new() -> Self {
        Self {
            head: ptr::null_mut(),
            n_objects: 0,
        }
    }

    pub fn new_object(&mut self, value: Box<dyn ObjectModel>, methods: Vec<ObjectMethod>) -> *mut HeapObject {
        let ptr = Box::into_raw(Box::new(HeapObject {
            marked: false,
            next: self.head,
            value,
            methods,
        }));

        self.head = ptr;
        self.n_objects += 1;

        ptr
    }

    pub fn mark(&mut self, ptr: *mut HeapObject) {
        let obj = unsafe { &mut *ptr };

        if obj.marked {
            return;
        }

        obj.marked = true;
    }

    pub fn sweep(&mut self) {
        let mut ptr_to_curr = &mut self.head;

        while !(*ptr_to_curr).is_null() {
            let curr = *ptr_to_curr;
            let obj = unsafe { &mut *curr };

            if obj.marked {
                obj.marked = false;
                ptr_to_curr = &mut obj.next;
            } else {
                *ptr_to_curr = obj.next;
                let _dropped = unsafe { Box::from_raw(curr) };
                self.n_objects -= 1;
            }
        }
    }
}

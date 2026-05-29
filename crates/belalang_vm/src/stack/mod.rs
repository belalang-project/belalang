use std::{
    fmt::Display,
    io,
    ptr,
};

use crate::heap::{
    HeapObject,
    HeapValue,
};

cfg_select! {
    target_family = "unix" => {
        mod unix;
        use unix as sys;
    }
    target_os = "windows" => {
        mod windows;
        use windows as sys;
    }
    _ => {
        compile_error!("Unsupported platform. Only Unix-like systems and Windows are currently supported.");
    }
}

const STACK_SIZE: usize = 4096;

/// Values that live on the stack
#[derive(Default, Debug)]
pub enum StackValue {
    Boolean(bool),
    Integer(i64),

    /// Pointer to an address in the bytecode
    AddressPtr(u8),

    /// Pointer to object in the heap memory
    ObjectPtr(*mut HeapObject),

    /// Null value in the stack
    ///
    /// This value is mostly used to indicate uninitialized variables and actual
    /// null values.
    ///
    /// See <https://github.com/belalang-project/belalang/issues/14>
    #[default]
    Null,
}

impl Display for StackValue {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::Boolean(b) => write!(f, "{b}"),
            Self::Integer(i) => write!(f, "{i}"),
            Self::AddressPtr(addr) => write!(f, "ptr:{addr}"),
            Self::ObjectPtr(ptr) => {
                if ptr.is_null() {
                    return write!(f, "<null-obj-ptr>");
                }

                let obj = unsafe { &*(*ptr) };

                match &obj.value {
                    HeapValue::String(s) => write!(f, "{s}"),
                }
            },
            Self::Null => write!(f, "<null>"),
        }
    }
}

#[derive(thiserror::Error, Debug)]
pub enum StackError {
    #[error("stack underflow")]
    StackUnderflow,

    #[error("stack overflow")]
    StackOverflow,
}

pub(crate) struct StackMemory {
    mmap_ptr: *mut StackValue,
    mmap_size: usize,

    sp: *mut StackValue,
    stack_top: *mut StackValue,
    stack_limit: *mut StackValue,
}

impl Default for StackMemory {
    fn default() -> Self {
        Self::new(STACK_SIZE).unwrap()
    }
}

impl StackMemory {
    pub fn new(size: usize) -> io::Result<Self> {
        sys::new_stack_memory(size)
    }

    pub fn push(&mut self, data: StackValue) -> Result<(), StackError> {
        unsafe {
            let new_sp = self.sp.sub(1);

            if new_sp < self.stack_limit {
                return Err(StackError::StackOverflow);
            }

            ptr::write_unaligned(new_sp, data);
            self.sp = new_sp;
            Ok(())
        }
    }

    pub fn pop(&mut self) -> Result<StackValue, StackError> {
        unsafe {
            let new_sp = self.sp.add(1);

            if new_sp > self.stack_top {
                return Err(StackError::StackUnderflow);
            }

            let data = ptr::read_unaligned(self.sp);
            self.sp = new_sp;
            Ok(data)
        }
    }

    #[allow(dead_code)]
    pub fn top(&self) -> Result<StackValue, StackError> {
        unsafe { Ok(ptr::read_unaligned(self.sp)) }
    }

    /// The number of allocated [`StackValue`].
    pub fn size(&self) -> usize {
        unsafe { self.stack_top.offset_from_unsigned(self.sp) }
    }

    pub fn iter<'a>(&'a self) -> StackMemoryIter<'a> {
        StackMemoryIter {
            stack: self,
            curr: self.sp,
        }
    }
}

impl Drop for StackMemory {
    fn drop(&mut self) {
        sys::drop_stack_memory(self);
    }
}

pub struct StackMemoryIter<'a> {
    stack: &'a StackMemory,
    curr: *mut StackValue,
}

impl<'a> Iterator for StackMemoryIter<'a> {
    type Item = StackValue;

    fn next(&mut self) -> Option<Self::Item> {
        if unsafe { self.curr.offset_from(self.stack.stack_top) } == 0 {
            return None;
        }

        if !self.curr.is_null() {
            let item = unsafe { self.curr.read() };
            self.curr = unsafe { self.curr.add(1) };
            Some(item)
        } else {
            None
        }
    }
}

#[cfg(test)]
mod tests {
    use std::{
        assert_matches::assert_matches,
        mem,
    };

    use super::{
        StackError,
        StackMemory,
        StackValue,
    };

    #[test]
    fn simple() {
        let mut stack = StackMemory::new(mem::size_of::<StackValue>() * 64).unwrap();

        stack.push(StackValue::Integer(1)).unwrap();
        stack.push(StackValue::Integer(2)).unwrap();

        assert_matches!(stack.top().unwrap(), StackValue::Integer(2));
        assert_matches!(stack.pop().unwrap(), StackValue::Integer(2));
        assert_matches!(stack.pop().unwrap(), StackValue::Integer(1));
    }

    #[test]
    fn overflow() {
        let mut stack = StackMemory::new(mem::size_of::<StackValue>() * 2).unwrap();

        stack.push(StackValue::Integer(1)).unwrap();
        stack.push(StackValue::Integer(2)).unwrap();

        let err = stack.push(StackValue::Integer(3)).unwrap_err();
        assert_matches!(err, StackError::StackOverflow);
    }

    #[test]
    fn underflow() {
        let mut stack = StackMemory::new(mem::size_of::<StackValue>() * 2).unwrap();
        let err = stack.pop().unwrap_err();
        assert_matches!(err, StackError::StackUnderflow);
    }

    #[test]
    fn iterator() {
        let mut stack = StackMemory::new(mem::size_of::<StackValue>() * 2).unwrap();

        stack.push(StackValue::Integer(1)).unwrap();
        stack.push(StackValue::Integer(2)).unwrap();

        let mut iter = stack.iter();
        assert_matches!(iter.next(), Some(StackValue::Integer(2)));
        assert_matches!(iter.next(), Some(StackValue::Integer(1)));
        assert_matches!(iter.next(), None);
    }
}

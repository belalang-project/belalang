use std::{
    alloc::{
        Layout,
        alloc,
    },
    cell::Cell,
    fmt::Debug,
    hash::{
        DefaultHasher,
        Hash,
        Hasher,
    },
    marker::PhantomData,
    ops::{
        Deref,
        DerefMut,
    },
    ptr::{
        NonNull,
        drop_in_place,
    },
};

use crate::errors::MemoryError;

#[derive(Clone)]
pub struct GcObjectHeader {
    pub obj_type: u32,
    pub ref_count: Cell<usize>,
    pub is_marked: bool,
    pub next: Option<NonNull<dyn GcObject>>,
}

impl GcObjectHeader {
    pub fn new<T: GcObject>() -> Self {
        Self {
            obj_type: T::r#type(),
            ref_count: Cell::new(0),
            is_marked: false,
            next: None,
        }
    }
}

pub trait GcObject {
    fn header(&self) -> &GcObjectHeader;
    fn header_mut(&mut self) -> &mut GcObjectHeader;

    fn type_name() -> String
    where
        Self: Sized;

    fn r#type() -> u32
    where
        Self: Sized,
    {
        let mut hasher = DefaultHasher::new();
        Self::type_name().hash(&mut hasher);
        let hash = hasher.finish();
        hash as u32
    }
}

#[derive(Default)]
pub struct GcHeap {
    pub start: Option<NonNull<dyn GcObject>>,
    _marker: PhantomData<GcObjectHeader>,
}

impl GcHeap {
    pub fn alloc<T: GcObject>(&mut self, object: T) -> Result<GcPtr<T>, MemoryError> {
        let layout = Layout::new::<T>();

        let base_ptr: *mut T = unsafe {
            let ptr = alloc(layout) as *mut T;

            if ptr.is_null() {
                return Err(MemoryError::AllocationFailed);
            }

            ptr.write(object);

            ptr
        };

        // Safety: base_ptr was just created in this function call
        let ptr = base_ptr as *mut dyn GcObject;
        unsafe { (*ptr).header_mut().next = self.start };

        // Safety: base_ptr was just created in this function call
        unsafe { self.start = Some(NonNull::new_unchecked(ptr)) };

        // Safety: base_ptr was just created in this function call
        unsafe { Ok(GcPtr::new(&mut *base_ptr)) }
    }
}

impl Drop for GcHeap {
    fn drop(&mut self) {
        self.start = None;
    }
}

pub struct GcPtr<T: GcObject + ?Sized> {
    ptr: NonNull<T>,
}

impl<T: GcObject + ?Sized> GcPtr<T> {
    pub fn new(ptr: &mut T) -> Self {
        let base = ptr.header();
        base.ref_count.set(base.ref_count.get() + 1);

        // Safety: pointer is valid since we just received it
        Self {
            ptr: unsafe { NonNull::new_unchecked(ptr) },
        }
    }

    pub fn as_ptr(&self) -> *mut T {
        self.ptr.as_ptr()
    }
}

impl<T: GcObject + ?Sized> Deref for GcPtr<T> {
    type Target = T;

    fn deref(&self) -> &Self::Target {
        // Safety: we maintain the invariant that ptr is always valid when GcPtr is
        // created
        unsafe { self.ptr.as_ref() }
    }
}

impl<T: GcObject + ?Sized> DerefMut for GcPtr<T> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        // Safety: we maintain the invariant that ptr is always valid when GcPtr is
        // created
        unsafe { self.ptr.as_mut() }
    }
}

impl<T: GcObject + ?Sized> Debug for GcPtr<T> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{:?}", self.ptr)
    }
}

impl<T: GcObject + ?Sized> Clone for GcPtr<T> {
    fn clone(&self) -> Self {
        let base = self.header();
        base.ref_count.set(base.ref_count.get() + 1);

        Self { ptr: self.ptr }
    }
}

impl<T: GcObject + ?Sized> Drop for GcPtr<T> {
    fn drop(&mut self) {
        let base = self.header();
        base.ref_count.set(base.ref_count.get() - 1);

        if base.ref_count.get() == 0 {
            // Safety: we just checked that ref count is zero
            unsafe { drop_in_place(self.ptr.as_ptr()) };
        }
    }
}

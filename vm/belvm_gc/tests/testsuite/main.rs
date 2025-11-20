use belvm_gc::{
    gc::{
        GcHeap,
        GcObject,
        GcObjectHeader,
    },
    with_heap,
};

mod immix;

#[derive(Clone)]
struct Integer {
    header: GcObjectHeader,
    value: i64,
}

impl Integer {
    pub fn new(value: i64) -> Integer {
        Self {
            header: GcObjectHeader::new::<Self>(),
            value,
        }
    }
}

impl GcObject for Integer {
    fn header(&self) -> &GcObjectHeader {
        &self.header
    }

    fn header_mut(&mut self) -> &mut GcObjectHeader {
        &mut self.header
    }

    fn type_name() -> String
    where
        Self: Sized,
    {
        String::from("Integer")
    }
}

#[derive(Clone)]
struct Float {
    header: GcObjectHeader,
    value: f64,
}

impl Float {
    pub fn new(value: f64) -> Float {
        Self {
            header: GcObjectHeader::new::<Self>(),
            value,
        }
    }
}

impl GcObject for Float {
    fn header(&self) -> &GcObjectHeader {
        &self.header
    }

    fn header_mut(&mut self) -> &mut GcObjectHeader {
        &mut self.header
    }

    fn type_name() -> String
    where
        Self: Sized,
    {
        String::from("Integer")
    }
}

#[test]
fn increments_ref_count() {
    let int = with_heap(|heap| heap.alloc(Integer::new(1)).unwrap());
    assert_eq!(int.header().ref_count.get(), 1);
}

#[test]
#[allow(unused_variables)]
fn increments_ref_count_2() {
    let int = with_heap(|heap| heap.alloc(Integer::new(1)).unwrap());
    let int2 = int.clone(); // should increment the ref_count to 2

    assert_eq!(int.header().ref_count.get(), 2);
}

#[test]
fn drop_decrements_ref_count() {
    let int = with_heap(|heap| heap.alloc(Integer::new(1)).unwrap());
    let int2 = int.clone();

    assert_eq!(int.header().ref_count.get(), 2); // from 2
    assert_eq!(int2.header().ref_count.get(), 2);

    drop(int2);

    assert_eq!(int.header().ref_count.get(), 1); // to 1
}

#[test]
fn test_heap_allocations() {
    let mut heap = GcHeap::default();

    heap.alloc(Integer::new(1)).unwrap();
    heap.alloc(Float::new(2.0)).unwrap();
    heap.alloc(Integer::new(3)).unwrap();

    let current = heap.start.unwrap();

    let c = unsafe { &*(current.as_ptr() as *const Integer) };
    assert_eq!(c.header().obj_type, Integer::r#type());
    assert_eq!(c.value, 3);
    let current = c.header().next.unwrap();

    let c = unsafe { &*(current.as_ptr() as *const Float) };
    assert_eq!(c.header().obj_type, Float::r#type());
    assert_eq!(c.value, 2.0);
    let current = c.header().next.unwrap();

    let c = unsafe { &*(current.as_ptr() as *const Integer) };
    assert_eq!(c.header().obj_type, Integer::r#type());
    assert_eq!(c.value, 1);
    let current = c.header().next;

    assert!(current.is_none(), "Heap has more elements than expected");
}

#[test]
fn test_heap_drop() {
    let mut heap = GcHeap::default();

    heap.alloc(Integer::new(1)).unwrap();
    heap.alloc(Float::new(2.0)).unwrap();
    heap.alloc(Integer::new(3)).unwrap();

    drop(heap); // simulate dropping the heap

    // no assertions needed --- if it doesn't crash, the test passes
}

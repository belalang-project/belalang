use std::fmt::Display;

use super::{
    ObjectMethod,
    ObjectModel,
};

pub static STRING_METHODS: [ObjectMethod; 1] = [impl_string_len];

pub struct BelalangString {
    inner: String,
}

impl BelalangString {
    pub fn new(s: String) -> Self {
        Self { inner: s }
    }
}

impl ObjectModel for BelalangString {
    fn as_any(&self) -> &dyn std::any::Any {
        self
    }
}

impl Display for BelalangString {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.inner)
    }
}

fn impl_string_len(s: &mut Box<dyn ObjectModel>) -> usize {
    let s = s.as_any().downcast_ref::<BelalangString>().unwrap();
    s.inner.len()
}

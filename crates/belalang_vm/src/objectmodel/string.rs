use std::fmt::Display;

use super::ObjectModel;

pub struct BelalangString {
    inner: String,
}

impl BelalangString {
    pub fn new(s: String) -> Self {
        Self { inner: s }
    }
}

impl ObjectModel for BelalangString {}

impl Display for BelalangString {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.inner)
    }
}

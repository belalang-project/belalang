mod string;

use std::{
    any::Any,
    fmt::Display,
};

pub use string::*;

pub trait ObjectModel: Display + Any {
    fn as_any(&self) -> &dyn Any;
}

pub type ObjectMethod = fn(instance: &mut Box<dyn ObjectModel>) -> usize;

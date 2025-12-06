#![feature(assert_matches, cfg_select)]

mod core;
pub mod errors;
mod fs;
mod heap;
mod io;
pub mod stack;

pub use core::VM;

pub use io::VMIO;

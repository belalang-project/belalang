pub mod interner;

use std::{
    cell::RefCell,
    path::PathBuf,
};

use interner::Interner;

#[derive(Clone, Copy, Debug, PartialEq, Eq, Default)]
pub struct SourceSpan {
    pub start: usize,
    pub end: usize,
}

impl SourceSpan {
    pub fn new(start: usize, end: usize) -> Self {
        Self { start, end }
    }
}

pub struct Session {
    pub source_text: String,
    pub interner: RefCell<Interner>,
}

impl Session {
    pub fn for_file(input: PathBuf) -> anyhow::Result<Self> {
        let source_text = std::fs::read_to_string(input)?;
        Ok(Self {
            source_text,
            interner: RefCell::new(Interner::with_pre_interned_symbols()),
        })
    }

    pub fn for_text(source_text: String) -> anyhow::Result<Self> {
        Ok(Self {
            source_text,
            interner: RefCell::new(Interner::with_pre_interned_symbols()),
        })
    }
}

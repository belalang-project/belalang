pub mod diag;
pub mod interner;

use std::{
    cell::RefCell,
    path::PathBuf,
};

use diag::{
    Diagnostic,
    Severity,
};
use interner::Interner;

use crate::interner::Symbol;

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
    interner: RefCell<Interner>,
    diagnostics: RefCell<Vec<Diagnostic>>,
}

impl Session {
    pub fn for_file(input: PathBuf) -> anyhow::Result<Self> {
        let source_text = std::fs::read_to_string(&input)?;
        Ok(Self {
            source_text,
            interner: RefCell::new(Interner::with_pre_interned_symbols()),
            diagnostics: RefCell::new(Vec::new()),
        })
    }

    pub fn for_text(source_text: String) -> anyhow::Result<Self> {
        Ok(Self {
            source_text,
            interner: RefCell::new(Interner::with_pre_interned_symbols()),
            diagnostics: RefCell::new(Vec::new()),
        })
    }

    pub fn intern_string(&self, s: &str) -> Symbol {
        self.interner.borrow_mut().intern(s)
    }

    pub fn lookup_string<'sess>(&'sess self, sym: Symbol) -> &'sess str {
        let interner = self.interner.borrow();
        let s = interner.lookup(sym);
        // SAFETY: the interned string does have the 'sess lifetime
        unsafe { std::mem::transmute::<&str, &'sess str>(s) }
    }

    pub fn emit(&self, diag: Diagnostic) {
        self.diagnostics.borrow_mut().push(diag);
    }

    pub fn has_errors(&self) -> bool {
        self.diagnostics.borrow().iter().any(|d| d.severity == Severity::Error)
    }

    pub fn take_diagnostics(&self) -> Vec<Diagnostic> {
        self.diagnostics.borrow_mut().drain(..).collect()
    }
}

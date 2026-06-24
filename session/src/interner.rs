use std::{
    collections::HashMap,
    fmt::Display,
    rc::Rc,
};

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct Symbol(pub u32);

impl Display for Symbol {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.0)
    }
}

#[derive(Default)]
pub struct Interner {
    strings: Vec<Rc<str>>,
    map: HashMap<Rc<str>, Symbol>,
}

pub mod syms {
    use crate::interner::Symbol;

    pub const INT: Symbol = Symbol(0);
    pub const PRINT: Symbol = Symbol(1);
}

impl Interner {
    pub fn with_pre_interned_symbols() -> Self {
        let mut s = Self::default();
        s.intern("Int");
        s.intern("print");
        s
    }

    pub fn intern(&mut self, value: &str) -> Symbol {
        if let Some(&sym) = self.map.get(value) {
            return sym;
        }

        let rc_value: Rc<str> = Rc::from(value);
        let id = self.map.len() as u32;
        let sym = Symbol(id);

        self.strings.push(rc_value.clone());
        self.map.insert(rc_value, sym);

        sym
    }

    pub fn lookup(&self, sym: Symbol) -> &str {
        &self.strings[sym.0 as usize]
    }
}

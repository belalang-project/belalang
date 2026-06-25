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

    // Types
    pub const INT: Symbol = Symbol(0);
    pub const FLOAT: Symbol = Symbol(1);
    pub const STRING: Symbol = Symbol(2);

    // Keywords
    pub const FN: Symbol = Symbol(3);
    pub const WHILE: Symbol = Symbol(4);
    pub const TRUE: Symbol = Symbol(5);
    pub const FALSE: Symbol = Symbol(6);
    pub const IF: Symbol = Symbol(7);
    pub const ELSE: Symbol = Symbol(8);
    pub const RETURN: Symbol = Symbol(9);

    // Built-ins
    pub const PRINT: Symbol = Symbol(10);
}

impl Interner {
    pub fn with_pre_interned_symbols() -> Self {
        let mut s = Self::default();

        // Types
        s.intern("Int");
        s.intern("Float");
        s.intern("String");

        // Keywords
        s.intern("fn");
        s.intern("while");
        s.intern("true");
        s.intern("false");
        s.intern("if");
        s.intern("else");
        s.intern("return");

        // Built-ins
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

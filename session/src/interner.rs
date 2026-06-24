use std::{
    collections::HashMap,
    rc::Rc,
};

#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub struct Symbol(u32);

#[derive(Default)]
pub struct Interner {
    strings: Vec<Rc<str>>,
    map: HashMap<Rc<str>, Symbol>,
}

impl Interner {
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

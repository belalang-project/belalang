use std::collections::{
    HashMap,
    hash_map::Entry,
};

use belvm_bytecode::opcode;
use belvm_std::BUILTIN_FUNCTIONS;

use crate::error::CodegenError;

#[derive(Debug, Clone, Copy)]
pub enum ScopeLevel {
    Builtin,
    Global,
    Local,
}

#[derive(Debug)]
pub struct Symbol {
    pub scope: ScopeLevel,
    pub index: usize,
}

pub struct CompilationScope {
    pub scope: ScopeLevel,
    pub instructions: Vec<u8>,
    pub symbol_store: HashMap<String, Symbol>,
    pub symbol_count: usize,
}

impl CompilationScope {
    pub fn define(&mut self, name: String) -> Result<&Symbol, CodegenError> {
        let symbol = Symbol {
            scope: self.scope,
            index: self.symbol_count,
        };

        self.symbol_count += 1;

        match self.symbol_store.entry(name.clone()) {
            Entry::Vacant(entry) => Ok(entry.insert(symbol)),
            Entry::Occupied(_) => Err(CodegenError::DuplicateSymbol(name)),
        }
    }

    pub fn resolve(&self, name: &String) -> Option<&Symbol> {
        self.symbol_store.get(name)
    }
}

pub struct ScopeManager {
    pub main_scope: CompilationScope,
    scope_store: Vec<CompilationScope>,
}

impl Default for ScopeManager {
    fn default() -> Self {
        let mut sm = Self {
            main_scope: CompilationScope {
                scope: ScopeLevel::Global,
                instructions: Vec::new(),
                symbol_store: HashMap::new(),
                symbol_count: 0,
            },
            scope_store: Vec::new(),
        };

        for key in BUILTIN_FUNCTIONS {
            sm.main_scope.symbol_store.insert(
                key.to_string(),
                Symbol {
                    scope: ScopeLevel::Builtin,
                    index: sm.main_scope.symbol_count,
                },
            );

            sm.main_scope.symbol_count += 1;
        }

        sm
    }
}

impl ScopeManager {
    pub fn enter(&mut self) {
        self.scope_store.push(CompilationScope {
            scope: ScopeLevel::Local,
            instructions: Vec::new(),
            symbol_store: HashMap::new(),
            symbol_count: 0,
        });
    }

    pub fn leave(&mut self) -> CompilationScope {
        // we want to panic when trying to leave main scope
        let mut scope = self.scope_store.pop().unwrap();

        if let Some(&opcode::POP) = scope.instructions.last() {
            scope.instructions.pop();
        }

        scope
    }

    pub fn current_mut(&mut self) -> &mut CompilationScope {
        self.scope_store.last_mut().unwrap_or(&mut self.main_scope)
    }

    pub fn current(&self) -> &CompilationScope {
        self.scope_store.last().unwrap_or(&self.main_scope)
    }

    pub fn define(&mut self, name: String) -> Result<&Symbol, CodegenError> {
        self.current_mut().define(name)
    }

    pub fn resolve(&self, name: String) -> Result<&Symbol, CodegenError> {
        for symbol in self.scope_store.iter().rev() {
            if let Some(symbol) = symbol.resolve(&name) {
                return Ok(symbol);
            }
        }

        self.main_scope.resolve(&name).ok_or(CodegenError::UnknownSymbol(name))
    }
}

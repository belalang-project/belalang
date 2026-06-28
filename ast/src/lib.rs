#![feature(if_let_guard)]

mod dump;
mod expressions;
mod parser;
mod program;
mod statements;
mod visitor;

pub use dump::*;
pub use expressions::*;
use lexer::{
    LexerError,
    TokenKind,
};
pub use parser::Parser;
pub use program::Program;
pub use statements::*;
pub use visitor::*;

pub enum Node<'ast> {
    Expression(Expression<'ast>),
    Statement(Statement<'ast>),
    Program(Program<'ast>),
}

#[derive(thiserror::Error, Debug)]
pub enum ParserError {
    #[error(transparent)]
    LexerError(#[from] LexerError),

    #[error("unexpected token: {0}")]
    UnexpectedToken(TokenKind),

    #[error("invalid lhs")]
    InvalidLHS,

    #[error("error parsing integer: could not parse {0} as integer")]
    ParsingInteger(String),

    #[error("error parsing float: could not parse {0} as float")]
    ParsingFloat(String),

    #[error("error parsing struct")]
    ParsingStruct,

    #[error("unknown prefix operator: {0}")]
    UnknownPrefixOperator(TokenKind),
}

pub struct Ast {
    bump: bumpalo::Bump,
}

impl Ast {
    pub fn new() -> Self {
        Self {
            bump: bumpalo::Bump::new(),
        }
    }

    pub fn alloc<T>(&self, val: T) -> &T {
        self.bump.alloc(val)
    }

    pub fn alloc_slice_clone<T: Clone>(&self, slice: &[T]) -> &[T] {
        self.bump.alloc_slice_clone(slice)
    }
}

impl Default for Ast {
    fn default() -> Self {
        Self::new()
    }
}

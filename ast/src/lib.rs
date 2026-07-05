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

#[derive(Debug)]
pub enum ParserError {
    LexerError(LexerError),
    UnexpectedToken(TokenKind),
    InvalidLHS,
    ParsingInteger(String),
    ParsingFloat(String),
    ParsingStruct,
    UnknownPrefixOperator(TokenKind),
}

impl From<LexerError> for ParserError {
    fn from(err: LexerError) -> Self {
        ParserError::LexerError(err)
    }
}

// NOTE: the parser error display isn't used by anything.
impl std::fmt::Display for ParserError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{self:?}")
    }
}

impl std::error::Error for ParserError {}

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

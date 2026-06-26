mod dump;
mod expressions;
mod parser;
mod program;
mod statements;
mod type_inferer;
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
pub use type_inferer::{
    Type,
    TypeInferer,
};
pub use visitor::*;

pub enum Node {
    Expression(Expression),
    Statement(Statement),
    Program(Program),
}

#[derive(thiserror::Error, Debug)]
pub enum ParserError {
    #[error(transparent)]
    LexerError(#[from] LexerError),

    #[error("unexpected token: {0}")]
    UnexpectedToken(TokenKind),

    #[error("invalid lhs: {0}")]
    InvalidLHS(Expression),

    #[error("error parsing integer: could not parse {0} as integer")]
    ParsingInteger(String),

    #[error("error parsing float: could not parse {0} as float")]
    ParsingFloat(String),

    #[error("unknown prefix operator: {0}")]
    UnknownPrefixOperator(TokenKind),
}

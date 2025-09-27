mod expressions;
mod parser;
mod program;
mod statements;

use belc_lexer::{LexerError, TokenKind};
pub use expressions::*;
pub use parser::Parser;
pub use program::Program;
pub use statements::*;

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

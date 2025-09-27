use belc_lexer::TokenKind;

#[derive(thiserror::Error, Debug)]
pub enum CodegenError {
    #[error("unknown infix operator: {0}")]
    UnknownInfixOp(TokenKind),

    #[error("duplicate symbol: {0}")]
    DuplicateSymbol(String),

    #[error("unknown symbol: {0}")]
    UnknownSymbol(String),
}

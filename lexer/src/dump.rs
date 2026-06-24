use session::Session;

use crate::{
    Lexer,
    TokenKind,
};

pub struct TokensDumper<'sess, 'lexer> {
    session: &'sess Session,
    lexer: &'lexer mut Lexer<'sess>,
}

impl<'sess, 'lexer> TokensDumper<'sess, 'lexer> {
    pub fn new(session: &'sess Session, lexer: &'lexer mut Lexer<'sess>) -> Self {
        Self { session, lexer }
    }

    pub fn dump(&mut self) -> Result<(), crate::LexerError> {
        loop {
            let token = self.lexer.next_token()?;
            if token.kind == TokenKind::EOF {
                break;
            }
            let kind_str = match token.kind {
                TokenKind::Ident { sym } => {
                    let val = self.session.interner.borrow().lookup(sym).to_string();
                    format!("Ident(\"{}\")", val)
                },
                TokenKind::Literal { kind, sym } => {
                    let val = self.session.interner.borrow().lookup(sym).to_string();
                    let val = if kind == crate::LiteralKind::String {
                        val.escape_debug().to_string()
                    } else {
                        val
                    };
                    format!("Literal({:?}, \"{}\")", kind, val)
                },
                TokenKind::Assign { kind } => {
                    format!("Assign({:?})", kind)
                },
                other => format!("{:?}", other),
            };
            println!("{} <{}..{}>", kind_str, token.span.start, token.span.end);
        }
        Ok(())
    }
}

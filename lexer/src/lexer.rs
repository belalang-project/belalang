use std::{
    iter::Peekable,
    str::Chars,
};

use session::{
    Session,
    SourceSpan,
};
use unicode_ident::{
    is_xid_continue,
    is_xid_start,
};

use super::Token;
use crate::{
    AssignmentKind,
    LiteralKind,
    TokenKind,
};

#[derive(thiserror::Error, Debug)]
pub enum LexerError {
    #[error("unknown token: {0}")]
    UnknownToken(String),

    #[error("unknown escape string")]
    UnknownEscapeString,

    #[error("unclosed string")]
    UnclosedString,
}

pub struct Lexer<'sess> {
    #[allow(dead_code)]
    session: &'sess Session,
    current: Option<char>,
    chars: Peekable<Chars<'sess>>,

    /// The current byte offset the lexer is at.
    current_offset: usize,
}

impl<'sess> Lexer<'sess> {
    pub fn new(session: &'sess Session) -> Lexer<'sess> {
        let mut chars = session.source_text.chars().peekable();
        let current = chars.next();

        Lexer {
            session,
            current,
            chars,
            current_offset: 0,
        }
    }

    fn advance(&mut self) -> Option<char> {
        let result = self.current;
        if let Some(c) = result {
            self.current_offset += c.len_utf8();
        }
        self.current = self.chars.next();
        result
    }

    pub fn next_token(&mut self) -> Result<Token, LexerError> {
        loop {
            match self.current {
                // skips all lines that start with `#`
                Some('#') => {
                    while let Some(c) = self.current {
                        if c == '\n' {
                            break;
                        }
                        self.advance();
                    }
                },
                // skips all empty whitespaces
                Some(' ' | '\t' | '\r') => {
                    self.advance();
                },
                // skips newlines
                Some('\n') => {
                    self.advance();
                },
                // break the loop if it isn't a whitespace or a comment
                _ => break,
            };
        }

        let start_offset = self.current_offset;

        if self.current.is_none() {
            return Ok(Token {
                span: SourceSpan::new(start_offset, start_offset),
                kind: TokenKind::EOF,
                value: String::new(),
            });
        }

        let mut token = match self.current {
            Some(':') => {
                self.advance();
                match self.current {
                    Some('=') => {
                        self.advance();
                        Ok(Token {
                            span: SourceSpan::default(),
                            kind: TokenKind::Assign {
                                kind: AssignmentKind::ColonAssign,
                            },
                            value: String::new(),
                        })
                    },
                    _ => Err(LexerError::UnknownToken(":".into())),
                }
            },
            Some('=') => {
                self.advance();
                match self.current {
                    Some('=') => {
                        self.advance();
                        Ok(Token {
                            span: SourceSpan::default(),
                            kind: TokenKind::Eq,
                            value: String::new(),
                        })
                    },
                    _ => Ok(Token {
                        span: SourceSpan::default(),
                        kind: TokenKind::Assign {
                            kind: AssignmentKind::Assign,
                        },
                        value: String::new(),
                    }),
                }
            },
            Some('!') => {
                self.advance();
                match self.current {
                    Some('=') => {
                        self.advance();
                        Ok(Token {
                            span: SourceSpan::default(),
                            kind: TokenKind::Ne,
                            value: String::new(),
                        })
                    },
                    _ => Ok(Token {
                        span: SourceSpan::default(),
                        kind: TokenKind::Not,
                        value: String::new(),
                    }),
                }
            },
            Some('&') => {
                self.advance();
                match self.current {
                    Some('&') => {
                        self.advance();
                        Ok(Token {
                            span: SourceSpan::default(),
                            kind: TokenKind::And,
                            value: String::new(),
                        })
                    },
                    Some('=') => {
                        self.advance();
                        Ok(Token {
                            span: SourceSpan::default(),
                            kind: TokenKind::Assign {
                                kind: AssignmentKind::BitAndAssign,
                            },
                            value: String::new(),
                        })
                    },
                    _ => Ok(Token {
                        span: SourceSpan::default(),
                        kind: TokenKind::BitAnd,
                        value: String::new(),
                    }),
                }
            },
            Some('|') => {
                self.advance();
                match self.current {
                    Some('|') => {
                        self.advance();
                        Ok(Token {
                            span: SourceSpan::default(),
                            kind: TokenKind::Or,
                            value: String::new(),
                        })
                    },
                    Some('=') => {
                        self.advance();
                        Ok(Token {
                            span: SourceSpan::default(),
                            kind: TokenKind::Assign {
                                kind: AssignmentKind::BitOrAssign,
                            },
                            value: String::new(),
                        })
                    },
                    _ => Ok(Token {
                        span: SourceSpan::default(),
                        kind: TokenKind::BitOr,
                        value: String::new(),
                    }),
                }
            },
            Some('^') => {
                self.advance();
                match self.current {
                    Some('=') => {
                        self.advance();
                        Ok(Token {
                            span: SourceSpan::default(),
                            kind: TokenKind::Assign {
                                kind: AssignmentKind::BitXorAssign,
                            },
                            value: String::new(),
                        })
                    },
                    _ => Ok(Token {
                        span: SourceSpan::default(),
                        kind: TokenKind::BitXor,
                        value: String::new(),
                    }),
                }
            },
            Some('<') => {
                self.advance();
                match self.current {
                    Some('=') => {
                        self.advance();
                        Ok(Token {
                            span: SourceSpan::default(),
                            kind: TokenKind::Le,
                            value: String::new(),
                        })
                    },
                    Some('<') => {
                        self.advance();
                        match self.chars.peek() {
                            Some('=') => {
                                self.advance();
                                Ok(Token {
                                    span: SourceSpan::default(),
                                    kind: TokenKind::Assign {
                                        kind: AssignmentKind::ShiftLeftAssign,
                                    },
                                    value: String::new(),
                                })
                            },
                            _ => Ok(Token {
                                span: SourceSpan::default(),
                                kind: TokenKind::ShiftLeft,
                                value: String::new(),
                            }),
                        }
                    },
                    _ => Ok(Token {
                        span: SourceSpan::default(),
                        kind: TokenKind::Lt,
                        value: String::new(),
                    }),
                }
            },
            Some('>') => {
                self.advance();
                match self.current {
                    Some('=') => {
                        self.advance();
                        Ok(Token {
                            span: SourceSpan::default(),
                            kind: TokenKind::Ge,
                            value: String::new(),
                        })
                    },
                    Some('>') => {
                        self.advance();
                        match self.chars.peek() {
                            Some('=') => {
                                self.advance();
                                Ok(Token {
                                    span: SourceSpan::default(),
                                    kind: TokenKind::Assign {
                                        kind: AssignmentKind::ShiftRightAssign,
                                    },
                                    value: String::new(),
                                })
                            },
                            _ => Ok(Token {
                                span: SourceSpan::default(),
                                kind: TokenKind::ShiftLeft,
                                value: String::new(),
                            }),
                        }
                    },
                    _ => Ok(Token {
                        span: SourceSpan::default(),
                        kind: TokenKind::Gt,
                        value: String::new(),
                    }),
                }
            },
            Some('+') => {
                self.advance();
                match self.current {
                    Some('=') => {
                        self.advance();
                        Ok(Token {
                            span: SourceSpan::default(),
                            kind: TokenKind::Assign {
                                kind: AssignmentKind::AddAssign,
                            },
                            value: String::new(),
                        })
                    },
                    _ => Ok(Token {
                        span: SourceSpan::default(),
                        kind: TokenKind::Add,
                        value: String::new(),
                    }),
                }
            },
            Some('-') => {
                self.advance();
                match self.current {
                    Some('=') => {
                        self.advance();
                        Ok(Token {
                            span: SourceSpan::default(),
                            kind: TokenKind::Assign {
                                kind: AssignmentKind::SubAssign,
                            },
                            value: String::new(),
                        })
                    },
                    _ => Ok(Token {
                        span: SourceSpan::default(),
                        kind: TokenKind::Sub,
                        value: String::new(),
                    }),
                }
            },
            Some('*') => {
                self.advance();
                match self.current {
                    Some('=') => {
                        self.advance();
                        Ok(Token {
                            span: SourceSpan::default(),
                            kind: TokenKind::Assign {
                                kind: AssignmentKind::MulAssign,
                            },
                            value: String::new(),
                        })
                    },
                    _ => Ok(Token {
                        span: SourceSpan::default(),
                        kind: TokenKind::Mul,
                        value: String::new(),
                    }),
                }
            },
            Some('/') => {
                self.advance();
                match self.current {
                    Some('=') => {
                        self.advance();
                        Ok(Token {
                            span: SourceSpan::default(),
                            kind: TokenKind::Assign {
                                kind: AssignmentKind::DivAssign,
                            },
                            value: String::new(),
                        })
                    },
                    _ => Ok(Token {
                        span: SourceSpan::default(),
                        kind: TokenKind::Div,
                        value: String::new(),
                    }),
                }
            },
            Some('%') => {
                self.advance();
                match self.current {
                    Some('=') => {
                        self.advance();
                        Ok(Token {
                            span: SourceSpan::default(),
                            kind: TokenKind::Assign {
                                kind: AssignmentKind::ModAssign,
                            },
                            value: String::new(),
                        })
                    },
                    _ => Ok(Token {
                        span: SourceSpan::default(),
                        kind: TokenKind::Mod,
                        value: String::new(),
                    }),
                }
            },
            Some('(') => {
                self.advance();
                Ok(Token {
                    span: SourceSpan::default(),
                    kind: TokenKind::LeftParen,
                    value: String::new(),
                })
            },
            Some(')') => {
                self.advance();
                Ok(Token {
                    span: SourceSpan::default(),
                    kind: TokenKind::RightParen,
                    value: String::new(),
                })
            },
            Some('{') => {
                self.advance();
                Ok(Token {
                    span: SourceSpan::default(),
                    kind: TokenKind::LeftBrace,
                    value: String::new(),
                })
            },
            Some('}') => {
                self.advance();
                Ok(Token {
                    span: SourceSpan::default(),
                    kind: TokenKind::RightBrace,
                    value: String::new(),
                })
            },
            Some('[') => {
                self.advance();
                Ok(Token {
                    span: SourceSpan::default(),
                    kind: TokenKind::LeftBracket,
                    value: String::new(),
                })
            },
            Some(']') => {
                self.advance();
                Ok(Token {
                    span: SourceSpan::default(),
                    kind: TokenKind::RightBracket,
                    value: String::new(),
                })
            },
            Some(';') => {
                self.advance();
                Ok(Token {
                    span: SourceSpan::default(),
                    kind: TokenKind::Semicolon,
                    value: String::new(),
                })
            },
            Some(',') => {
                self.advance();
                Ok(Token {
                    span: SourceSpan::default(),
                    kind: TokenKind::Comma,
                    value: String::new(),
                })
            },
            Some('\\') => {
                self.advance();
                Ok(Token {
                    span: SourceSpan::default(),
                    kind: TokenKind::Backslash,
                    value: String::new(),
                })
            },
            Some('"') => self.read_string(),
            Some(c) if c.is_numeric() => self.read_number(),
            Some(_) => self.read_identifier(),
            _ => unreachable!(),
        }?;

        token.span = SourceSpan::new(start_offset, self.current_offset);
        Ok(token)
    }

    fn read_string(&mut self) -> Result<Token, LexerError> {
        self.advance(); // consume the opening "
        let mut result = String::new();

        loop {
            match self.advance() {
                Some('\\') => match self.current {
                    Some('n') => {
                        self.advance();
                        result.push('\n');
                    },
                    Some('r') => {
                        self.advance();
                        result.push('\r');
                    },
                    Some('t') => {
                        self.advance();
                        result.push('\t');
                    },
                    Some('"') => {
                        self.advance();
                        result.push('"');
                    },
                    Some('\\') => {
                        self.advance();
                        result.push('\\');
                    },
                    Some('x') => {
                        self.advance(); // consume the 'x'

                        let hi = self.advance().and_then(|c| c.to_digit(16)).map(|d| d as u8);
                        let lo = self.advance().and_then(|c| c.to_digit(16)).map(|d| d as u8);

                        match (hi, lo) {
                            (Some(hi), Some(lo)) => result.push(((hi << 4) | lo) as char),
                            (_, _) => return Err(LexerError::UnknownEscapeString),
                        }
                    },
                    Some(_) => return Err(LexerError::UnknownEscapeString),
                    None => return Err(LexerError::UnclosedString),
                },
                Some('"') => break,
                Some(c) => result.push(c),
                None => return Err(LexerError::UnclosedString),
            }
        }

        Ok(Token {
            span: SourceSpan::default(),
            kind: TokenKind::Literal {
                kind: LiteralKind::String,
            },
            value: result,
        })
    }

    fn read_identifier(&mut self) -> Result<Token, LexerError> {
        match self.current {
            Some(c) if is_xid_start(c) => {
                let mut identifier = String::from(c);
                self.advance();

                while let Some(c) = self.current {
                    if is_xid_continue(c) {
                        identifier.push(c);
                        self.advance();
                    } else {
                        break;
                    }
                }

                let kind = match identifier.as_str() {
                    "fn" => TokenKind::Function,
                    "while" => TokenKind::While,
                    "true" | "false" => TokenKind::Literal {
                        kind: LiteralKind::Boolean,
                    },
                    "if" => TokenKind::If,
                    "else" => TokenKind::Else,
                    "return" => TokenKind::Return,
                    _ => TokenKind::Ident,
                };
                Ok(Token {
                    kind,
                    value: identifier,
                    span: SourceSpan::default(),
                })
            },
            Some(c) => Err(LexerError::UnknownToken(c.to_string())),
            _ => Ok(Token {
                span: SourceSpan::default(),
                kind: TokenKind::EOF,
                value: String::new(),
            }),
        }
    }

    fn read_number(&mut self) -> Result<Token, LexerError> {
        let mut has_decimal = false;
        let mut number = String::new();

        while let Some(c) = self.current {
            if c.is_ascii_digit() {
                number.push(c);
                self.advance();
            } else if c == '.' && !has_decimal {
                has_decimal = true;
                number.push(c);
                self.advance();
            } else {
                break;
            }
        }

        let kind = if has_decimal {
            LiteralKind::Float
        } else {
            LiteralKind::Integer
        };
        Ok(Token {
            span: SourceSpan::default(),
            kind: TokenKind::Literal { kind },
            value: number,
        })
    }
}

#[cfg(test)]
mod tests {
    use session::{
        Session,
        SourceSpan,
    };

    use super::{
        Lexer,
        Token,
    };
    use crate::{
        LiteralKind,
        TokenKind,
    };

    #[test]
    fn str_ascii() {
        let session = Session::for_text("\"Hello\"".to_string()).unwrap();
        let mut lexer = Lexer::new(&session);

        let result = lexer.read_string();

        let expect = Token {
            span: SourceSpan::default(),
            kind: TokenKind::Literal {
                kind: LiteralKind::String,
            },
            value: "Hello".into(),
        };
        assert_eq!(result.unwrap(), expect);
        assert_eq!(lexer.current_offset, 7);
    }

    #[test]
    fn str_japanese_chars() {
        let session = Session::for_text("\"こんにちわ\"".to_string()).unwrap();
        let mut lexer = Lexer::new(&session);
        let result = lexer.read_string();

        let expect = Token {
            span: SourceSpan::default(),
            kind: TokenKind::Literal {
                kind: LiteralKind::String,
            },
            value: "こんにちわ".into(),
        };
        assert_eq!(result.unwrap(), expect);
        assert_eq!(lexer.current_offset, 17);
    }

    #[test]
    fn str_emojis() {
        let session = Session::for_text("\"🦗\"".to_string()).unwrap();
        let mut lexer = Lexer::new(&session);
        let result = lexer.read_string();

        let expect = Token {
            span: SourceSpan::default(),
            kind: TokenKind::Literal {
                kind: LiteralKind::String,
            },
            value: "🦗".into(),
        };
        assert_eq!(result.unwrap(), expect);
        assert_eq!(lexer.current_offset, 6);
    }

    #[test]
    fn ident_ascii() {
        let session = Session::for_text("Hello".to_string()).unwrap();
        let mut lexer = Lexer::new(&session);
        let result = lexer.read_identifier();

        assert_eq!(
            result.unwrap(),
            Token {
                span: SourceSpan::default(),
                kind: TokenKind::Ident,
                value: "Hello".into()
            }
        );
        assert_eq!(lexer.current_offset, 5);
    }

    #[test]
    fn ident_japanese_chars() {
        let session = Session::for_text("こんにちわ".to_string()).unwrap();
        let mut lexer = Lexer::new(&session);
        let result = lexer.read_identifier();

        assert_eq!(
            result.unwrap(),
            Token {
                span: SourceSpan::default(),
                kind: TokenKind::Ident,
                value: "こんにちわ".into()
            }
        );
        assert_eq!(lexer.current_offset, 15);
    }

    #[test]
    fn ident_underscores() {
        let session = Session::for_text("hel_lo_".to_string()).unwrap();
        let mut lexer = Lexer::new(&session);
        let result = lexer.read_identifier();

        assert_eq!(
            result.unwrap(),
            Token {
                span: SourceSpan::default(),
                kind: TokenKind::Ident,
                value: "hel_lo_".into(),
            }
        );
        assert_eq!(lexer.current_offset, 7);
    }

    #[test]
    fn number_int_ascii() {
        let session = Session::for_text("123".to_string()).unwrap();
        let mut lexer = Lexer::new(&session);
        let result = lexer.read_number();

        let expect = Token {
            span: SourceSpan::default(),
            kind: TokenKind::Literal {
                kind: LiteralKind::Integer,
            },
            value: "123".into(),
        };
        assert_eq!(result.unwrap(), expect);
        assert_eq!(lexer.current_offset, 3);
    }

    #[test]
    fn number_float_ascii() {
        let session = Session::for_text("123.123".to_string()).unwrap();
        let mut lexer = Lexer::new(&session);
        let result = lexer.read_number();

        let expect = Token {
            span: SourceSpan::default(),
            kind: TokenKind::Literal {
                kind: LiteralKind::Float,
            },
            value: "123.123".into(),
        };
        assert_eq!(result.unwrap(), expect);
        assert_eq!(lexer.current_offset, 7);
    }

    #[test]
    fn multiline() {
        let session = Session::for_text("123.123\n\n".to_string()).unwrap();
        let mut lexer = Lexer::new(&session);
        lexer.next_token().unwrap();
        lexer.next_token().unwrap();

        assert_eq!(lexer.current_offset, 9);
    }
}

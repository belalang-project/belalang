use std::{
    cell::RefCell,
    iter::Peekable,
    str::Chars,
};

use diag::{
    Diagnostic,
    Label,
    Severity,
};
use session::{
    Session,
    interner::syms,
};
use span::SourceSpan;
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

#[derive(Debug)]
pub enum LexerError {
    UnknownToken(String),
    UnknownEscapeString,
    UnclosedString,
}

// NOTE: the lexer error display isn't used by anything.
impl std::fmt::Display for LexerError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{self:?}")
    }
}

impl std::error::Error for LexerError {}

pub struct Lexer<'sess> {
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
            });
        }

        let mut token = match self.current {
            Some(':') => {
                self.advance();
                Ok(Token {
                    span: SourceSpan::default(),
                    kind: TokenKind::Colon,
                })
            },
            Some('=') => {
                self.advance();
                match self.current {
                    Some('=') => {
                        self.advance();
                        Ok(Token {
                            span: SourceSpan::default(),
                            kind: TokenKind::Eq,
                        })
                    },
                    _ => Ok(Token {
                        span: SourceSpan::default(),
                        kind: TokenKind::Assign {
                            kind: AssignmentKind::Assign,
                        },
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
                        })
                    },
                    _ => Ok(Token {
                        span: SourceSpan::default(),
                        kind: TokenKind::Not,
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
                        })
                    },
                    Some('=') => {
                        self.advance();
                        Ok(Token {
                            span: SourceSpan::default(),
                            kind: TokenKind::Assign {
                                kind: AssignmentKind::BitAndAssign,
                            },
                        })
                    },
                    _ => Ok(Token {
                        span: SourceSpan::default(),
                        kind: TokenKind::BitAnd,
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
                        })
                    },
                    Some('=') => {
                        self.advance();
                        Ok(Token {
                            span: SourceSpan::default(),
                            kind: TokenKind::Assign {
                                kind: AssignmentKind::BitOrAssign,
                            },
                        })
                    },
                    _ => Ok(Token {
                        span: SourceSpan::default(),
                        kind: TokenKind::BitOr,
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
                        })
                    },
                    _ => Ok(Token {
                        span: SourceSpan::default(),
                        kind: TokenKind::BitXor,
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
                                })
                            },
                            _ => Ok(Token {
                                span: SourceSpan::default(),
                                kind: TokenKind::ShiftLeft,
                            }),
                        }
                    },
                    _ => Ok(Token {
                        span: SourceSpan::default(),
                        kind: TokenKind::Lt,
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
                                })
                            },
                            _ => Ok(Token {
                                span: SourceSpan::default(),
                                kind: TokenKind::ShiftLeft,
                            }),
                        }
                    },
                    _ => Ok(Token {
                        span: SourceSpan::default(),
                        kind: TokenKind::Gt,
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
                        })
                    },
                    _ => Ok(Token {
                        span: SourceSpan::default(),
                        kind: TokenKind::Add,
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
                        })
                    },
                    _ => Ok(Token {
                        span: SourceSpan::default(),
                        kind: TokenKind::Sub,
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
                        })
                    },
                    _ => Ok(Token {
                        span: SourceSpan::default(),
                        kind: TokenKind::Mul,
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
                        })
                    },
                    _ => Ok(Token {
                        span: SourceSpan::default(),
                        kind: TokenKind::Div,
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
                        })
                    },
                    _ => Ok(Token {
                        span: SourceSpan::default(),
                        kind: TokenKind::Mod,
                    }),
                }
            },
            Some('(') => {
                self.advance();
                Ok(Token {
                    span: SourceSpan::default(),
                    kind: TokenKind::LeftParen,
                })
            },
            Some(')') => {
                self.advance();
                Ok(Token {
                    span: SourceSpan::default(),
                    kind: TokenKind::RightParen,
                })
            },
            Some('{') => {
                self.advance();
                Ok(Token {
                    span: SourceSpan::default(),
                    kind: TokenKind::LeftBrace,
                })
            },
            Some('}') => {
                self.advance();
                Ok(Token {
                    span: SourceSpan::default(),
                    kind: TokenKind::RightBrace,
                })
            },
            Some('[') => {
                self.advance();
                Ok(Token {
                    span: SourceSpan::default(),
                    kind: TokenKind::LeftBracket,
                })
            },
            Some(']') => {
                self.advance();
                Ok(Token {
                    span: SourceSpan::default(),
                    kind: TokenKind::RightBracket,
                })
            },
            Some(';') => {
                self.advance();
                Ok(Token {
                    span: SourceSpan::default(),
                    kind: TokenKind::Semicolon,
                })
            },
            Some(',') => {
                self.advance();
                Ok(Token {
                    span: SourceSpan::default(),
                    kind: TokenKind::Comma,
                })
            },
            Some('.') => {
                self.advance();
                Ok(Token {
                    span: SourceSpan::default(),
                    kind: TokenKind::Dot,
                })
            },
            Some('\\') => {
                self.advance();
                Ok(Token {
                    span: SourceSpan::default(),
                    kind: TokenKind::Backslash,
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
        let string_start = self.current_offset;
        self.advance(); // consume the opening "
        let mut result = String::new();

        loop {
            match self.advance() {
                Some('\\') => {
                    let escape_start = self.current_offset - 1;
                    match self.current {
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
                                (_, _) => {
                                    let span = SourceSpan::new(escape_start, self.current_offset);
                                    self.session.emit(
                                        Diagnostic::error("Unknown escape string")
                                            .with_label(Label::primary(span, "Unknown escape string")),
                                    );
                                    return Err(LexerError::UnknownEscapeString);
                                },
                            }
                        },
                        Some(_) => {
                            self.advance();
                            let span = SourceSpan::new(escape_start, self.current_offset);
                            self.session.emit(
                                Diagnostic::error("Unknown escape string")
                                    .with_label(Label::primary(span, "Unknown escape string")),
                            );
                            return Err(LexerError::UnknownEscapeString);
                        },
                        None => {
                            let span = SourceSpan::new(string_start, self.current_offset);
                            self.session.emit(
                                Diagnostic::error("Unclosed string")
                                    .with_label(Label::primary(span, "Unclosed string")),
                            );
                            return Err(LexerError::UnclosedString);
                        },
                    }
                },
                Some('"') => break,
                Some(c) => result.push(c),
                None => {
                    let span = SourceSpan::new(string_start, self.current_offset);
                    self.session
                        .emit(Diagnostic::error("Unclosed string").with_label(Label::primary(span, "Unclosed string")));
                    return Err(LexerError::UnclosedString);
                },
            }
        }

        let sym = self.session.intern_string(&result);

        Ok(Token {
            span: SourceSpan::default(),
            kind: TokenKind::Literal {
                kind: LiteralKind::String,
                sym,
            },
        })
    }

    fn read_identifier(&mut self) -> Result<Token, LexerError> {
        let start_offset = self.current_offset;
        match self.current {
            Some(c) if is_xid_start(c) => {
                self.advance();

                while let Some(c) = self.current {
                    if is_xid_continue(c) {
                        self.advance();
                    } else {
                        break;
                    }
                }

                let identifier = &self.session.source_text[start_offset..self.current_offset];
                let sym = self.session.intern_string(&identifier);
                let kind = match sym {
                    syms::FN => TokenKind::Function,
                    syms::WHILE => TokenKind::While,
                    syms::BREAK => TokenKind::KwBreak,
                    syms::CONTINUE => TokenKind::KwContinue,
                    syms::TRUE => TokenKind::KwTrue,
                    syms::FALSE => TokenKind::KwFalse,
                    syms::IF => TokenKind::If,
                    syms::ELSE => TokenKind::Else,
                    syms::RETURN => TokenKind::Return,
                    syms::STRUCT => TokenKind::Struct,
                    _ => TokenKind::Ident { sym },
                };

                Ok(Token {
                    kind,
                    span: SourceSpan::default(),
                })
            },
            Some(c) => {
                let char_len = c.len_utf8();
                let span = SourceSpan::new(self.current_offset, self.current_offset + char_len);
                self.session
                    .emit(Diagnostic::error("Unknown token").with_label(Label::primary(span, "Unknown token")));
                Err(LexerError::UnknownToken(c.to_string()))
            },
            _ => Ok(Token {
                span: SourceSpan::default(),
                kind: TokenKind::EOF,
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

        let sym = self.session.intern_string(&number);

        Ok(Token {
            span: SourceSpan::default(),
            kind: TokenKind::Literal { kind, sym },
        })
    }
}

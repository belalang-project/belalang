use std::iter::Peekable;
use std::str::Chars;

use unicode_ident::{is_xid_continue, is_xid_start};

use super::Token;
use crate::{AssignmentKind, LiteralKind};

#[derive(thiserror::Error, Debug)]
pub enum LexerError {
    #[error("unknown token: {0}")]
    UnknownToken(String),

    #[error("unknown escape string")]
    UnknownEscapeString,

    #[error("unclosed string")]
    UnclosedString,
}

pub fn char_to_u8(c: char) -> Option<u8> {
    match c {
        '0'..='9' => Some(c as u8 - b'0'),
        'a'..='f' => Some(c as u8 - b'a' + 10),
        'A'..='F' => Some(c as u8 - b'A' + 10),
        _ => None,
    }
}

pub struct Lexer<'a> {
    current: Option<char>,
    chars: Peekable<Chars<'a>>,
    #[allow(dead_code)]
    source: &'a String,

    /// The current line number the lexer is at.
    ///
    /// Points to the next line to process.
    current_row: u32,

    /// The current column number the lexer is at.
    ///
    /// Points to the next character to process.
    current_col: u32,
}

impl<'a> Lexer<'a> {
    pub fn new(source: &'a String) -> Lexer<'a> {
        let mut chars = source.chars().peekable();
        let current = chars.next();

        Lexer {
            current,
            chars,
            source,
            current_row: 1,
            current_col: 1,
        }
    }

    fn advance(&mut self) -> Option<char> {
        let result = self.current;
        self.current = self.chars.next();
        self.current_col += 1;
        result
    }

    pub fn next_token(&mut self) -> Result<Token, LexerError> {
        loop {
            match self.current {
                // skips all lines that start with `#`
                Some('#') => {
                    while let Some(c) = self.advance() {
                        if c == '\n' {
                            self.advance();
                            self.current_row += 1;
                            self.current_col = 1;
                            break;
                        }
                    }
                },
                // skips all empty whitespaces
                Some(' ' | '\t' | '\r') => {
                    self.advance();
                },
                // skips newlines
                Some('\n') => {
                    self.advance();
                    self.current_row += 1;
                    self.current_col = 1;
                },
                // break the loop if it isn't a whitespace or a comment
                _ => break,
            };
        }

        if self.current.is_none() {
            return Ok(Token::EOF);
        }

        match self.current {
            Some(':') => {
                self.advance();
                match self.current {
                    Some('=') => {
                        self.advance();
                        Ok(Token::Assign {
                            kind: AssignmentKind::ColonAssign,
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
                        Ok(Token::Eq)
                    },
                    _ => Ok(Token::Assign {
                        kind: AssignmentKind::Assign,
                    }),
                }
            },
            Some('!') => {
                self.advance();
                match self.current {
                    Some('=') => {
                        self.advance();
                        Ok(Token::Ne)
                    },
                    _ => Ok(Token::Not),
                }
            },
            Some('&') => {
                self.advance();
                match self.current {
                    Some('&') => {
                        self.advance();
                        Ok(Token::And)
                    },
                    Some('=') => {
                        self.advance();
                        Ok(Token::Assign {
                            kind: AssignmentKind::BitAndAssign,
                        })
                    },
                    _ => Ok(Token::BitAnd),
                }
            },
            Some('|') => {
                self.advance();
                match self.current {
                    Some('|') => {
                        self.advance();
                        Ok(Token::Or)
                    },
                    Some('=') => {
                        self.advance();
                        Ok(Token::Assign {
                            kind: AssignmentKind::BitOrAssign,
                        })
                    },
                    _ => Ok(Token::BitOr),
                }
            },
            Some('^') => {
                self.advance();
                match self.current {
                    Some('=') => {
                        self.advance();
                        Ok(Token::Assign {
                            kind: AssignmentKind::BitXorAssign,
                        })
                    },
                    _ => Ok(Token::BitXor),
                }
            },
            Some('<') => {
                self.advance();
                match self.current {
                    Some('=') => {
                        self.advance();
                        Ok(Token::Le)
                    },
                    Some('<') => {
                        self.advance();
                        match self.chars.peek() {
                            Some('=') => {
                                self.advance();
                                Ok(Token::Assign {
                                    kind: AssignmentKind::ShiftLeftAssign,
                                })
                            },
                            _ => Ok(Token::ShiftLeft),
                        }
                    },
                    _ => Ok(Token::Lt),
                }
            },
            Some('>') => {
                self.advance();
                match self.current {
                    Some('=') => {
                        self.advance();
                        Ok(Token::Ge)
                    },
                    Some('>') => {
                        self.advance();
                        match self.chars.peek() {
                            Some('=') => {
                                self.advance();
                                Ok(Token::Assign {
                                    kind: AssignmentKind::ShiftRightAssign,
                                })
                            },
                            _ => Ok(Token::ShiftRight),
                        }
                    },
                    _ => Ok(Token::Gt),
                }
            },
            Some('+') => {
                self.advance();
                match self.current {
                    Some('=') => {
                        self.advance();
                        Ok(Token::Assign {
                            kind: AssignmentKind::AddAssign,
                        })
                    },
                    _ => Ok(Token::Add),
                }
            },
            Some('-') => {
                self.advance();
                match self.current {
                    Some('=') => {
                        self.advance();
                        Ok(Token::Assign {
                            kind: AssignmentKind::SubAssign,
                        })
                    },
                    _ => Ok(Token::Sub),
                }
            },
            Some('*') => {
                self.advance();
                match self.current {
                    Some('=') => {
                        self.advance();
                        Ok(Token::Assign {
                            kind: AssignmentKind::MulAssign,
                        })
                    },
                    _ => Ok(Token::Mul),
                }
            },
            Some('/') => {
                self.advance();
                match self.current {
                    Some('=') => {
                        self.advance();
                        Ok(Token::Assign {
                            kind: AssignmentKind::DivAssign,
                        })
                    },
                    _ => Ok(Token::Div),
                }
            },
            Some('%') => {
                self.advance();
                match self.current {
                    Some('=') => {
                        self.advance();
                        Ok(Token::Assign {
                            kind: AssignmentKind::ModAssign,
                        })
                    },
                    _ => Ok(Token::Mod),
                }
            },
            Some('(') => {
                self.advance();
                Ok(Token::LeftParen)
            },
            Some(')') => {
                self.advance();
                Ok(Token::RightParen)
            },
            Some('{') => {
                self.advance();
                Ok(Token::LeftBrace)
            },
            Some('}') => {
                self.advance();
                Ok(Token::RightBrace)
            },
            Some('[') => {
                self.advance();
                Ok(Token::LeftBracket)
            },
            Some(']') => {
                self.advance();
                Ok(Token::RightBracket)
            },
            Some(';') => {
                self.advance();
                Ok(Token::Semicolon)
            },
            Some(',') => {
                self.advance();
                Ok(Token::Comma)
            },
            Some('\\') => {
                self.advance();
                Ok(Token::Backslash)
            },
            Some('"') => self.read_string(),
            Some(c) if c.is_numeric() => Ok(self.read_number()?),
            Some(_) => Ok(self.read_identifier()?),
            _ => unreachable!(),
        }
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

                        match (self.advance().and_then(char_to_u8), self.advance().and_then(char_to_u8)) {
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

        Ok(Token::Literal {
            kind: LiteralKind::String,
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

                Ok(Token::from(identifier.as_str()))
            },
            Some(c) => Err(LexerError::UnknownToken(c.to_string())),
            _ => Ok(Token::EOF),
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
        Ok(Token::Literal { kind, value: number })
    }
}

#[cfg(test)]
mod tests {
    use super::Lexer;
    use super::Token;
    use crate::LiteralKind;

    #[test]
    fn str_ascii() {
        let source = String::from("\"Hello\"");
        let mut lexer = Lexer::new(&source);
        let result = lexer.read_string();

        let expect = Token::Literal {
            kind: LiteralKind::String,
            value: "Hello".into(),
        };
        assert_eq!(result.unwrap(), expect);
        assert_eq!(lexer.current_row, 1);
        assert_eq!(lexer.current_col, 8);
    }

    #[test]
    fn str_japanese_chars() {
        let source = String::from("\"こんにちわ\"");
        let mut lexer = Lexer::new(&source);
        let result = lexer.read_string();

        let expect = Token::Literal {
            kind: LiteralKind::String,
            value: "こんにちわ".into(),
        };
        assert_eq!(result.unwrap(), expect);
        assert_eq!(lexer.current_row, 1);
        assert_eq!(lexer.current_col, 8);
    }

    #[test]
    fn str_emojis() {
        let source = String::from("\"🦗\"");
        let mut lexer = Lexer::new(&source);
        let result = lexer.read_string();

        let expect = Token::Literal {
            kind: LiteralKind::String,
            value: "🦗".into(),
        };
        assert_eq!(result.unwrap(), expect);
        assert_eq!(lexer.current_row, 1);
        assert_eq!(lexer.current_col, 4);
    }

    #[test]
    fn ident_ascii() {
        let source = String::from("Hello");
        let mut lexer = Lexer::new(&source);
        let result = lexer.read_identifier();

        assert_eq!(result.unwrap(), Token::Ident { value: "Hello".into() });
        assert_eq!(lexer.current_row, 1);
        assert_eq!(lexer.current_col, 6);
    }

    #[test]
    fn ident_japanese_chars() {
        let source = String::from("こんにちわ");
        let mut lexer = Lexer::new(&source);
        let result = lexer.read_identifier();

        assert_eq!(
            result.unwrap(),
            Token::Ident {
                value: "こんにちわ".into()
            }
        );
        assert_eq!(lexer.current_row, 1);
        assert_eq!(lexer.current_col, 6);
    }

    #[test]
    fn ident_underscores() {
        let source = String::from("hel_lo_");
        let mut lexer = Lexer::new(&source);
        let result = lexer.read_identifier();

        assert_eq!(
            result.unwrap(),
            Token::Ident {
                value: "hel_lo_".into()
            }
        );
        assert_eq!(lexer.current_row, 1);
        assert_eq!(lexer.current_col, 8);
    }

    #[test]
    fn number_int_ascii() {
        let source = String::from("123");
        let mut lexer = Lexer::new(&source);
        let result = lexer.read_number();

        let expect = Token::Literal {
            kind: LiteralKind::Integer,
            value: "123".into(),
        };
        assert_eq!(result.unwrap(), expect);
        assert_eq!(lexer.current_row, 1);
        assert_eq!(lexer.current_col, 4);
    }

    #[test]
    fn number_float_ascii() {
        let source = String::from("123.123");
        let mut lexer = Lexer::new(&source);
        let result = lexer.read_number();

        let expect = Token::Literal {
            kind: LiteralKind::Float,
            value: "123.123".into(),
        };
        assert_eq!(result.unwrap(), expect);
        assert_eq!(lexer.current_row, 1);
        assert_eq!(lexer.current_col, 8);
    }

    #[test]
    fn multiline() {
        let source = String::from("123.123\n\n");
        let mut lexer = Lexer::new(&source);
        lexer.next_token().unwrap();
        lexer.next_token().unwrap();

        assert_eq!(lexer.current_row, 3);
        assert_eq!(lexer.current_col, 1);
    }
}

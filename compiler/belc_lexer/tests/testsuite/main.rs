use belc_lexer::{AssignmentKind, Lexer, LiteralKind, Token, TokenKind};

#[track_caller]
fn test_tokens(input: &str, expected: Vec<Token>) {
    let source = input.to_owned();
    let mut lexer = Lexer::new(&source);
    let mut result = Vec::new();
    while let Ok(token) = lexer.next_token() {
        if let TokenKind::EOF = token.kind {
            break;
        }
        result.push(token);
    }
    assert_eq!(result, expected);
}

fn empty_token(kind: TokenKind) -> Token {
    Token {
        kind,
        value: String::new(),
    }
}

#[test]
fn tokens_all() {
    test_tokens(
        "=+(){}[],;!-/*5;5 < 10 > 5;:= >= <= += -= /= %= *= || &&",
        vec![
            empty_token(TokenKind::Assign {
                kind: AssignmentKind::Assign,
            }),
            empty_token(TokenKind::Add),
            empty_token(TokenKind::LeftParen),
            empty_token(TokenKind::RightParen),
            empty_token(TokenKind::LeftBrace),
            empty_token(TokenKind::RightBrace),
            empty_token(TokenKind::LeftBracket),
            empty_token(TokenKind::RightBracket),
            empty_token(TokenKind::Comma),
            empty_token(TokenKind::Semicolon),
            empty_token(TokenKind::Not),
            empty_token(TokenKind::Sub),
            empty_token(TokenKind::Div),
            empty_token(TokenKind::Mul),
            Token {
                kind: TokenKind::Literal {
                    kind: LiteralKind::Integer,
                },
                value: "5".into(),
            },
            empty_token(TokenKind::Semicolon),
            Token {
                kind: TokenKind::Literal {
                    kind: LiteralKind::Integer,
                },
                value: "5".into(),
            },
            empty_token(TokenKind::Lt),
            Token {
                kind: TokenKind::Literal {
                    kind: LiteralKind::Integer,
                },
                value: "10".into(),
            },
            empty_token(TokenKind::Gt),
            Token {
                kind: TokenKind::Literal {
                    kind: LiteralKind::Integer,
                },
                value: "5".into(),
            },
            empty_token(TokenKind::Semicolon),
            empty_token(TokenKind::Assign {
                kind: AssignmentKind::ColonAssign,
            }),
            empty_token(TokenKind::Ge),
            empty_token(TokenKind::Le),
            empty_token(TokenKind::Assign {
                kind: AssignmentKind::AddAssign,
            }),
            empty_token(TokenKind::Assign {
                kind: AssignmentKind::SubAssign,
            }),
            empty_token(TokenKind::Assign {
                kind: AssignmentKind::DivAssign,
            }),
            empty_token(TokenKind::Assign {
                kind: AssignmentKind::ModAssign,
            }),
            empty_token(TokenKind::Assign {
                kind: AssignmentKind::MulAssign,
            }),
            empty_token(TokenKind::Or),
            empty_token(TokenKind::And),
        ],
    );
}

#[test]
fn tokens_strings() {
    test_tokens(
        r#""Hello, World"; "Test""#,
        vec![
            Token {
                kind: TokenKind::Literal {
                    kind: LiteralKind::String,
                },
                value: "Hello, World".into(),
            },
            empty_token(TokenKind::Semicolon),
            Token {
                kind: TokenKind::Literal {
                    kind: LiteralKind::String,
                },
                value: "Test".into(),
            },
        ],
    );
}

#[test]
fn tokens_integers() {
    test_tokens(
        "123; 456; 789 + 1",
        vec![
            Token {
                kind: TokenKind::Literal {
                    kind: LiteralKind::Integer,
                },
                value: "123".into(),
            },
            empty_token(TokenKind::Semicolon),
            Token {
                kind: TokenKind::Literal {
                    kind: LiteralKind::Integer,
                },
                value: "456".into(),
            },
            empty_token(TokenKind::Semicolon),
            Token {
                kind: TokenKind::Literal {
                    kind: LiteralKind::Integer,
                },
                value: "789".into(),
            },
            empty_token(TokenKind::Add),
            Token {
                kind: TokenKind::Literal {
                    kind: LiteralKind::Integer,
                },
                value: "1".into(),
            },
        ],
    );
}

#[test]
fn tokens_identifiers() {
    test_tokens(
        "x; x + y",
        vec![
            Token {
                kind: TokenKind::Ident,
                value: "x".into(),
            },
            empty_token(TokenKind::Semicolon),
            Token {
                kind: TokenKind::Ident,
                value: "x".into(),
            },
            empty_token(TokenKind::Add),
            Token {
                kind: TokenKind::Ident,
                value: "y".into(),
            },
        ],
    );
}

#[test]
fn tokens_escape_strings() {
    test_tokens(
        r#""\n\r\t\"\x41""#,
        vec![Token {
            kind: TokenKind::Literal {
                kind: LiteralKind::String,
            },
            value: "\n\r\t\"A".into(),
        }],
    );
}

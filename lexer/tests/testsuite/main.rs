use lexer::{
    AssignmentKind,
    Lexer,
    LiteralKind,
    Token,
    TokenKind,
};
use session::{
    Session,
    SourceSpan,
};

#[track_caller]
fn test_tokens(input: &str, expected: Vec<Token>) {
    let session = Session::for_text(input.to_owned()).unwrap();
    let mut lexer = Lexer::new(&session);
    let mut result = Vec::new();
    while let Ok(token) = lexer.next_token() {
        if let TokenKind::EOF = token.kind {
            break;
        }
        result.push(token);
    }
    assert_eq!(result, expected);
}

fn token(kind: TokenKind, value: &str, start: usize, end: usize) -> Token {
    Token {
        kind,
        value: value.to_string(),
        span: SourceSpan::new(start, end),
    }
}

fn empty_token(kind: TokenKind, start: usize, end: usize) -> Token {
    Token {
        kind,
        value: String::new(),
        span: SourceSpan::new(start, end),
    }
}

#[test]
fn tokens_all() {
    test_tokens(
        "=+(){}[],;!-/*5;5 < 10 > 5;:= >= <= += -= /= %= *= || &&",
        vec![
            empty_token(
                TokenKind::Assign {
                    kind: AssignmentKind::Assign,
                },
                0,
                1,
            ),
            empty_token(TokenKind::Add, 1, 2),
            empty_token(TokenKind::LeftParen, 2, 3),
            empty_token(TokenKind::RightParen, 3, 4),
            empty_token(TokenKind::LeftBrace, 4, 5),
            empty_token(TokenKind::RightBrace, 5, 6),
            empty_token(TokenKind::LeftBracket, 6, 7),
            empty_token(TokenKind::RightBracket, 7, 8),
            empty_token(TokenKind::Comma, 8, 9),
            empty_token(TokenKind::Semicolon, 9, 10),
            empty_token(TokenKind::Not, 10, 11),
            empty_token(TokenKind::Sub, 11, 12),
            empty_token(TokenKind::Div, 12, 13),
            empty_token(TokenKind::Mul, 13, 14),
            token(
                TokenKind::Literal {
                    kind: LiteralKind::Integer,
                },
                "5",
                14,
                15,
            ),
            empty_token(TokenKind::Semicolon, 15, 16),
            token(
                TokenKind::Literal {
                    kind: LiteralKind::Integer,
                },
                "5",
                16,
                17,
            ),
            empty_token(TokenKind::Lt, 18, 19),
            token(
                TokenKind::Literal {
                    kind: LiteralKind::Integer,
                },
                "10",
                20,
                22,
            ),
            empty_token(TokenKind::Gt, 23, 24),
            token(
                TokenKind::Literal {
                    kind: LiteralKind::Integer,
                },
                "5",
                25,
                26,
            ),
            empty_token(TokenKind::Semicolon, 26, 27),
            empty_token(
                TokenKind::Assign {
                    kind: AssignmentKind::ColonAssign,
                },
                27,
                29,
            ),
            empty_token(TokenKind::Ge, 30, 32),
            empty_token(TokenKind::Le, 33, 35),
            empty_token(
                TokenKind::Assign {
                    kind: AssignmentKind::AddAssign,
                },
                36,
                38,
            ),
            empty_token(
                TokenKind::Assign {
                    kind: AssignmentKind::SubAssign,
                },
                39,
                41,
            ),
            empty_token(
                TokenKind::Assign {
                    kind: AssignmentKind::DivAssign,
                },
                42,
                44,
            ),
            empty_token(
                TokenKind::Assign {
                    kind: AssignmentKind::ModAssign,
                },
                45,
                47,
            ),
            empty_token(
                TokenKind::Assign {
                    kind: AssignmentKind::MulAssign,
                },
                48,
                50,
            ),
            empty_token(TokenKind::Or, 51, 53),
            empty_token(TokenKind::And, 54, 56),
        ],
    );
}

#[test]
fn tokens_strings() {
    test_tokens(
        r#""Hello, World"; "Test""#,
        vec![
            token(
                TokenKind::Literal {
                    kind: LiteralKind::String,
                },
                "Hello, World",
                0,
                14,
            ),
            empty_token(TokenKind::Semicolon, 14, 15),
            token(
                TokenKind::Literal {
                    kind: LiteralKind::String,
                },
                "Test",
                16,
                22,
            ),
        ],
    );
}

#[test]
fn tokens_integers() {
    test_tokens(
        "123; 456; 789 + 1",
        vec![
            token(
                TokenKind::Literal {
                    kind: LiteralKind::Integer,
                },
                "123",
                0,
                3,
            ),
            empty_token(TokenKind::Semicolon, 3, 4),
            token(
                TokenKind::Literal {
                    kind: LiteralKind::Integer,
                },
                "456",
                5,
                8,
            ),
            empty_token(TokenKind::Semicolon, 8, 9),
            token(
                TokenKind::Literal {
                    kind: LiteralKind::Integer,
                },
                "789",
                10,
                13,
            ),
            empty_token(TokenKind::Add, 14, 15),
            token(
                TokenKind::Literal {
                    kind: LiteralKind::Integer,
                },
                "1",
                16,
                17,
            ),
        ],
    );
}

#[test]
fn tokens_identifiers() {
    test_tokens(
        "x; x + y",
        vec![
            token(TokenKind::Ident, "x", 0, 1),
            empty_token(TokenKind::Semicolon, 1, 2),
            token(TokenKind::Ident, "x", 3, 4),
            empty_token(TokenKind::Add, 5, 6),
            token(TokenKind::Ident, "y", 7, 8),
        ],
    );
}

#[test]
fn tokens_escape_strings() {
    test_tokens(
        r#""\n\r\t\"\x41""#,
        vec![token(
            TokenKind::Literal {
                kind: LiteralKind::String,
            },
            "\n\r\t\"A",
            0,
            14,
        )],
    );
}

use belc_lexer::{AssignmentKind, Lexer, LiteralKind, Token};

fn test_tokens(input: &str, expected: Vec<Token>) {
    let source = input.to_owned();
    let mut lexer = Lexer::new(&source);
    let mut result = Vec::new();
    while let Ok(token) = lexer.next_token() {
        if let Token::EOF = token {
            break;
        }
        result.push(token);
    }
    assert_eq!(result, expected);
}

#[test]
fn tokens_all() {
    test_tokens(
        "=+(){}[],;!-/*5;5 < 10 > 5;:= >= <= += -= /= %= *= || &&",
        vec![
            Token::Assign {
                kind: AssignmentKind::Assign,
            },
            Token::Add,
            Token::LeftParen,
            Token::RightParen,
            Token::LeftBrace,
            Token::RightBrace,
            Token::LeftBracket,
            Token::RightBracket,
            Token::Comma,
            Token::Semicolon,
            Token::Not,
            Token::Sub,
            Token::Div,
            Token::Mul,
            Token::Literal {
                kind: LiteralKind::Integer,
                value: "5".into(),
            },
            Token::Semicolon,
            Token::Literal {
                kind: LiteralKind::Integer,
                value: "5".into(),
            },
            Token::Lt,
            Token::Literal {
                kind: LiteralKind::Integer,
                value: "10".into(),
            },
            Token::Gt,
            Token::Literal {
                kind: LiteralKind::Integer,
                value: "5".into(),
            },
            Token::Semicolon,
            Token::Assign {
                kind: AssignmentKind::ColonAssign,
            },
            Token::Ge,
            Token::Le,
            Token::Assign {
                kind: AssignmentKind::AddAssign,
            },
            Token::Assign {
                kind: AssignmentKind::SubAssign,
            },
            Token::Assign {
                kind: AssignmentKind::DivAssign,
            },
            Token::Assign {
                kind: AssignmentKind::ModAssign,
            },
            Token::Assign {
                kind: AssignmentKind::MulAssign,
            },
            Token::Or,
            Token::And,
        ],
    );
}

#[test]
fn tokens_strings() {
    test_tokens(
        r#""Hello, World"; "Test""#,
        vec![
            Token::Literal {
                kind: LiteralKind::String,
                value: "Hello, World".into(),
            },
            Token::Semicolon,
            Token::Literal {
                kind: LiteralKind::String,
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
            Token::Literal {
                kind: LiteralKind::Integer,
                value: "123".into(),
            },
            Token::Semicolon,
            Token::Literal {
                kind: LiteralKind::Integer,
                value: "456".into(),
            },
            Token::Semicolon,
            Token::Literal {
                kind: LiteralKind::Integer,
                value: "789".into(),
            },
            Token::Add,
            Token::Literal {
                kind: LiteralKind::Integer,
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
            Token::Ident { value: "x".into() },
            Token::Semicolon,
            Token::Ident { value: "x".into() },
            Token::Add,
            Token::Ident { value: "y".into() },
        ],
    );
}

#[test]
fn tokens_escape_strings() {
    test_tokens(
        r#""\n\r\t\"\x41""#,
        vec![Token::Literal {
            kind: LiteralKind::String,
            value: "\n\r\t\"A".into(),
        }],
    );
}

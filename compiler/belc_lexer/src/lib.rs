mod lexer;

pub use lexer::*;

/// Belalang language's tokens
///
/// This is all tokens that exist in the belalang language grammar.
#[derive(PartialEq, Eq, Debug, Clone, Default)]
pub enum Token {
    /// End of file marker
    #[default]
    EOF,

    /// Empty token placeholder
    Empty,

    /// Identifier token containing a variable or function name
    Ident { value: String },

    /// Literals
    Literal { kind: LiteralKind, value: String },

    /// Assignments
    Assign { kind: AssignmentKind },

    /// Addition operator `+`
    Add,
    /// Subtraction operator `-`
    Sub,
    /// Multiplication operator `*`
    Mul,
    /// Division operator `/`
    Div,
    /// Modulo operator `%`
    Mod,

    /// Logical NOT operator `!`
    Not,
    /// Logical AND operator `&&`
    And,
    /// Logical OR operator `||`
    Or,

    /// Bitwise AND operator `&`
    BitAnd,
    /// Bitwise OR operator `|`
    BitOr,
    /// Bitwise XOR operator `^`
    BitXor,
    /// Shift left operator `<<`
    ShiftLeft,
    /// Shift right operator `>>`
    ShiftRight,

    /// Equality comparison operator `==`
    Eq,
    /// Inequality comparison operator `!=`
    Ne,

    /// Less than operator `<`
    Lt,
    /// Less than or equal operator `<=`
    Le,
    /// Greater than operator `>`
    Gt,
    /// Greater than or equal operator `>=`
    Ge,

    /// Left parenthesis `()`
    LeftParen,
    /// Right parenthesis `)`
    RightParen,

    /// Left brace `{`
    LeftBrace,
    /// Right brace `}`
    RightBrace,

    /// Left bracket `[`
    LeftBracket,
    /// Right bracket `]`
    RightBracket,

    /// Function keyword `fn`
    Function,
    /// While loop keyword `while`
    While,
    /// If conditional keyword `if`
    If,
    /// Else conditional keyword `else`
    Else,
    /// Return keyword `return`
    Return,
    /// Boolean true literal `true`
    True,
    /// Boolean false literal `false`
    False,

    /// Comma separator `,`
    Comma,
    /// Semicolon terminator `;`
    Semicolon,
    /// Backslash character `\`
    Backslash,
}

/// Literal types supported by the lexer
#[derive(PartialEq, Eq, Debug, Clone)]
pub enum LiteralKind {
    Integer,
    Float,
    String,
}

/// Assignment types supported by the lexer
#[derive(PartialEq, Eq, Debug, Clone)]
pub enum AssignmentKind {
    /// Assignment operator `=`
    Assign,
    /// Colon assignment operator `:=`
    ColonAssign,
    /// Addition assignment operator `+=`
    AddAssign,
    /// Subtraction assignment operator `-=`
    SubAssign,
    /// Multiplication assignment operator `*=`
    MulAssign,
    /// Division assignment operator `/=`
    DivAssign,
    /// Modulo assignment operator `%=`
    ModAssign,
    /// Bitwise AND assignment operator `&=`
    BitAndAssign,
    /// Bitwise OR assignment operator `|=`
    BitOrAssign,
    /// Bitwise XOR assignment operator `^=`
    BitXorAssign,
    /// Shift left assignment operator `<<=`
    ShiftLeftAssign,
    /// Shift right assignment operator `>>=`
    ShiftRightAssign,
}

impl std::fmt::Display for AssignmentKind {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str(match self {
            Self::Assign => "=",
            Self::ColonAssign => ":=",
            Self::AddAssign => "+=",
            Self::SubAssign => "-=",
            Self::MulAssign => "*=",
            Self::DivAssign => "/=",
            Self::ModAssign => "%=",
            Self::BitAndAssign => "&=",
            Self::BitOrAssign => "|=",
            Self::BitXorAssign => "^=",
            Self::ShiftLeftAssign => "<<=",
            Self::ShiftRightAssign => ">>=",
        })
    }
}

/// Prefix operators supported by the lexer
#[derive(PartialEq, Eq, Debug, Clone)]
pub enum PrefixKind {
    Not,
    Sub,
}

impl std::fmt::Display for PrefixKind {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str(match self {
            Self::Not => "!",
            Self::Sub => "-",
        })
    }
}

#[derive(PartialEq, Eq, Debug, Clone)]
pub enum InfixKind {
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Eq,
    Ne,
    Gt,
    Ge,
    Lt,
    Le,
    BitAnd,
    BitOr,
    BitXor,
    ShiftLeft,
    ShiftRight,
    Or,
    And,
}

impl std::fmt::Display for InfixKind {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str(match self {
            Self::Add => "+",
            Self::Sub => "-",
            Self::Mul => "*",
            Self::Div => "/",
            Self::Mod => "%",
            Self::Eq => "==",
            Self::Ne => "!=",
            Self::Gt => ">",
            Self::Ge => ">=",
            Self::Lt => "<",
            Self::Le => "<=",
            Self::BitAnd => "&",
            Self::BitOr => "|",
            Self::BitXor => "^",
            Self::ShiftLeft => "<<",
            Self::ShiftRight => ">>",
            Self::Or => "||",
            Self::And => "&&",
        })
    }
}

impl From<&str> for Token {
    fn from(value: &str) -> Self {
        match value {
            "fn" => Token::Function,
            "while" => Token::While,
            "true" => Token::True,
            "false" => Token::False,
            "if" => Token::If,
            "else" => Token::Else,
            "return" => Token::Return,
            _ => Token::Ident {
                value: value.to_string(),
            },
        }
    }
}

impl std::fmt::Display for Token {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        if let Token::Assign { kind } = self {
            f.write_str(&kind.to_string())?;
            return Ok(());
        }

        f.write_str(match self {
            Token::Empty => "<empty>",
            Token::EOF => "EOF",

            Token::Ident { value } => value,
            Token::Literal { value, .. } => value,

            Token::Add => "+",
            Token::Sub => "-",
            Token::Mul => "*",
            Token::Div => "/",
            Token::Mod => "%",

            Token::Not => "!",
            Token::And => "&&",
            Token::Or => "||",

            Token::BitAnd => "&",
            Token::BitOr => "|",
            Token::BitXor => "^",
            Token::ShiftLeft => "<<",
            Token::ShiftRight => ">>",

            Token::Eq => "==",
            Token::Ne => "!=",
            Token::Lt => "<",
            Token::Le => "<=",
            Token::Gt => ">",
            Token::Ge => ">=",

            Token::LeftParen => "(",
            Token::RightParen => ")",
            Token::LeftBrace => "{",
            Token::RightBrace => "}",
            Token::LeftBracket => "[",
            Token::RightBracket => "]",

            Token::Function => "fn",
            Token::While => "while",
            Token::If => "if",
            Token::Else => "else",
            Token::Return => "return",
            Token::True => "true",
            Token::False => "false",

            Token::Comma => ",",
            Token::Semicolon => ";",
            Token::Backslash => r"\",

            _ => unreachable!(),
        })
    }
}

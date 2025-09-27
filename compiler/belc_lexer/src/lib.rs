mod lexer;

pub use lexer::*;

#[derive(PartialEq, Eq, Debug, Clone, Default)]
pub struct Token {
    pub kind: TokenKind,
    pub value: String,
}

impl From<&str> for Token {
    fn from(value: &str) -> Self {
        let kind = match value {
            "fn" => TokenKind::Function,
            "while" => TokenKind::While,
            "true" => TokenKind::True,
            "false" => TokenKind::False,
            "if" => TokenKind::If,
            "else" => TokenKind::Else,
            "return" => TokenKind::Return,
            _ => TokenKind::Ident,
        };

        Self {
            kind,
            value: value.to_string(),
        }
    }
}

/// Belalang language's tokens
///
/// This is all tokens that exist in the belalang language grammar.
#[derive(PartialEq, Eq, Debug, Clone, Copy, Default)]
pub enum TokenKind {
    /// End of file marker
    #[default]
    EOF,

    /// Empty token placeholder
    Empty,

    /// Identifier token containing a variable or function name
    Ident,

    /// Literals
    Literal { kind: LiteralKind },

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
#[derive(PartialEq, Eq, Debug, Clone, Copy)]
pub enum LiteralKind {
    Integer,
    Float,
    String,
}

/// Assignment types supported by the lexer
#[derive(PartialEq, Eq, Debug, Clone, Copy)]
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

impl std::fmt::Display for TokenKind {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        if let TokenKind::Assign { kind } = self {
            f.write_str(&kind.to_string())?;
            return Ok(());
        }

        f.write_str(match self {
            TokenKind::Empty => "<empty>",
            TokenKind::EOF => "EOF",

            TokenKind::Ident => "<ident>",
            TokenKind::Literal { .. } => "<literal>",

            TokenKind::Add => "+",
            TokenKind::Sub => "-",
            TokenKind::Mul => "*",
            TokenKind::Div => "/",
            TokenKind::Mod => "%",

            TokenKind::Not => "!",
            TokenKind::And => "&&",
            TokenKind::Or => "||",

            TokenKind::BitAnd => "&",
            TokenKind::BitOr => "|",
            TokenKind::BitXor => "^",
            TokenKind::ShiftLeft => "<<",
            TokenKind::ShiftRight => ">>",

            TokenKind::Eq => "==",
            TokenKind::Ne => "!=",
            TokenKind::Lt => "<",
            TokenKind::Le => "<=",
            TokenKind::Gt => ">",
            TokenKind::Ge => ">=",

            TokenKind::LeftParen => "(",
            TokenKind::RightParen => ")",
            TokenKind::LeftBrace => "{",
            TokenKind::RightBrace => "}",
            TokenKind::LeftBracket => "[",
            TokenKind::RightBracket => "]",

            TokenKind::Function => "fn",
            TokenKind::While => "while",
            TokenKind::If => "if",
            TokenKind::Else => "else",
            TokenKind::Return => "return",
            TokenKind::True => "true",
            TokenKind::False => "false",

            TokenKind::Comma => ",",
            TokenKind::Semicolon => ";",
            TokenKind::Backslash => r"\",

            _ => unreachable!(),
        })
    }
}

use belalang_lexer::{
    InfixKind,
    Lexer,
    LiteralKind,
    PrefixKind,
    Token,
    TokenKind,
};

use super::{
    Expression,
    ParserError,
    Statement,
};
use crate::{
    ArrayLiteral,
    BlockExpression,
    BooleanExpression,
    CallExpression,
    ExpressionStatement,
    FloatLiteral,
    FunctionLiteral,
    Identifier,
    IfExpression,
    IndexExpression,
    InfixExpression,
    IntegerLiteral,
    PrefixExpression,
    Program,
    ReturnStatement,
    StringLiteral,
    VarExpression,
    WhileStatement,
};

#[derive(Debug, PartialEq, PartialOrd)]
pub enum Precedence {
    Lowest,
    AssignmentOps,
    LogicalOr,
    LogicalAnd,
    BitOr,
    BitXor,
    BitAnd,
    Equality,
    Relational,
    Shift,
    Additive,
    Multiplicative,
    Prefix,
    Call,
    Index,
}

impl From<&TokenKind> for Precedence {
    fn from(value: &TokenKind) -> Self {
        match value {
            TokenKind::Assign { .. } => Self::AssignmentOps,
            TokenKind::Or => Self::LogicalOr,
            TokenKind::And => Self::LogicalAnd,
            TokenKind::BitOr => Self::BitOr,
            TokenKind::BitXor => Self::BitXor,
            TokenKind::BitAnd => Self::BitAnd,
            TokenKind::Eq | TokenKind::Ne => Self::Equality,
            TokenKind::Lt | TokenKind::Le | TokenKind::Gt | TokenKind::Ge => Self::Relational,
            TokenKind::ShiftLeft | TokenKind::ShiftRight => Self::Shift,
            TokenKind::Add | TokenKind::Sub => Self::Additive,
            TokenKind::Div | TokenKind::Mul | TokenKind::Mod => Self::Multiplicative,
            TokenKind::LeftParen => Self::Call,
            TokenKind::LeftBracket => Self::Index,
            _ => Self::Lowest,
        }
    }
}

macro_rules! expect_peek {
    ($self:expr, $token:pat) => {
        if matches!($self.peek_token.kind, $token) {
            $self.next_token()?;
            true
        } else {
            return Err(ParserError::UnexpectedToken($self.peek_token.kind));
        }
    };
}

macro_rules! optional_peek {
    ($self:expr, $token:pat) => {
        if matches!($self.peek_token.kind, $token) {
            $self.next_token()?;
            true
        } else {
            false
        }
    };
}

/// Belalang language parser.
///
/// Responsible for parsing a token stream into an abstract syntax tree. Also
/// see [`Lexer`] and [`Token`].
pub struct Parser<'a> {
    lexer: Lexer<'a>,
    curr_token: Token,
    peek_token: Token,

    depth: i32,
    has_semicolon: bool,
}

impl Parser<'_> {
    /// Creates a new Parser using a [`Lexer`].
    pub fn new(lexer: Lexer<'_>) -> Parser<'_> {
        Parser {
            lexer,
            curr_token: Token::default(),
            peek_token: Token::default(),

            depth: 0,
            has_semicolon: false,
        }
    }

    fn next_token(&mut self) -> Result<(), ParserError> {
        self.curr_token = std::mem::take(&mut self.peek_token);
        self.peek_token = self.lexer.next_token()?;

        Ok(())
    }

    /// Parses the token stream into a [`Program`] instance.
    ///
    /// Continues parsing the token stream until the end of input is reached.
    /// Each statement and expression is parsed and added to the program.
    pub fn parse_program(&mut self) -> Result<Program, ParserError> {
        self.curr_token = self.lexer.next_token()?;
        self.peek_token = self.lexer.next_token()?;

        let mut program = Program::default();

        while !matches!(self.curr_token.kind, TokenKind::EOF) {
            program.add_stmt(self.parse_statement()?);
            self.next_token()?;
        }

        Ok(program)
    }

    fn parse_statement(&mut self) -> Result<Statement, ParserError> {
        match self.curr_token.kind {
            // parse_return
            TokenKind::Return => {
                self.next_token()?;
                let return_value = self.parse_expression(Precedence::Lowest)?;

                self.has_semicolon = expect_peek!(self, TokenKind::Semicolon);

                Ok(Statement::Return(ReturnStatement { return_value }))
            },

            // parse_while
            TokenKind::While => {
                self.next_token()?;
                let condition = self.parse_expression(Precedence::Lowest)?;

                expect_peek!(self, TokenKind::LeftBrace);

                let block = self.parse_block()?;

                self.has_semicolon = optional_peek!(self, TokenKind::Semicolon);

                Ok(Statement::While(WhileStatement {
                    condition: Box::new(condition),
                    block,
                }))
            },

            // parse_if: parse if expression as statement
            TokenKind::If => {
                let expression = self.parse_if()?;

                self.has_semicolon = optional_peek!(self, TokenKind::Semicolon);

                Ok(Statement::Expression(ExpressionStatement { expression }))
            },

            _ => {
                let stmt = ExpressionStatement {
                    expression: self.parse_expression(Precedence::Lowest)?,
                };

                self.has_semicolon = if self.depth == 0 {
                    expect_peek!(self, TokenKind::Semicolon)
                } else {
                    optional_peek!(self, TokenKind::Semicolon)
                };

                Ok(Statement::Expression(stmt))
            },
        }
    }

    fn parse_expression(&mut self, precedence: Precedence) -> Result<Expression, ParserError> {
        let mut left_expr = self.parse_prefix()?;

        while precedence < Precedence::from(&self.peek_token.kind) {
            match self.parse_infix(&left_expr)? {
                Some(expr) => left_expr = expr,
                None => return Ok(left_expr),
            };
        }

        Ok(left_expr)
    }

    fn parse_block(&mut self) -> Result<BlockExpression, ParserError> {
        let mut statements = Vec::new();

        self.next_token()?;

        self.depth += 1;
        while !matches!(self.curr_token.kind, TokenKind::RightBrace | TokenKind::EOF) {
            statements.push(self.parse_statement()?);
            self.next_token()?;
        }
        self.depth -= 1;

        Ok(BlockExpression { statements })
    }

    fn parse_if(&mut self) -> Result<Expression, ParserError> {
        self.next_token()?;
        let condition = self.parse_expression(Precedence::Lowest)?;

        expect_peek!(self, TokenKind::LeftBrace);

        let consequence = self.parse_block()?;

        let alternative: Option<Box<Expression>> = if matches!(self.peek_token.kind, TokenKind::Else) {
            self.next_token()?;
            self.next_token()?;

            Some(Box::new(match self.curr_token.kind {
                TokenKind::If => self.parse_if()?,
                TokenKind::LeftBrace => Expression::Block(self.parse_block()?),
                _ => return Err(ParserError::UnexpectedToken(self.curr_token.kind)),
            }))
        } else {
            None
        };

        Ok(Expression::If(IfExpression {
            condition: Box::new(condition),
            consequence,
            alternative,
        }))
    }

    fn parse_infix(&mut self, left: &Expression) -> Result<Option<Expression>, ParserError> {
        match self.peek_token.kind {
            // parse_infix: parse infix expression
            TokenKind::Add
            | TokenKind::Sub
            | TokenKind::Mul
            | TokenKind::Div
            | TokenKind::Mod
            | TokenKind::Eq
            | TokenKind::Ne
            | TokenKind::Gt
            | TokenKind::Ge
            | TokenKind::Lt
            | TokenKind::Le
            | TokenKind::BitAnd
            | TokenKind::BitOr
            | TokenKind::BitXor
            | TokenKind::ShiftLeft
            | TokenKind::ShiftRight
            | TokenKind::Or
            | TokenKind::And => {
                self.next_token()?;

                let operator = self.curr_token.clone();
                let precedence = Precedence::from(&self.curr_token.kind);

                self.next_token()?;

                let right = self.parse_expression(precedence)?;

                Ok(Some(Expression::Infix(InfixExpression {
                    left: Box::new(left.clone()),
                    operator: match operator.kind {
                        TokenKind::Add => InfixKind::Add,
                        TokenKind::Sub => InfixKind::Sub,
                        TokenKind::Mul => InfixKind::Mul,
                        TokenKind::Div => InfixKind::Div,
                        TokenKind::Mod => InfixKind::Mod,
                        TokenKind::Eq => InfixKind::Eq,
                        TokenKind::Ne => InfixKind::Ne,
                        TokenKind::Gt => InfixKind::Gt,
                        TokenKind::Ge => InfixKind::Ge,
                        TokenKind::Lt => InfixKind::Lt,
                        TokenKind::Le => InfixKind::Le,
                        TokenKind::BitAnd => InfixKind::BitAnd,
                        TokenKind::BitOr => InfixKind::BitOr,
                        TokenKind::BitXor => InfixKind::BitXor,
                        TokenKind::ShiftLeft => InfixKind::ShiftLeft,
                        TokenKind::ShiftRight => InfixKind::ShiftRight,
                        TokenKind::Or => InfixKind::Or,
                        TokenKind::And => InfixKind::And,
                        _ => unreachable!(),
                    },
                    right: Box::new(right),
                })))
            },

            // parse_call: parse call expression
            TokenKind::LeftParen => {
                self.next_token()?;
                self.next_token()?;

                let mut args = Vec::new();

                if !matches!(self.curr_token.kind, TokenKind::RightParen) {
                    loop {
                        args.push(self.parse_expression(Precedence::Lowest)?);

                        if !matches!(self.peek_token.kind, TokenKind::Comma) {
                            break;
                        }

                        self.next_token()?;
                        self.next_token()?;
                    }

                    expect_peek!(self, TokenKind::RightParen);
                }

                Ok(Some(Expression::Call(CallExpression {
                    function: Box::new(left.clone()),
                    args,
                })))
            },

            TokenKind::LeftBracket => {
                self.next_token()?;
                self.next_token()?;

                let index = Box::new(self.parse_expression(Precedence::Lowest)?);

                expect_peek!(self, TokenKind::RightBracket);

                Ok(Some(Expression::Index(IndexExpression {
                    left: Box::new(left.clone()),
                    index,
                })))
            },

            TokenKind::Assign { ref kind } => {
                let kind = *kind;
                if !matches!(left, Expression::Identifier(_)) {
                    return Err(ParserError::InvalidLHS(left.clone()));
                }

                let name = Identifier {
                    value: self.curr_token.value.clone(),
                };

                self.next_token()?;

                self.next_token()?;
                let value = Box::new(self.parse_expression(Precedence::Lowest)?);

                Ok(Some(Expression::Var(VarExpression { kind, name, value })))
            },

            _ => Ok(None),
        }
    }

    fn parse_prefix(&mut self) -> Result<Expression, ParserError> {
        match self.curr_token.kind {
            // parse_identifier: parse current token as identifier
            TokenKind::Ident => Ok(Expression::Identifier(Identifier {
                value: self.curr_token.value.clone(),
            })),

            TokenKind::Literal { ref kind } => match kind {
                LiteralKind::Integer => match self.curr_token.value.parse::<i64>() {
                    Ok(lit) => Ok(Expression::Integer(IntegerLiteral { value: lit })),
                    Err(_) => Err(ParserError::ParsingInteger(self.curr_token.value.clone())),
                },
                LiteralKind::Float => match self.curr_token.value.parse::<f64>() {
                    Ok(lit) => Ok(Expression::Float(FloatLiteral { value: lit })),
                    Err(_) => Err(ParserError::ParsingFloat(self.curr_token.value.clone())),
                },
                LiteralKind::String => Ok(Expression::String(StringLiteral {
                    value: self.curr_token.value.clone(),
                })),
            },

            // parse_boolean: parse current token as boolean
            TokenKind::True | TokenKind::False => Ok(Expression::Boolean(BooleanExpression {
                value: matches!(self.curr_token.kind, TokenKind::True),
            })),

            // parse_array
            TokenKind::LeftBracket => Ok(Expression::Array(ArrayLiteral {
                elements: {
                    self.next_token()?;

                    let mut elements = Vec::new();

                    if !matches!(self.curr_token.kind, TokenKind::RightBracket) {
                        loop {
                            elements.push(self.parse_expression(Precedence::Lowest)?);

                            if !matches!(self.peek_token.kind, TokenKind::Comma) {
                                break;
                            }

                            self.next_token()?;
                            self.next_token()?;
                        }

                        expect_peek!(self, TokenKind::RightBracket);
                    }

                    elements
                },
            })),

            // parse_prefix: parse current expression with prefix
            TokenKind::Not | TokenKind::Sub => {
                let prev_token = self.curr_token.clone();

                self.next_token()?;

                let right = self.parse_expression(Precedence::Prefix).unwrap();

                Ok(Expression::Prefix(PrefixExpression {
                    operator: match prev_token.kind {
                        TokenKind::Not => PrefixKind::Not,
                        TokenKind::Sub => PrefixKind::Sub,
                        _ => unreachable!(),
                    },
                    right: Box::new(right),
                }))
            },

            // parse_grouped: parse grouped expression
            TokenKind::LeftParen => {
                self.next_token()?;
                let expr = self.parse_expression(Precedence::Lowest);

                expect_peek!(self, TokenKind::RightParen);

                expr
            },

            // parse_block
            TokenKind::LeftBrace => {
                let block = self.parse_block()?;
                Ok(Expression::Block(block))
            },

            // parse_if: parse current if expression
            TokenKind::If => self.parse_if(),

            // parse_function: parse current expression as function
            TokenKind::Function => {
                let mut params = Vec::new();

                expect_peek!(self, TokenKind::LeftParen);

                self.next_token()?;

                if !matches!(self.curr_token.kind, TokenKind::RightParen) {
                    params.push(Identifier {
                        value: self.curr_token.value.clone(),
                    });

                    while matches!(self.peek_token.kind, TokenKind::Comma) {
                        self.next_token()?;
                        self.next_token()?;

                        params.push(Identifier {
                            value: self.curr_token.value.clone(),
                        });
                    }

                    expect_peek!(self, TokenKind::RightParen);
                }

                expect_peek!(self, TokenKind::LeftBrace);

                let body = self.parse_block()?;

                Ok(Expression::Function(FunctionLiteral { params, body }))
            },

            _ => Err(ParserError::UnknownPrefixOperator(self.curr_token.kind)),
        }
    }
}

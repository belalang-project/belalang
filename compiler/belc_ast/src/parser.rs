use belc_lexer::InfixKind;
use belc_lexer::Lexer;
use belc_lexer::LiteralKind;
use belc_lexer::PrefixKind;
use belc_lexer::Token;

use super::{Expression, ParserError, Statement};
use crate::ArrayLiteral;
use crate::BlockExpression;
use crate::BooleanExpression;
use crate::CallExpression;
use crate::ExpressionStatement;
use crate::FloatLiteral;
use crate::FunctionLiteral;
use crate::Identifier;
use crate::IfExpression;
use crate::IndexExpression;
use crate::InfixExpression;
use crate::IntegerLiteral;
use crate::PrefixExpression;
use crate::Program;
use crate::ReturnStatement;
use crate::StringLiteral;
use crate::VarExpression;
use crate::WhileStatement;

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

impl From<&Token> for Precedence {
    fn from(value: &Token) -> Self {
        match value {
            Token::Assign { .. } => Self::AssignmentOps,
            Token::Or => Self::LogicalOr,
            Token::And => Self::LogicalAnd,
            Token::BitOr => Self::BitOr,
            Token::BitXor => Self::BitXor,
            Token::BitAnd => Self::BitAnd,
            Token::Eq | Token::Ne => Self::Equality,
            Token::Lt | Token::Le | Token::Gt | Token::Ge => Self::Relational,
            Token::ShiftLeft | Token::ShiftRight => Self::Shift,
            Token::Add | Token::Sub => Self::Additive,
            Token::Div | Token::Mul | Token::Mod => Self::Multiplicative,
            Token::LeftParen => Self::Call,
            Token::LeftBracket => Self::Index,
            _ => Self::Lowest,
        }
    }
}

macro_rules! expect_peek {
    ($self:expr, $token:pat) => {
        if matches!($self.peek_token, $token) {
            $self.next_token()?;
            true
        } else {
            return Err(ParserError::UnexpectedToken($self.peek_token.clone()));
        }
    };
}

macro_rules! optional_peek {
    ($self:expr, $token:pat) => {
        if matches!($self.peek_token, $token) {
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

        while !matches!(self.curr_token, Token::EOF) {
            program.add_stmt(self.parse_statement()?);
            self.next_token()?;
        }

        Ok(program)
    }

    fn parse_statement(&mut self) -> Result<Statement, ParserError> {
        match self.curr_token {
            // parse_return
            Token::Return => {
                self.next_token()?;
                let return_value = self.parse_expression(Precedence::Lowest)?;

                self.has_semicolon = expect_peek!(self, Token::Semicolon);

                Ok(Statement::Return(ReturnStatement { return_value }))
            },

            // parse_while
            Token::While => {
                self.next_token()?;
                let condition = self.parse_expression(Precedence::Lowest)?;

                expect_peek!(self, Token::LeftBrace);

                let block = self.parse_block()?;

                self.has_semicolon = optional_peek!(self, Token::Semicolon);

                Ok(Statement::While(WhileStatement {
                    condition: Box::new(condition),
                    block,
                }))
            },

            // parse_if: parse if expression as statement
            Token::If => {
                let expression = self.parse_if()?;

                self.has_semicolon = optional_peek!(self, Token::Semicolon);

                Ok(Statement::Expression(ExpressionStatement { expression }))
            },

            _ => {
                let stmt = ExpressionStatement {
                    expression: self.parse_expression(Precedence::Lowest)?,
                };

                self.has_semicolon = if self.depth == 0 {
                    expect_peek!(self, Token::Semicolon)
                } else {
                    optional_peek!(self, Token::Semicolon)
                };

                Ok(Statement::Expression(stmt))
            },
        }
    }

    fn parse_expression(&mut self, precedence: Precedence) -> Result<Expression, ParserError> {
        let mut left_expr = self.parse_prefix()?;

        while precedence < Precedence::from(&self.peek_token) {
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
        while !matches!(self.curr_token, Token::RightBrace | Token::EOF) {
            statements.push(self.parse_statement()?);
            self.next_token()?;
        }
        self.depth -= 1;

        Ok(BlockExpression { statements })
    }

    fn parse_if(&mut self) -> Result<Expression, ParserError> {
        self.next_token()?;
        let condition = self.parse_expression(Precedence::Lowest)?;

        expect_peek!(self, Token::LeftBrace);

        let consequence = self.parse_block()?;

        let alternative: Option<Box<Expression>> = if matches!(self.peek_token, Token::Else) {
            self.next_token()?;
            self.next_token()?;

            Some(Box::new(match self.curr_token {
                Token::If => self.parse_if()?,
                Token::LeftBrace => Expression::Block(self.parse_block()?),
                _ => return Err(ParserError::UnexpectedToken(self.curr_token.clone())),
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
        match self.peek_token {
            // parse_infix: parse infix expression
            Token::Add
            | Token::Sub
            | Token::Mul
            | Token::Div
            | Token::Mod
            | Token::Eq
            | Token::Ne
            | Token::Gt
            | Token::Ge
            | Token::Lt
            | Token::Le
            | Token::BitAnd
            | Token::BitOr
            | Token::BitXor
            | Token::ShiftLeft
            | Token::ShiftRight
            | Token::Or
            | Token::And => {
                self.next_token()?;

                let operator = self.curr_token.clone();
                let precedence = Precedence::from(&self.curr_token);

                self.next_token()?;

                let right = self.parse_expression(precedence)?;

                Ok(Some(Expression::Infix(InfixExpression {
                    left: Box::new(left.clone()),
                    operator: match operator {
                        Token::Add => InfixKind::Add,
                        Token::Sub => InfixKind::Sub,
                        Token::Mul => InfixKind::Mul,
                        Token::Div => InfixKind::Div,
                        Token::Mod => InfixKind::Mod,
                        Token::Eq => InfixKind::Eq,
                        Token::Ne => InfixKind::Ne,
                        Token::Gt => InfixKind::Gt,
                        Token::Ge => InfixKind::Ge,
                        Token::Lt => InfixKind::Lt,
                        Token::Le => InfixKind::Le,
                        Token::BitAnd => InfixKind::BitAnd,
                        Token::BitOr => InfixKind::BitOr,
                        Token::BitXor => InfixKind::BitXor,
                        Token::ShiftLeft => InfixKind::ShiftLeft,
                        Token::ShiftRight => InfixKind::ShiftRight,
                        Token::Or => InfixKind::Or,
                        Token::And => InfixKind::And,
                        _ => unreachable!(),
                    },
                    right: Box::new(right),
                })))
            },

            // parse_call: parse call expression
            Token::LeftParen => {
                self.next_token()?;
                self.next_token()?;

                let mut args = Vec::new();

                if !matches!(self.curr_token, Token::RightParen) {
                    loop {
                        args.push(self.parse_expression(Precedence::Lowest)?);

                        if !matches!(self.peek_token, Token::Comma) {
                            break;
                        }

                        self.next_token()?;
                        self.next_token()?;
                    }

                    expect_peek!(self, Token::RightParen);
                }

                Ok(Some(Expression::Call(CallExpression {
                    function: Box::new(left.clone()),
                    args,
                })))
            },

            Token::LeftBracket => {
                self.next_token()?;
                self.next_token()?;

                let index = Box::new(self.parse_expression(Precedence::Lowest)?);

                expect_peek!(self, Token::RightBracket);

                Ok(Some(Expression::Index(IndexExpression {
                    left: Box::new(left.clone()),
                    index,
                })))
            },

            Token::Assign { ref kind } => {
                let kind = kind.clone();
                if !matches!(left, Expression::Identifier(_)) {
                    return Err(ParserError::InvalidLHS(left.clone()));
                }

                let name = Identifier {
                    value: self.curr_token.to_string(),
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
        match self.curr_token {
            // parse_identifier: parse current token as identifier
            Token::Ident { ref value } => Ok(Expression::Identifier(Identifier { value: value.clone() })),

            Token::Literal { ref kind, ref value } => match kind {
                LiteralKind::Integer => match value.parse::<i64>() {
                    Ok(lit) => Ok(Expression::Integer(IntegerLiteral { value: lit })),
                    Err(_) => Err(ParserError::ParsingInteger(value.into())),
                },
                LiteralKind::Float => match value.parse::<f64>() {
                    Ok(lit) => Ok(Expression::Float(FloatLiteral { value: lit })),
                    Err(_) => Err(ParserError::ParsingFloat(value.into())),
                },
                LiteralKind::String => Ok(Expression::String(StringLiteral { value: value.into() })),
            },

            // parse_boolean: parse current token as boolean
            Token::True | Token::False => Ok(Expression::Boolean(BooleanExpression {
                value: matches!(self.curr_token, Token::True),
            })),

            // parse_array
            Token::LeftBracket => Ok(Expression::Array(ArrayLiteral {
                elements: {
                    self.next_token()?;

                    let mut elements = Vec::new();

                    if !matches!(self.curr_token, Token::RightBracket) {
                        loop {
                            elements.push(self.parse_expression(Precedence::Lowest)?);

                            if !matches!(self.peek_token, Token::Comma) {
                                break;
                            }

                            self.next_token()?;
                            self.next_token()?;
                        }

                        expect_peek!(self, Token::RightBracket);
                    }

                    elements
                },
            })),

            // parse_prefix: parse current expression with prefix
            Token::Not | Token::Sub => {
                let prev_token = self.curr_token.clone();

                self.next_token()?;

                let right = self.parse_expression(Precedence::Prefix).unwrap();

                Ok(Expression::Prefix(PrefixExpression {
                    operator: match prev_token {
                        Token::Not => PrefixKind::Not,
                        Token::Sub => PrefixKind::Sub,
                        _ => unreachable!(),
                    },
                    right: Box::new(right),
                }))
            },

            // parse_grouped: parse grouped expression
            Token::LeftParen => {
                self.next_token()?;
                let expr = self.parse_expression(Precedence::Lowest);

                expect_peek!(self, Token::RightParen);

                expr
            },

            // parse_block
            Token::LeftBrace => {
                let block = self.parse_block()?;
                Ok(Expression::Block(block))
            },

            // parse_if: parse current if expression
            Token::If => self.parse_if(),

            // parse_function: parse current expression as function
            Token::Function => {
                let mut params = Vec::new();

                expect_peek!(self, Token::LeftParen);

                self.next_token()?;

                if !matches!(self.curr_token, Token::RightParen) {
                    params.push(Identifier {
                        value: self.curr_token.to_string(),
                    });

                    while matches!(self.peek_token, Token::Comma) {
                        self.next_token()?;
                        self.next_token()?;

                        params.push(Identifier {
                            value: self.curr_token.to_string(),
                        });
                    }

                    expect_peek!(self, Token::RightParen);
                }

                expect_peek!(self, Token::LeftBrace);

                let body = self.parse_block()?;

                Ok(Expression::Function(FunctionLiteral { params, body }))
            },

            _ => Err(ParserError::UnknownPrefixOperator(self.curr_token.clone())),
        }
    }
}

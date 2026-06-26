use diag::{
    Diagnostic,
    Label,
};
use lexer::{
    AssignmentKind,
    InfixKind,
    Lexer,
    LiteralKind,
    PrefixKind,
    Token,
    TokenKind,
};
use session::{
    Session,
    interner::syms,
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
    VarDeclExpression,
    VarExpression,
    WhileStatement,
    type_inferer::Type,
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
            TokenKind::Assign { .. } | TokenKind::Colon => Self::AssignmentOps,
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
            return Err($self.error_unexpected_token());
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
pub struct Parser<'sess> {
    #[allow(dead_code)]
    session: &'sess Session,
    lexer: Lexer<'sess>,
    curr_token: Token,
    peek_token: Token,

    depth: i32,
    has_semicolon: bool,
}

impl<'sess> Parser<'sess> {
    /// Creates a new Parser using a [`Lexer`].
    pub fn new(session: &'sess Session, lexer: Lexer<'sess>) -> Parser<'sess> {
        Parser {
            session,
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

                self.has_semicolon = optional_peek!(self, TokenKind::Semicolon);

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

                self.has_semicolon = optional_peek!(self, TokenKind::Semicolon);

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
                _ => return Err(self.error_unexpected_token()),
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
                    return Err(self.error_invalid_lhs(&left));
                }

                let TokenKind::Ident { sym } = self.curr_token.kind else {
                    todo!()
                };

                let name = Identifier { value: sym };

                self.next_token()?;

                self.next_token()?;
                let value = Box::new(self.parse_expression(Precedence::Lowest)?);

                Ok(Some(Expression::Var(VarExpression { kind, name, value })))
            },

            TokenKind::Colon => {
                if !matches!(left, Expression::Identifier(_)) {
                    return Err(self.error_invalid_lhs(&left));
                }
                let name = match left {
                    Expression::Identifier(ident) => ident.clone(),
                    _ => unreachable!(),
                };

                self.next_token()?; // consume name; curr_token is colon
                self.next_token()?; // consume colon; curr_token is type name or assign

                let explicit_ty = if let TokenKind::Assign {
                    kind: AssignmentKind::Assign,
                } = self.curr_token.kind
                {
                    None
                } else {
                    let TokenKind::Ident { sym } = self.curr_token.kind else {
                        return Err(self.error_unexpected_token());
                    };
                    let ty = match sym {
                        syms::INT => Type::Integer,
                        syms::FLOAT => Type::Float,
                        syms::STRING => Type::String,
                        _ => Type::None, // TODO: fill this in
                    };

                    expect_peek!(
                        self,
                        TokenKind::Assign {
                            kind: AssignmentKind::Assign
                        }
                    ); // consumes equal sign; curr_token is Assign
                    Some(ty)
                };

                self.next_token()?; // consume equal sign; curr_token is start of RHS value
                let value = Box::new(self.parse_expression(Precedence::Lowest)?);
                Ok(Some(Expression::VarDecl(VarDeclExpression {
                    name,
                    value,
                    explicit_ty,
                })))
            },

            _ => Ok(None),
        }
    }

    fn parse_prefix(&mut self) -> Result<Expression, ParserError> {
        match self.curr_token.kind {
            // parse_identifier: parse current token as identifier
            TokenKind::Ident { sym } => Ok(Expression::Identifier(Identifier { value: sym })),

            TokenKind::Literal { ref kind, sym } => {
                let str_value = self.session.lookup_string(sym);
                match kind {
                    LiteralKind::Integer => match str_value.parse::<i64>() {
                        Ok(lit) => Ok(Expression::Integer(IntegerLiteral { value: lit })),
                        Err(_) => Err(ParserError::ParsingInteger(str_value.to_string())),
                    },
                    LiteralKind::Float => match str_value.parse::<f64>() {
                        Ok(lit) => Ok(Expression::Float(FloatLiteral { value: lit })),
                        Err(_) => Err(ParserError::ParsingFloat(str_value.to_string())),
                    },
                    LiteralKind::String => Ok(Expression::String(StringLiteral { value: sym })),
                }
            },

            TokenKind::KwTrue => Ok(Expression::Boolean(BooleanExpression { value: true })),

            TokenKind::KwFalse => Ok(Expression::Boolean(BooleanExpression { value: false })),

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
                    let TokenKind::Ident { sym } = self.curr_token.kind else {
                        todo!()
                    };

                    params.push(Identifier { value: sym });

                    while matches!(self.peek_token.kind, TokenKind::Comma) {
                        self.next_token()?;
                        self.next_token()?;

                        let TokenKind::Ident { sym } = self.curr_token.kind else {
                            todo!()
                        };

                        params.push(Identifier { value: sym });
                    }

                    expect_peek!(self, TokenKind::RightParen);
                }

                expect_peek!(self, TokenKind::LeftBrace);

                let body = self.parse_block()?;

                Ok(Expression::Function(FunctionLiteral { params, body }))
            },

            _ => Err(self.error_unknown_prefix_op()),
        }
    }

    fn error_unexpected_token(&self) -> ParserError {
        let label = Label::primary(self.curr_token.span, "unexpected token");
        self.session
            .emit(Diagnostic::error("unexpected token").with_label(label));
        ParserError::UnexpectedToken(self.curr_token.kind)
    }

    fn error_unknown_prefix_op(&self) -> ParserError {
        let label = Label::primary(self.curr_token.span, "unknown prefix");
        self.session.emit(Diagnostic::error("unknown prefix").with_label(label));
        ParserError::UnknownPrefixOperator(self.curr_token.kind)
    }

    fn error_invalid_lhs(&self, left: &Expression) -> ParserError {
        // TODO: change this with expression span
        let label = Label::primary(self.curr_token.span, "invalid lhs");
        self.session.emit(Diagnostic::error("invalid lhs").with_label(label));
        ParserError::InvalidLHS(left.clone())
    }
}

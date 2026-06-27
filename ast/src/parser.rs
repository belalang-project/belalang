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
    Ast,
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
    VarDeclStatement,
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
pub struct Parser<'sess, 'ast> {
    #[allow(dead_code)]
    session: &'sess Session,
    lexer: Lexer<'sess>,
    curr_token: Token,
    peek_token: Token,

    ast: &'ast Ast,

    depth: i32,
    has_semicolon: bool,
}

impl<'sess, 'ast> Parser<'sess, 'ast> {
    /// Creates a new Parser using a [`Lexer`].
    pub fn new(session: &'sess Session, lexer: Lexer<'sess>, ast: &'ast Ast) -> Parser<'sess, 'ast> {
        Parser {
            session,
            lexer,
            curr_token: Token::default(),
            peek_token: Token::default(),
            ast,
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
    pub fn parse_program(&mut self) -> Result<Program<'ast>, ParserError> {
        self.curr_token = self.lexer.next_token()?;
        self.peek_token = self.lexer.next_token()?;

        let mut statements = Vec::new();

        while !matches!(self.curr_token.kind, TokenKind::EOF) {
            statements.push(self.parse_statement()?);
            self.next_token()?;
        }

        Ok(Program {
            statements: self.ast.alloc_slice_clone(&statements),
        })
    }

    fn parse_statement(&mut self) -> Result<Statement<'ast>, ParserError> {
        match self.curr_token.kind {
            // parse_return
            TokenKind::Return => {
                self.next_token()?;
                let return_value = *self.parse_expression(Precedence::Lowest)?;

                self.has_semicolon = optional_peek!(self, TokenKind::Semicolon);

                Ok(Statement::Return(ReturnStatement { return_value }))
            },

            // parse_while
            TokenKind::While => {
                self.next_token()?;
                let condition = *self.parse_expression(Precedence::Lowest)?;

                expect_peek!(self, TokenKind::LeftBrace);

                let block = self.parse_block()?;

                self.has_semicolon = optional_peek!(self, TokenKind::Semicolon);

                Ok(Statement::While(WhileStatement { condition, block }))
            },

            // parse_if: parse if expression as statement
            TokenKind::If => {
                let expression = *self.parse_if()?;

                self.has_semicolon = optional_peek!(self, TokenKind::Semicolon);

                Ok(Statement::Expression(ExpressionStatement { expression }))
            },

            // matches `<ident> :`
            TokenKind::Ident { sym } if let TokenKind::Colon = self.peek_token.kind => {
                let name = Identifier { value: sym };
                self.next_token()?; // curr_token is now `:`

                let explicit_ty = match self.peek_token.kind {
                    // matches `=`; meaning there is no explicit type
                    TokenKind::Assign {
                        kind: AssignmentKind::Assign,
                    } => {
                        self.next_token()?; // curr_token is now `=`
                        None
                    },
                    // matches `<typename>`
                    TokenKind::Ident { sym } => {
                        let ty = match sym {
                            syms::INT => Type::Integer,
                            syms::FLOAT => Type::Float,
                            syms::STRING => Type::String,
                            _ => Type::None,
                        };
                        self.next_token()?; // curr_token is now `<typename>`
                        if let TokenKind::Assign {
                            kind: AssignmentKind::Assign,
                        } = self.peek_token.kind
                        {
                            self.next_token()?; // curr_token is now `=`
                        };
                        Some(ty)
                    },
                    // matches an unexpected token
                    _ => {
                        self.next_token()?; // curr_token is now the unexpected token
                        return Err(self.error_unexpected_token());
                    },
                };

                let value = if let TokenKind::Assign {
                    kind: AssignmentKind::Assign,
                } = self.curr_token.kind
                {
                    self.next_token()?; // curr_token is start of RHS value
                    let value = self.parse_expression(Precedence::Lowest)?;
                    Some(value)
                } else {
                    None
                };

                self.has_semicolon = optional_peek!(self, TokenKind::Semicolon);

                Ok(*self.ast.alloc(Statement::VarDecl(VarDeclStatement {
                    name,
                    value,
                    explicit_ty,
                })))
            },

            _ => {
                let stmt = ExpressionStatement {
                    expression: *self.parse_expression(Precedence::Lowest)?,
                };

                self.has_semicolon = optional_peek!(self, TokenKind::Semicolon);

                Ok(Statement::Expression(stmt))
            },
        }
    }

    fn parse_expression(&mut self, precedence: Precedence) -> Result<&'ast Expression<'ast>, ParserError> {
        let mut left_expr = self.parse_prefix()?;

        while precedence < Precedence::from(&self.peek_token.kind) {
            match self.parse_infix(left_expr)? {
                Some(expr) => left_expr = expr,
                None => return Ok(left_expr),
            };
        }

        Ok(left_expr)
    }

    fn parse_block(&mut self) -> Result<BlockExpression<'ast>, ParserError> {
        let mut statements = Vec::new();

        self.next_token()?;

        self.depth += 1;
        while !matches!(self.curr_token.kind, TokenKind::RightBrace | TokenKind::EOF) {
            statements.push(self.parse_statement()?);
            self.next_token()?;
        }
        self.depth -= 1;

        Ok(BlockExpression {
            statements: self.ast.alloc_slice_clone(&statements),
        })
    }

    fn parse_if(&mut self) -> Result<&'ast Expression<'ast>, ParserError> {
        self.next_token()?;
        let condition = self.parse_expression(Precedence::Lowest)?;

        expect_peek!(self, TokenKind::LeftBrace);

        let consequence = self.parse_block()?;

        let alternative: Option<&'ast Expression<'ast>> = if matches!(self.peek_token.kind, TokenKind::Else) {
            self.next_token()?;
            self.next_token()?;

            Some(match self.curr_token.kind {
                TokenKind::If => self.parse_if()?,
                TokenKind::LeftBrace => self.ast.alloc(Expression::Block(self.parse_block()?)),
                _ => return Err(self.error_unexpected_token()),
            })
        } else {
            None
        };

        Ok(self.ast.alloc(Expression::If(IfExpression {
            condition,
            consequence,
            alternative,
        })))
    }

    fn parse_infix(&mut self, left: &'ast Expression<'ast>) -> Result<Option<&'ast Expression<'ast>>, ParserError> {
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

                Ok(Some(self.ast.alloc(Expression::Infix(InfixExpression {
                    left,
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
                    right,
                }))))
            },

            // parse_call: parse call expression
            TokenKind::LeftParen => {
                self.next_token()?;
                self.next_token()?;

                let mut args = Vec::new();

                if !matches!(self.curr_token.kind, TokenKind::RightParen) {
                    loop {
                        args.push(*self.parse_expression(Precedence::Lowest)?);

                        if !matches!(self.peek_token.kind, TokenKind::Comma) {
                            break;
                        }

                        self.next_token()?;
                        self.next_token()?;
                    }

                    expect_peek!(self, TokenKind::RightParen);
                }

                Ok(Some(self.ast.alloc(Expression::Call(CallExpression {
                    function: left,
                    args: self.ast.alloc_slice_clone(&args),
                }))))
            },

            TokenKind::LeftBracket => {
                self.next_token()?;
                self.next_token()?;

                let index = self.parse_expression(Precedence::Lowest)?;

                expect_peek!(self, TokenKind::RightBracket);

                Ok(Some(self.ast.alloc(Expression::Index(IndexExpression { left, index }))))
            },

            TokenKind::Assign { ref kind } => {
                let kind = *kind;
                if !matches!(left, Expression::Identifier(_)) {
                    return Err(self.error_invalid_lhs(left));
                }

                let TokenKind::Ident { sym } = self.curr_token.kind else {
                    todo!()
                };

                let name = Identifier { value: sym };

                self.next_token()?;

                self.next_token()?;
                let value = self.parse_expression(Precedence::Lowest)?;

                Ok(Some(self.ast.alloc(Expression::Var(VarExpression {
                    kind,
                    name,
                    value,
                }))))
            },

            _ => Ok(None),
        }
    }

    fn parse_prefix(&mut self) -> Result<&'ast Expression<'ast>, ParserError> {
        match self.curr_token.kind {
            // parse_identifier: parse current token as identifier
            TokenKind::Ident { sym } => Ok(self.ast.alloc(Expression::Identifier(Identifier { value: sym }))),

            TokenKind::Literal { ref kind, sym } => {
                let str_value = self.session.lookup_string(sym);
                match kind {
                    LiteralKind::Integer => match str_value.parse::<i64>() {
                        Ok(lit) => Ok(self.ast.alloc(Expression::Integer(IntegerLiteral { value: lit }))),
                        Err(_) => Err(self.error_parsing_integer(str_value)),
                    },
                    LiteralKind::Float => match str_value.parse::<f64>() {
                        Ok(lit) => Ok(self.ast.alloc(Expression::Float(FloatLiteral { value: lit }))),
                        Err(_) => Err(self.error_parsing_float(str_value)),
                    },
                    LiteralKind::String => Ok(self.ast.alloc(Expression::String(StringLiteral { value: sym }))),
                }
            },

            TokenKind::KwTrue => Ok(self.ast.alloc(Expression::Boolean(BooleanExpression { value: true }))),

            TokenKind::KwFalse => Ok(self.ast.alloc(Expression::Boolean(BooleanExpression { value: false }))),

            // parse_array
            TokenKind::LeftBracket => {
                self.next_token()?;

                let mut elements = Vec::new();

                if !matches!(self.curr_token.kind, TokenKind::RightBracket) {
                    loop {
                        elements.push(*self.parse_expression(Precedence::Lowest)?);

                        if !matches!(self.peek_token.kind, TokenKind::Comma) {
                            break;
                        }

                        self.next_token()?;
                        self.next_token()?;
                    }

                    expect_peek!(self, TokenKind::RightBracket);
                }

                Ok(self.ast.alloc(Expression::Array(ArrayLiteral {
                    elements: self.ast.alloc_slice_clone(&elements),
                })))
            },

            // parse_prefix: parse current expression with prefix
            TokenKind::Not | TokenKind::Sub => {
                let prev_token = self.curr_token.clone();

                self.next_token()?;

                let right = self.parse_expression(Precedence::Prefix).unwrap();

                Ok(self.ast.alloc(Expression::Prefix(PrefixExpression {
                    operator: match prev_token.kind {
                        TokenKind::Not => PrefixKind::Not,
                        TokenKind::Sub => PrefixKind::Sub,
                        _ => unreachable!(),
                    },
                    right,
                })))
            },

            // parse_grouped: parse grouped expression
            TokenKind::LeftParen => {
                self.next_token()?;
                let expr = self.parse_expression(Precedence::Lowest)?;

                expect_peek!(self, TokenKind::RightParen);

                Ok(expr)
            },

            // parse_block
            TokenKind::LeftBrace => {
                let block = self.parse_block()?;
                Ok(self.ast.alloc(Expression::Block(block)))
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

                Ok(self.ast.alloc(Expression::Function(FunctionLiteral {
                    params: self.ast.alloc_slice_clone(&params),
                    body,
                })))
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

    fn error_invalid_lhs(&self, _left: &Expression) -> ParserError {
        // TODO: change this with expression span
        let label = Label::primary(self.curr_token.span, "invalid lhs");
        self.session.emit(Diagnostic::error("invalid lhs").with_label(label));
        ParserError::InvalidLHS
    }

    fn error_parsing_integer(&self, v: &str) -> ParserError {
        let label = Label::primary(self.curr_token.span, "error parsing integer");
        self.session
            .emit(Diagnostic::error("error parsing integer").with_label(label));
        ParserError::ParsingInteger(v.to_string())
    }

    fn error_parsing_float(&self, v: &str) -> ParserError {
        let label = Label::primary(self.curr_token.span, "error parsing float");
        self.session
            .emit(Diagnostic::error("error parsing float").with_label(label));
        ParserError::ParsingFloat(v.to_string())
    }
}

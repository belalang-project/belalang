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
use session::Session;
use span::SourceSpan;

use super::{
    Expression,
    ExpressionKind,
    ParserError,
    Statement,
    StatementKind,
};
use crate::{
    ArrayLiteral,
    Ast,
    BlockExpression,
    BooleanExpression,
    BreakStatement,
    CallExpression,
    ContinueStatement,
    ExpressionStatement,
    FloatLiteral,
    FunctionLiteral,
    Identifier,
    IfExpression,
    IndexExpression,
    InfixExpression,
    IntegerLiteral,
    MemberAccessExpression,
    PrefixExpression,
    Program,
    ReturnStatement,
    StringLiteral,
    StructDeclStatement,
    StructLiteral,
    VarDeclStatement,
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
            TokenKind::LeftParen | TokenKind::LeftBrace => Self::Call,
            TokenKind::LeftBracket | TokenKind::Dot => Self::Index,
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

bitflags::bitflags! {
    /// Context applied when parsing.
    ///
    /// This is inspired by rustc_parse's Restrictions. This essentially tracks
    /// what is being parsed and allow the parser to change behaviour based on
    /// it.
    #[derive(Debug, Clone, Copy, PartialEq, Eq)]
    pub struct Restrictions: u8 {
        const NO_STRUCT_LITERAL = 1 << 0;
    }
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

    restrictions: Restrictions,
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
            restrictions: Restrictions::empty(),
        }
    }

    fn with_restr<T, F>(&mut self, restrictions: Restrictions, f: F) -> Result<T, ParserError>
    where
        F: FnOnce(&mut Self) -> Result<T, ParserError>,
    {
        let old = self.restrictions;
        self.restrictions = restrictions;
        let res = f(self);
        self.restrictions = old;
        res
    }

    fn with_no_restr<T, F>(&mut self, f: F) -> Result<T, ParserError>
    where
        F: FnOnce(&mut Self) -> Result<T, ParserError>,
    {
        self.with_restr(Restrictions::empty(), f)
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
        let start_span = self.curr_token.span;
        match self.curr_token.kind {
            // matches `return`
            TokenKind::Return => {
                // TODO: more concrete statement terminator. see #208
                let return_value = if matches!(
                    self.peek_token.kind,
                    TokenKind::Semicolon | TokenKind::RightBrace | TokenKind::EOF
                ) {
                    None
                } else {
                    self.next_token()?; // curr_token is now the return value
                    Some(*self.parse_expression(Precedence::Lowest)?)
                };

                self.has_semicolon = optional_peek!(self, TokenKind::Semicolon);
                let span = SourceSpan::new(start_span.start, self.curr_token.span.end);

                Ok(Statement {
                    kind: StatementKind::Return(ReturnStatement { return_value }),
                    span,
                })
            },

            // parse_while
            TokenKind::While => {
                self.next_token()?;
                let condition = self.with_restr(Restrictions::NO_STRUCT_LITERAL, |p| {
                    p.parse_expression(Precedence::Lowest)
                })?;
                let condition = *condition;

                expect_peek!(self, TokenKind::LeftBrace);

                let block = self.parse_block()?;

                self.has_semicolon = optional_peek!(self, TokenKind::Semicolon);
                let span = SourceSpan::new(start_span.start, self.curr_token.span.end);

                Ok(Statement {
                    kind: StatementKind::While(WhileStatement { condition, block }),
                    span,
                })
            },

            // match `break`
            TokenKind::KwBreak => Ok(Statement {
                kind: StatementKind::Break(BreakStatement),
                span: start_span,
            }),

            // match `continue`
            TokenKind::KwContinue => Ok(Statement {
                kind: StatementKind::Continue(ContinueStatement),
                span: start_span,
            }),

            // parse_if: parse if expression as statement
            TokenKind::If => {
                let expression = *self.parse_if()?;

                self.has_semicolon = optional_peek!(self, TokenKind::Semicolon);
                let span = SourceSpan::new(start_span.start, self.curr_token.span.end);

                Ok(Statement {
                    kind: StatementKind::Expression(ExpressionStatement { expression }),
                    span,
                })
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
                        self.next_token()?; // curr_token is now `<typename>`
                        if let TokenKind::Assign {
                            kind: AssignmentKind::Assign,
                        } = self.peek_token.kind
                        {
                            self.next_token()?; // curr_token is now `=`
                        };
                        Some(sym)
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
                let span = SourceSpan::new(start_span.start, self.curr_token.span.end);

                Ok(*self.ast.alloc(Statement {
                    kind: StatementKind::VarDecl(VarDeclStatement {
                        name,
                        value,
                        explicit_ty,
                    }),
                    span,
                }))
            },

            // matches `struct`
            TokenKind::Struct => {
                self.next_token(); // curr_token should now be ident
                let TokenKind::Ident { sym } = self.curr_token.kind else {
                    return Err(self.error_unexpected_token());
                };
                let struct_name = Identifier { value: sym };

                expect_peek!(self, TokenKind::LeftBrace); // curr_token should now be `{`

                let mut fields = Vec::new();

                self.next_token()?; // curr_token now at first statement

                self.depth += 1;
                while !matches!(self.curr_token.kind, TokenKind::RightBrace | TokenKind::EOF) {
                    let stmt = self.parse_statement()?;
                    let StatementKind::VarDecl(var_decl) = stmt.kind else {
                        return Err(self.error_parsing_struct(stmt));
                    };
                    fields.push(var_decl);
                    self.next_token()?; // curr_token now at next statement
                } // curr_token now at end of block
                self.depth -= 1;
                let span = SourceSpan::new(start_span.start, self.curr_token.span.end);

                Ok(*self.ast.alloc(Statement {
                    kind: StatementKind::StructDecl(StructDeclStatement {
                        name: struct_name,
                        fields: self.ast.alloc_slice_clone(&fields),
                    }),
                    span,
                }))
            },

            _ => {
                let expr = self.parse_expression(Precedence::Lowest)?;
                let stmt = ExpressionStatement { expression: *expr };

                self.has_semicolon = optional_peek!(self, TokenKind::Semicolon);
                let span = SourceSpan::new(start_span.start, self.curr_token.span.end);

                Ok(Statement {
                    kind: StatementKind::Expression(stmt),
                    span,
                })
            },
        }
    }

    fn parse_expression(&mut self, precedence: Precedence) -> Result<&'ast Expression<'ast>, ParserError> {
        let mut left_expr = self.parse_prefix()?;

        while precedence < Precedence::from(&self.peek_token.kind) {
            if self.peek_token.kind == TokenKind::LeftBrace
                && self.restrictions.contains(Restrictions::NO_STRUCT_LITERAL)
            {
                break;
            }
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
        let start_span = self.curr_token.span;
        self.next_token()?;

        let condition = self.with_restr(Restrictions::NO_STRUCT_LITERAL, |p| {
            p.parse_expression(Precedence::Lowest)
        })?;

        expect_peek!(self, TokenKind::LeftBrace);

        let consequence = self.parse_block()?;
        let mut end_span = self.curr_token.span;

        let alternative: Option<&'ast Expression<'ast>> = if matches!(self.peek_token.kind, TokenKind::Else) {
            self.next_token()?;
            self.next_token()?;

            let alt = match self.curr_token.kind {
                TokenKind::If => self.parse_if()?,
                TokenKind::LeftBrace => {
                    let block_start = self.curr_token.span;
                    let block = self.parse_block()?;
                    let block_span = SourceSpan::new(block_start.start, self.curr_token.span.end);
                    self.ast.alloc(Expression {
                        kind: ExpressionKind::Block(block),
                        span: block_span,
                    })
                },
                _ => return Err(self.error_unexpected_token()),
            };
            end_span = alt.span;
            Some(alt)
        } else {
            None
        };

        let span = SourceSpan::new(start_span.start, end_span.end);

        Ok(self.ast.alloc(Expression {
            kind: ExpressionKind::If(IfExpression {
                condition,
                consequence,
                alternative,
            }),
            span,
        }))
    }

    fn parse_infix(&mut self, left: &'ast Expression<'ast>) -> Result<Option<&'ast Expression<'ast>>, ParserError> {
        let start_span = left.span;
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
                let span = SourceSpan::new(start_span.start, right.span.end);

                Ok(Some(self.ast.alloc(Expression {
                    kind: ExpressionKind::Infix(InfixExpression {
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
                    }),
                    span,
                })))
            },

            // parse_call: parse call expression
            TokenKind::LeftParen => {
                self.next_token()?;
                self.next_token()?;

                let mut args = Vec::new();

                if !matches!(self.curr_token.kind, TokenKind::RightParen) {
                    loop {
                        let expr = self.with_no_restr(|p| p.parse_expression(Precedence::Lowest))?;
                        args.push(*expr);

                        if !matches!(self.peek_token.kind, TokenKind::Comma) {
                            break;
                        }

                        self.next_token()?;
                        self.next_token()?;
                    }

                    expect_peek!(self, TokenKind::RightParen);
                }
                let span = SourceSpan::new(start_span.start, self.curr_token.span.end);

                Ok(Some(self.ast.alloc(Expression {
                    kind: ExpressionKind::Call(CallExpression {
                        function: left,
                        args: self.ast.alloc_slice_clone(&args),
                    }),
                    span,
                })))
            },

            TokenKind::LeftBracket => {
                self.next_token()?;
                self.next_token()?;

                let index = self.with_no_restr(|p| p.parse_expression(Precedence::Lowest))?;

                expect_peek!(self, TokenKind::RightBracket);
                let span = SourceSpan::new(start_span.start, self.curr_token.span.end);

                Ok(Some(self.ast.alloc(Expression {
                    kind: ExpressionKind::Index(IndexExpression { left, index }),
                    span,
                })))
            },

            // matches `.`
            TokenKind::Dot => {
                self.next_token()?; // curr_token is now `.`
                self.next_token()?; // curr_token is not the property expr

                // expect the property identifier
                let TokenKind::Ident { sym } = self.curr_token.kind else {
                    return Err(self.error_unexpected_token());
                };

                let member = Identifier { value: sym };
                let span = SourceSpan::new(start_span.start, self.curr_token.span.end);

                // make member access!
                Ok(Some(self.ast.alloc(Expression {
                    kind: ExpressionKind::MemberAccess(MemberAccessExpression { source: left, member }),
                    span,
                })))
            },

            // match `{`
            TokenKind::LeftBrace => {
                self.next_token()?; // curr_token is now `{`
                self.next_token()?; // curr_token is now first field or `}`

                let mut fields = Vec::new();
                if !matches!(self.curr_token.kind, TokenKind::RightBrace) {
                    loop {
                        let expr = self.with_no_restr(|p| p.parse_expression(Precedence::Lowest))?;
                        let ExpressionKind::Var(var_expr) = expr.kind else {
                            return Err(self.error_invalid_struct_field(expr));
                        };
                        if !matches!(var_expr.kind, AssignmentKind::Assign) {
                            return Err(self.error_invalid_struct_field(expr));
                        }
                        fields.push(var_expr);

                        match self.peek_token.kind {
                            TokenKind::RightBrace => break,
                            TokenKind::Comma => {
                                self.next_token()?; // curr_token is now `,`
                                self.next_token()?; // curr_token is now next field
                            },
                            _ => return Err(self.error_unexpected_token()),
                        }
                    }
                    expect_peek!(self, TokenKind::RightBrace);
                }
                let span = SourceSpan::new(start_span.start, self.curr_token.span.end);

                let ExpressionKind::Identifier(struct_name) = left.kind else {
                    return Err(self.error_invalid_struct_name(left));
                };

                Ok(Some(self.ast.alloc(Expression {
                    kind: ExpressionKind::StructLiteral(StructLiteral {
                        name: struct_name,
                        fields: self.ast.alloc_slice_clone(&fields),
                    }),
                    span,
                })))
            },

            TokenKind::Assign { ref kind } => {
                let kind = *kind;
                if !matches!(left.kind, ExpressionKind::Identifier(_)) {
                    return Err(self.error_invalid_lhs(left));
                }

                let TokenKind::Ident { sym } = self.curr_token.kind else {
                    todo!()
                };

                let name = Identifier { value: sym };

                self.next_token()?;

                self.next_token()?;
                let value = self.parse_expression(Precedence::Lowest)?;
                let span = SourceSpan::new(start_span.start, value.span.end);

                Ok(Some(self.ast.alloc(Expression {
                    kind: ExpressionKind::Var(VarExpression { kind, name, value }),
                    span,
                })))
            },

            _ => Ok(None),
        }
    }

    fn parse_prefix(&mut self) -> Result<&'ast Expression<'ast>, ParserError> {
        let start_span = self.curr_token.span;
        match self.curr_token.kind {
            // parse_identifier: parse current token as identifier
            TokenKind::Ident { sym } => Ok(self.ast.alloc(Expression {
                kind: ExpressionKind::Identifier(Identifier { value: sym }),
                span: start_span,
            })),

            TokenKind::Literal { ref kind, sym } => {
                let str_value = self.session.lookup_string(sym);
                match kind {
                    LiteralKind::Integer => match str_value.parse::<i64>() {
                        Ok(lit) => Ok(self.ast.alloc(Expression {
                            kind: ExpressionKind::Integer(IntegerLiteral { value: lit }),
                            span: start_span,
                        })),
                        Err(_) => Err(self.error_parsing_integer(str_value)),
                    },
                    LiteralKind::Float => match str_value.parse::<f64>() {
                        Ok(lit) => Ok(self.ast.alloc(Expression {
                            kind: ExpressionKind::Float(FloatLiteral { value: lit }),
                            span: start_span,
                        })),
                        Err(_) => Err(self.error_parsing_float(str_value)),
                    },
                    LiteralKind::String => Ok(self.ast.alloc(Expression {
                        kind: ExpressionKind::String(StringLiteral { value: sym }),
                        span: start_span,
                    })),
                }
            },

            TokenKind::KwTrue => Ok(self.ast.alloc(Expression {
                kind: ExpressionKind::Boolean(BooleanExpression { value: true }),
                span: start_span,
            })),

            TokenKind::KwFalse => Ok(self.ast.alloc(Expression {
                kind: ExpressionKind::Boolean(BooleanExpression { value: false }),
                span: start_span,
            })),

            // parse_array
            TokenKind::LeftBracket => {
                self.next_token()?;

                let mut elements = Vec::new();

                if !matches!(self.curr_token.kind, TokenKind::RightBracket) {
                    loop {
                        let expr = self.with_no_restr(|p| p.parse_expression(Precedence::Lowest))?;
                        elements.push(*expr);

                        if !matches!(self.peek_token.kind, TokenKind::Comma) {
                            break;
                        }

                        self.next_token()?;
                        self.next_token()?;
                    }

                    expect_peek!(self, TokenKind::RightBracket);
                }
                let span = SourceSpan::new(start_span.start, self.curr_token.span.end);

                Ok(self.ast.alloc(Expression {
                    kind: ExpressionKind::Array(ArrayLiteral {
                        elements: self.ast.alloc_slice_clone(&elements),
                    }),
                    span,
                }))
            },

            // parse_prefix: parse current expression with prefix
            TokenKind::Not | TokenKind::Sub => {
                let prev_token = self.curr_token.clone();

                self.next_token()?;

                let right = self.parse_expression(Precedence::Prefix).unwrap();
                let span = SourceSpan::new(start_span.start, right.span.end);

                Ok(self.ast.alloc(Expression {
                    kind: ExpressionKind::Prefix(PrefixExpression {
                        operator: match prev_token.kind {
                            TokenKind::Not => PrefixKind::Not,
                            TokenKind::Sub => PrefixKind::Sub,
                            _ => unreachable!(),
                        },
                        right,
                    }),
                    span,
                }))
            },

            // parse_grouped: parse grouped expression
            TokenKind::LeftParen => {
                self.next_token()?;
                let expr = self.with_no_restr(|p| p.parse_expression(Precedence::Lowest))?;

                expect_peek!(self, TokenKind::RightParen);

                Ok(expr)
            },

            // parse_block
            TokenKind::LeftBrace => {
                let block = self.parse_block()?;
                let span = SourceSpan::new(start_span.start, self.curr_token.span.end);
                Ok(self.ast.alloc(Expression {
                    kind: ExpressionKind::Block(block),
                    span,
                }))
            },

            // parse_if: parse current if expression
            TokenKind::If => self.parse_if(),

            // match `fn`
            TokenKind::Function => {
                let mut params = Vec::new();

                expect_peek!(self, TokenKind::LeftParen); // curr_token is now `(`

                self.next_token()?; // curr_token is now first argument

                if !matches!(self.curr_token.kind, TokenKind::RightParen) {
                    loop {
                        let TokenKind::Ident { sym: name } = self.curr_token.kind else {
                            return Err(self.error_unexpected_token());
                        };

                        expect_peek!(self, TokenKind::Colon); // curr_token is now `:`

                        expect_peek!(self, TokenKind::Ident { .. }); // curr_token is now `<typename>`
                        let TokenKind::Ident { sym: ty } = self.curr_token.kind else {
                            unreachable!()
                        };

                        let name = Identifier { value: name };
                        let explicit_ty = Some(ty);

                        params.push(VarDeclStatement {
                            name,
                            value: None,
                            explicit_ty,
                        });

                        match self.peek_token.kind {
                            TokenKind::RightParen => break,
                            TokenKind::Comma => {
                                self.next_token(); // curr_token is now comma
                                self.next_token(); // curr_token is now next argument
                            },
                            _ => return Err(self.error_unexpected_token()),
                        }
                    }

                    expect_peek!(self, TokenKind::RightParen);
                } // curr_token is now `)`

                let explicit_ty = if let TokenKind::Colon = self.peek_token.kind {
                    self.next_token(); // curr_token is now `:`
                    expect_peek!(self, TokenKind::Ident { .. }); // curr_token is now `<typename>`

                    let TokenKind::Ident { sym } = self.curr_token.kind else {
                        unreachable!()
                    };
                    Some(sym)
                } else {
                    None
                };

                expect_peek!(self, TokenKind::LeftBrace); // curr_token is now `{`

                let body = self.parse_block()?;
                let span = SourceSpan::new(start_span.start, self.curr_token.span.end);

                Ok(self.ast.alloc(Expression {
                    kind: ExpressionKind::Function(FunctionLiteral {
                        params: self.ast.alloc_slice_clone(&params),
                        body,
                        explicit_ty,
                    }),
                    span,
                }))
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
        let label = Label::primary(left.span, "invalid lhs");
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

    fn error_parsing_struct(&self, stmt: Statement) -> ParserError {
        let label = Label::primary(stmt.span, "invalid statement");
        self.session
            .emit(Diagnostic::error("error parsing struct").with_label(label));
        ParserError::ParsingStruct
    }

    fn error_invalid_struct_name(&self, left: &Expression) -> ParserError {
        let label = Label::primary(left.span, "invalid struct name");
        self.session
            .emit(Diagnostic::error("error parsing struct").with_label(label));
        ParserError::ParsingStruct
    }

    fn error_invalid_struct_field(&self, expr: &Expression) -> ParserError {
        let label = Label::primary(expr.span, "expected field assignment");
        self.session
            .emit(Diagnostic::error("error parsing struct").with_label(label));
        ParserError::ParsingStruct
    }
}

use std::collections::HashMap;

use ast::Visitor;
use diag::{
    Diagnostic,
    Label,
};
use session::{
    Session,
    interner::{
        Symbol,
        syms,
    },
};

#[derive(Clone, Copy, Debug, PartialEq)]
pub enum Type {
    String,
    Integer,
    Float,
    Boolean,
    None,
}

pub struct TypeChecker<'sess> {
    #[allow(dead_code)]
    session: &'sess Session,
    env: HashMap<Symbol, Type>,
    current_type: Type,
}

impl<'sess> TypeChecker<'sess> {
    pub fn new(session: &'sess Session) -> TypeChecker<'sess> {
        TypeChecker {
            session,
            env: HashMap::new(),
            current_type: Type::None,
        }
    }

    pub fn infer<'ast>(&mut self, program: &ast::Program<'ast>) {
        self.visit_program(program);
    }

    pub fn infer_expr<'ast>(&mut self, expr: &ast::Expression<'ast>) -> Type {
        match expr.kind {
            ast::ExpressionKind::Integer(_) => Type::Integer,
            ast::ExpressionKind::Float(_) => Type::Float,
            ast::ExpressionKind::String(_) => Type::String,
            ast::ExpressionKind::Boolean(_) => Type::Boolean,
            ast::ExpressionKind::Identifier(i) => *self.env.get(&i.value).unwrap_or(&Type::None),
            ast::ExpressionKind::Infix(ref infix) => self.infer_infix(infix),
            _ => {
                self.walk_expression(expr);
                self.current_type
            },
        }
    }

    pub fn check_expr<'ast>(&mut self, expr: &ast::Expression<'ast>, expected: Type) {
        let inferred = self.infer_expr(expr);
        if inferred != expected {
            let label = Label::primary(
                expr.span,
                format!(
                    "expected type `{}`, found type `{}`",
                    ty_to_str(expected),
                    ty_to_str(inferred),
                ),
            );
            self.session
                .emit(Diagnostic::error("mismatched type").with_label(label))
        }
    }

    pub fn infer_infix<'ast>(&mut self, infix: &ast::InfixExpression<'ast>) -> Type {
        let left_ty = self.infer_expr(infix.left);
        let right_ty = self.infer_expr(infix.right);

        if left_ty != right_ty {
            let label = Label::primary(
                infix.right.span,
                format!(
                    "expected type `{}`, found type `{}`",
                    ty_to_str(left_ty),
                    ty_to_str(right_ty),
                ),
            );
            self.session
                .emit(Diagnostic::error("mismatched type").with_label(label));
        }

        match infix.operator {
            lexer::InfixKind::Eq
            | lexer::InfixKind::Ne
            | lexer::InfixKind::Lt
            | lexer::InfixKind::Le
            | lexer::InfixKind::Gt
            | lexer::InfixKind::Ge => Type::Boolean,

            lexer::InfixKind::And | lexer::InfixKind::Or => {
                if left_ty != Type::Boolean {
                    let label = Label::primary(
                        infix.left.span,
                        format!("expected type `Boolean`, found type `{}`", ty_to_str(left_ty)),
                    );
                    self.session
                        .emit(Diagnostic::error("mismatched type").with_label(label));
                }
                Type::Boolean
            },

            _ => left_ty,
        }
    }
}

impl<'ast, 'sess> Visitor<'ast> for TypeChecker<'sess> {
    fn visit_expression(&mut self, expr: &ast::Expression<'ast>) {
        self.current_type = self.infer_expr(expr);
    }

    fn visit_var_decl_statement(&mut self, node: &ast::VarDeclStatement<'ast>) {
        let explicit_ty = node.explicit_ty.map(|sym| sym_to_ty(sym));

        let rhs_ty = if let Some(resolved_ty) = explicit_ty {
            if let Some(value) = node.value {
                self.check_expr(value, resolved_ty);
            }
            resolved_ty
        } else {
            if let Some(value) = node.value {
                self.infer_expr(value)
            } else {
                Type::None
            }
        };

        self.env.insert(node.name.value, rhs_ty);
        self.current_type = rhs_ty;
    }
}

fn sym_to_ty(symbol: Symbol) -> Type {
    match symbol {
        syms::STRING => Type::String,
        syms::INT => Type::Integer,
        syms::FLOAT => Type::Float,
        syms::BOOL => Type::Boolean,
        _ => Type::None,
    }
}

fn ty_to_str(ty: Type) -> String {
    String::from(match ty {
        Type::String => "String",
        Type::Integer => "Integer",
        Type::Float => "Float",
        Type::Boolean => "Boolean",
        Type::None => "None",
    })
}

#[cfg(test)]
mod tests {
    use ast::{
        Expression,
        ExpressionKind,
        Identifier,
        IntegerLiteral,
        StringLiteral,
        VarDeclStatement,
        Visitor,
    };
    use session::{
        Session,
        interner::{
            Symbol,
            syms,
        },
    };

    use super::{
        Type,
        TypeChecker,
    };

    #[test]
    fn test_implicit_string() {
        let str_expr = Expression {
            kind: ExpressionKind::String(StringLiteral { value: Symbol(1) }),
            span: Default::default(),
        };
        let expr = VarDeclStatement {
            name: Identifier { value: Symbol(0) },
            explicit_ty: None,
            value: Some(&str_expr),
        };

        let session = Session::for_text("".to_string()).unwrap();
        let mut ty_infer = TypeChecker::new(&session);
        ty_infer.visit_var_decl_statement(&expr);

        assert_eq!(*ty_infer.env.get(&Symbol(0)).unwrap(), Type::String);
        assert_eq!(ty_infer.env.len(), 1);
    }

    #[test]
    fn test_explicit_int() {
        let int_expr = Expression {
            kind: ExpressionKind::Integer(IntegerLiteral { value: 12 }),
            span: Default::default(),
        };
        let expr = VarDeclStatement {
            name: Identifier { value: Symbol(0) },
            explicit_ty: Some(syms::INT),
            value: Some(&int_expr),
        };

        let session = Session::for_text("".to_string()).unwrap();
        let mut ty_infer = TypeChecker::new(&session);
        ty_infer.visit_var_decl_statement(&expr);

        assert_eq!(*ty_infer.env.get(&Symbol(0)).unwrap(), Type::Integer);
        assert_eq!(ty_infer.env.len(), 1);
    }
}

use std::collections::HashMap;

use ast::Visitor;
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
}

impl<'ast, 'sess> Visitor<'ast> for TypeChecker<'sess> {
    fn visit_integer(&mut self, _node: &ast::IntegerLiteral) {
        self.current_type = Type::Integer;
    }

    fn visit_string(&mut self, _node: &ast::StringLiteral) {
        self.current_type = Type::String;
    }

    fn visit_float(&mut self, _node: &ast::FloatLiteral) {
        self.current_type = Type::Float;
    }

    fn visit_identifier(&mut self, node: &ast::Identifier) {
        if let Some(ty) = self.env.get(&node.value) {
            self.current_type = *ty;
        } else {
            // TODO: handle unknown variables
        }
    }

    fn visit_var_decl_statement(&mut self, node: &ast::VarDeclStatement<'ast>) {
        let rhs_ty = node.explicit_ty.map(sym_to_ty).unwrap_or_else(|| {
            if let Some(value) = node.value {
                self.visit_expression(value);
            }
            self.current_type
        });

        self.env.insert(node.name.value, rhs_ty);
        self.current_type = rhs_ty;
    }
}

fn sym_to_ty(symbol: Symbol) -> Type {
    match symbol {
        syms::STRING => Type::String,
        syms::INT => Type::Integer,
        syms::FLOAT => Type::Float,
        _ => Type::None,
    }
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

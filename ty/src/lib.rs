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

pub struct TypeInferer<'sess> {
    #[allow(dead_code)]
    session: &'sess Session,
    inner: TypeInfererInner,
}

impl<'sess> TypeInferer<'sess> {
    pub fn new(session: &'sess Session) -> TypeInferer<'sess> {
        TypeInferer {
            session,
            inner: TypeInfererInner::new(),
        }
    }

    pub fn infer<'ast>(&mut self, program: &ast::Program<'ast>) {
        self.inner.visit_program(program);
    }
}

pub(crate) struct TypeInfererInner {
    env: HashMap<Symbol, Type>,
    current_type: Type,
}

impl TypeInfererInner {
    pub fn new() -> Self {
        Self {
            env: HashMap::new(),
            current_type: Type::None,
        }
    }
}

impl<'ast> Visitor<'ast> for TypeInfererInner {
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
        Identifier,
        IntegerLiteral,
        StringLiteral,
        VarDeclStatement,
        Visitor,
    };
    use session::interner::{
        Symbol,
        syms,
    };

    use super::{
        Type,
        TypeInfererInner,
    };

    #[test]
    fn test_implicit_string() {
        let str_expr = Expression::String(StringLiteral { value: Symbol(1) });
        let expr = VarDeclStatement {
            name: Identifier { value: Symbol(0) },
            explicit_ty: None,
            value: Some(&str_expr),
        };

        let mut ty_infer = TypeInfererInner::new();
        ty_infer.visit_var_decl_statement(&expr);

        assert_eq!(*ty_infer.env.get(&Symbol(0)).unwrap(), Type::String);
        assert_eq!(ty_infer.env.len(), 1);
    }

    #[test]
    fn test_explicit_int() {
        let int_expr = Expression::Integer(IntegerLiteral { value: 12 });
        let expr = VarDeclStatement {
            name: Identifier { value: Symbol(0) },
            explicit_ty: Some(syms::INT),
            value: Some(&int_expr),
        };

        let mut ty_infer = TypeInfererInner::new();
        ty_infer.visit_var_decl_statement(&expr);

        assert_eq!(*ty_infer.env.get(&Symbol(0)).unwrap(), Type::Integer);
        assert_eq!(ty_infer.env.len(), 1);
    }
}

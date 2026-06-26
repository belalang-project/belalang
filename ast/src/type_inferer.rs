use std::collections::HashMap;

use session::{
    Session,
    interner::Symbol,
};

use super::Visitor;

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

    pub fn infer(&mut self, program: &crate::Program) {
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

impl Visitor for TypeInfererInner {
    fn visit_integer(&mut self, _node: &crate::IntegerLiteral) {
        self.current_type = Type::Integer;
    }

    fn visit_string(&mut self, _node: &crate::StringLiteral) {
        self.current_type = Type::String;
    }

    fn visit_float(&mut self, _node: &crate::FloatLiteral) {
        self.current_type = Type::Float;
    }

    fn visit_identifier(&mut self, node: &crate::Identifier) {
        if let Some(ty) = self.env.get(&node.value) {
            self.current_type = *ty;
        } else {
            // TODO: handle unknown variables
        }
    }

    fn visit_var_decl(&mut self, node: &crate::VarDeclExpression) {
        let rhs_ty = node.explicit_ty.unwrap_or_else(|| {
            if let Some(value) = &node.value {
                self.visit_expression(value);
            }
            self.current_type
        });

        self.env.insert(node.name.value, rhs_ty);
        self.current_type = rhs_ty;
    }
}

#[cfg(test)]
mod tests {
    use session::interner::Symbol;

    use super::TypeInfererInner;
    use crate::{
        Expression,
        Identifier,
        IntegerLiteral,
        StringLiteral,
        VarDeclExpression,
        Visitor,
        type_inferer::Type,
    };

    #[test]
    fn test_implicit_string() {
        let expr = VarDeclExpression {
            name: Identifier { value: Symbol(0) },
            explicit_ty: None,
            value: Some(Box::new(Expression::String(StringLiteral { value: Symbol(1) }))),
        };

        let mut ty_infer = TypeInfererInner::new();
        ty_infer.visit_var_decl(&expr);

        assert_eq!(*ty_infer.env.get(&Symbol(0)).unwrap(), Type::String);
        assert_eq!(ty_infer.env.len(), 1);
    }

    #[test]
    fn test_explicit_int() {
        let expr = VarDeclExpression {
            name: Identifier { value: Symbol(0) },
            explicit_ty: Some(Type::Integer),
            value: Some(Box::new(Expression::Integer(IntegerLiteral { value: 12 }))),
        };

        let mut ty_infer = TypeInfererInner::new();
        ty_infer.visit_var_decl(&expr);

        assert_eq!(*ty_infer.env.get(&Symbol(0)).unwrap(), Type::Integer);
        assert_eq!(ty_infer.env.len(), 1);
    }
}

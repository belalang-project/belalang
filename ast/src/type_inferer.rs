use std::collections::HashMap;

use lexer::AssignmentKind;
use session::Session;

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
    env: HashMap<String, Type>,
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
            self.current_type = ty.clone();
        } else {
            // TODO: handle unknown variables
        }
    }

    fn visit_var(&mut self, node: &crate::VarExpression) {
        self.visit_expression(&node.value);
        let rhs_ty = self.current_type.clone();

        match node.kind {
            AssignmentKind::ColonAssign => {
                self.env.insert(node.name.value.clone(), rhs_ty);
            },
            _ => {
                // TODO: handle other kinds of assignment
            },
        }

        self.current_type = rhs_ty;
    }
}

#[cfg(test)]
mod tests {
    use lexer::AssignmentKind;

    use super::TypeInfererInner;
    use crate::{
        Expression,
        Identifier,
        StringLiteral,
        VarExpression,
        Visitor,
        type_inferer::Type,
    };

    #[test]
    fn test_infer_str() {
        let expr = VarExpression {
            kind: AssignmentKind::ColonAssign,
            name: Identifier { value: "x".to_string() },
            value: Box::new(Expression::String(StringLiteral {
                value: "Hello".to_string(),
            })),
        };

        let mut ty_infer = TypeInfererInner::new();
        ty_infer.visit_var(&expr);

        assert_eq!(*ty_infer.env.get("x").unwrap(), Type::String);
        assert_eq!(ty_infer.env.len(), 1);
    }
}

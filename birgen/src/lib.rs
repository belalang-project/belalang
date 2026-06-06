use belalang_ast::{
    Expression,
    InfixExpression,
    Program,
    Statement,
};
use belalang_lexer::InfixKind;
use bir::ffi;

pub struct BIRGen {
    builder: cxx::UniquePtr<ffi::BIRBuilder>,
}

impl BIRGen {
    pub fn new() -> Self {
        Self {
            builder: ffi::create_builder(),
        }
    }

    pub fn generate_program(&mut self, program: &Program) {
        for stmt in &program.statements {
            self.generate_statement(stmt);
        }
    }

    pub fn generate_statement(&mut self, stmt: &Statement) {
        match stmt {
            Statement::Expression(expr_stmt) => {
                self.generate_expression(&expr_stmt.expression);
            },
            Statement::Return(_ret_stmt) => {
                // TODO: Implement return
            },
            Statement::While(_while_stmt) => {
                // TODO: Implement while
            },
        }
    }

    pub fn generate_expression(&mut self, expr: &Expression) -> cxx::UniquePtr<ffi::BIRValue> {
        match expr {
            Expression::Integer(lit) => self.builder.pin_mut().build_constant_int(lit.value),
            Expression::Float(lit) => self.builder.pin_mut().build_constant_float(lit.value),
            Expression::Infix(infix) => self.generate_infix(infix),
            _ => todo!("Generation for expression {:?} not implemented", expr),
        }
    }

    fn generate_infix(&mut self, infix: &InfixExpression) -> cxx::UniquePtr<ffi::BIRValue> {
        let lhs = self.generate_expression(&infix.left);
        let rhs = self.generate_expression(&infix.right);

        match infix.operator {
            InfixKind::Add => self.builder.pin_mut().build_add(&lhs, &rhs),
            InfixKind::Sub => self.builder.pin_mut().build_sub(&lhs, &rhs),
            InfixKind::Mul => self.builder.pin_mut().build_mul(&lhs, &rhs),
            InfixKind::Div => self.builder.pin_mut().build_div(&lhs, &rhs),
            InfixKind::Mod => self.builder.pin_mut().build_mod(&lhs, &rhs),
            _ => todo!("Infix operator {:?} not implemented", infix.operator),
        }
    }

    pub fn dump(&self) {
        self.builder.dump();
    }

    pub fn dump_to_string(&self) -> String {
        self.builder.dump_to_string()
    }
}

impl Default for BIRGen {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
#[allow(clippy::approx_constant)]
mod tests {
    use belalang_ast::{
        Expression,
        FloatLiteral,
        InfixExpression,
        IntegerLiteral,
    };
    use belalang_lexer::InfixKind;

    use super::*;

    #[test]
    fn test_generate_infix() {
        let mut generator = BIRGen::new();
        let expr = Expression::Infix(InfixExpression {
            left: Box::new(Expression::Integer(IntegerLiteral { value: 10 })),
            operator: InfixKind::Add,
            right: Box::new(Expression::Integer(IntegerLiteral { value: 32 })),
        });

        generator.generate_expression(&expr);
        let ir = generator.dump_to_string();

        assert!(ir.contains("bir.constant 10 : !bir.int"));
        assert!(ir.contains("bir.constant 32 : !bir.int"));
        assert!(ir.contains("bir.add"));
    }

    #[test]
    fn test_generate_float() {
        let mut generator = BIRGen::new();
        let expr = Expression::Float(FloatLiteral { value: 3.14 });

        generator.generate_expression(&expr);
        let ir = generator.dump_to_string();

        assert!(ir.contains("bir.constant 3.140000e+00 : !bir.float"));
    }
}

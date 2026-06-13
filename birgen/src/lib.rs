use ast::{
    Expression,
    InfixExpression,
    Program,
    Statement,
};
use lexer::InfixKind;

#[cxx::bridge(namespace = "belalang::birgen")]
mod ffi {
    unsafe extern "C++" {
        include!("belalang/BIRGen/BIRGen.h");

        type BIRValue;
        type BIRGen;

        fn create_birgen() -> UniquePtr<BIRGen>;

        fn build_constant_int(self: Pin<&mut BIRGen>, val: i64) -> UniquePtr<BIRValue>;
        fn build_constant_float(self: Pin<&mut BIRGen>, val: f64) -> UniquePtr<BIRValue>;
        fn build_add(self: Pin<&mut BIRGen>, lhs: &BIRValue, rhs: &BIRValue) -> UniquePtr<BIRValue>;
        fn build_sub(self: Pin<&mut BIRGen>, lhs: &BIRValue, rhs: &BIRValue) -> UniquePtr<BIRValue>;
        fn build_mul(self: Pin<&mut BIRGen>, lhs: &BIRValue, rhs: &BIRValue) -> UniquePtr<BIRValue>;
        fn build_div(self: Pin<&mut BIRGen>, lhs: &BIRValue, rhs: &BIRValue) -> UniquePtr<BIRValue>;
        fn build_mod(self: Pin<&mut BIRGen>, lhs: &BIRValue, rhs: &BIRValue) -> UniquePtr<BIRValue>;
        fn build_print(self: Pin<&mut BIRGen>, val: &BIRValue);
        fn build_empty_return(self: Pin<&mut BIRGen>);
        fn optimize(self: Pin<&mut BIRGen>) -> bool;

        fn dump(self: &BIRGen);
        fn dump_to_string(self: &BIRGen) -> String;
    }
}

pub struct BIRGen {
    inner: cxx::UniquePtr<ffi::BIRGen>,
}

impl BIRGen {
    pub fn new() -> Self {
        Self {
            inner: ffi::create_birgen(),
        }
    }

    pub fn generate_program(&mut self, program: &Program) {
        for stmt in &program.statements {
            self.generate_statement(stmt);
        }
        self.inner.pin_mut().build_empty_return();
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
            Expression::Integer(lit) => self.inner.pin_mut().build_constant_int(lit.value),
            Expression::Float(lit) => self.inner.pin_mut().build_constant_float(lit.value),
            Expression::Infix(infix) => self.generate_infix(infix),
            Expression::Call(call) => {
                // HACK: this checks for the print function hardcoded-ly
                if let Expression::Identifier(ref ident) = *call.function
                    && ident.value == "print"
                {
                    // TODO: handle more than one arguments
                    let arg = self.generate_expression(&call.args[0]);
                    self.inner.pin_mut().build_print(&arg);

                    // TODO: maybe not return nullptr here
                    return cxx::UniquePtr::null();
                }

                todo!("Generation for call expression not implemented");
            },
            _ => todo!("Generation for expression {:?} not implemented", expr),
        }
    }

    fn generate_infix(&mut self, infix: &InfixExpression) -> cxx::UniquePtr<ffi::BIRValue> {
        let lhs = self.generate_expression(&infix.left);
        let rhs = self.generate_expression(&infix.right);

        match infix.operator {
            InfixKind::Add => self.inner.pin_mut().build_add(&lhs, &rhs),
            InfixKind::Sub => self.inner.pin_mut().build_sub(&lhs, &rhs),
            InfixKind::Mul => self.inner.pin_mut().build_mul(&lhs, &rhs),
            InfixKind::Div => self.inner.pin_mut().build_div(&lhs, &rhs),
            InfixKind::Mod => self.inner.pin_mut().build_mod(&lhs, &rhs),
            _ => todo!("Infix operator {:?} not implemented", infix.operator),
        }
    }

    pub fn dump(&self) {
        self.inner.dump();
    }

    pub fn dump_to_string(&self) -> String {
        self.inner.dump_to_string()
    }

    pub fn optimize(&mut self) -> bool {
        self.inner.pin_mut().optimize()
    }
}

impl Default for BIRGen {
    fn default() -> Self {
        Self::new()
    }
}

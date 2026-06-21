use std::collections::HashMap;

use ast::{
    Expression,
    InfixExpression,
    Program,
    Statement,
};
use lexer::{
    AssignmentKind,
    InfixKind,
};
use session::Session;

#[cxx::bridge(namespace = "belalang::birgen")]
mod ffi {
    unsafe extern "C++" {
        include!("belalang/BIRGen/BIRGen.h");

        type BIRValue;
        type BIRGen;
        type LLVMGen;

        fn create_birgen() -> UniquePtr<BIRGen>;

        fn build_constant_int(self: Pin<&mut BIRGen>, val: i64) -> UniquePtr<BIRValue>;
        fn build_constant_float(self: Pin<&mut BIRGen>, val: f64) -> UniquePtr<BIRValue>;
        fn build_constant_string(self: Pin<&mut BIRGen>, val: String) -> UniquePtr<BIRValue>;
        fn build_constant_bool(self: Pin<&mut BIRGen>, val: bool) -> UniquePtr<BIRValue>;
        fn build_add(self: Pin<&mut BIRGen>, lhs: &BIRValue, rhs: &BIRValue) -> UniquePtr<BIRValue>;
        fn build_sub(self: Pin<&mut BIRGen>, lhs: &BIRValue, rhs: &BIRValue) -> UniquePtr<BIRValue>;
        fn build_mul(self: Pin<&mut BIRGen>, lhs: &BIRValue, rhs: &BIRValue) -> UniquePtr<BIRValue>;
        fn build_div(self: Pin<&mut BIRGen>, lhs: &BIRValue, rhs: &BIRValue) -> UniquePtr<BIRValue>;
        fn build_mod(self: Pin<&mut BIRGen>, lhs: &BIRValue, rhs: &BIRValue) -> UniquePtr<BIRValue>;
        fn build_print(self: Pin<&mut BIRGen>, val: &BIRValue);
        fn build_var_declare(self: Pin<&mut BIRGen>, v: &BIRValue, name: String) -> UniquePtr<BIRValue>;
        fn build_var_load(self: Pin<&mut BIRGen>, refValue: &BIRValue) -> UniquePtr<BIRValue>;
        fn build_var_store(self: Pin<&mut BIRGen>, v: &BIRValue, refv: &BIRValue);
        fn build_empty_return(self: Pin<&mut BIRGen>);
        fn build_main_return(self: Pin<&mut BIRGen>);
        fn optimize(self: Pin<&mut BIRGen>) -> bool;
        fn dump(self: &BIRGen);
        fn dump_to_string(self: &BIRGen) -> String;
        fn llvmgen(self: Pin<&mut BIRGen>) -> UniquePtr<LLVMGen>;

        fn dump_to_string(self: &LLVMGen) -> String;
        fn compile_object_file(self: &LLVMGen, out: String) -> String;
    }
}

pub struct BIRGen<'sess> {
    #[allow(dead_code)]
    session: &'sess Session,
    inner: cxx::UniquePtr<ffi::BIRGen>,
    symbol_table: HashMap<String, cxx::UniquePtr<ffi::BIRValue>>,
}

impl<'sess> BIRGen<'sess> {
    pub fn new(session: &'sess Session) -> Self {
        Self {
            session,
            inner: ffi::create_birgen(),
            symbol_table: HashMap::new(),
        }
    }

    pub fn generate_program(&mut self, program: &Program) {
        for stmt in &program.statements {
            self.generate_statement(stmt);
        }
        self.inner.pin_mut().build_main_return();
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
            Expression::Boolean(lit) => self.inner.pin_mut().build_constant_bool(lit.value),
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
            Expression::Var(var) => match var.kind {
                AssignmentKind::ColonAssign => match *var.value {
                    Expression::Integer(ref i) => {
                        let v = self.inner.pin_mut().build_constant_int(i.value);
                        let declare = self.inner.pin_mut().build_var_declare(&v, var.name.value.clone());
                        self.inner.pin_mut().build_var_store(&v, &declare);
                        self.symbol_table.insert(var.name.value.clone(), declare);
                        cxx::UniquePtr::null() // FIXME: don't return nullptr
                    },
                    Expression::Float(ref f) => {
                        let v = self.inner.pin_mut().build_constant_float(f.value);
                        let declare = self.inner.pin_mut().build_var_declare(&v, var.name.value.clone());
                        self.inner.pin_mut().build_var_store(&v, &declare);
                        self.symbol_table.insert(var.name.value.clone(), declare);
                        cxx::UniquePtr::null() // FIXME: don't return nullptr
                    },
                    Expression::Identifier(_) | Expression::Infix(_) | Expression::String(_) => {
                        let v = self.generate_expression(&var.value);
                        let declare = self.inner.pin_mut().build_var_declare(&v, var.name.value.clone());
                        self.inner.pin_mut().build_var_store(&v, &declare);
                        self.symbol_table.insert(var.name.value.clone(), declare);
                        cxx::UniquePtr::null() // FIXME: don't return nullptr
                    },
                    _ => todo!("Generation for expression {:?} not implemented", expr),
                },
                _ => todo!("Generation for expression {:?} not implemented", expr),
            },
            Expression::Identifier(ident) => {
                if let Some(ssa) = self.symbol_table.get(&ident.value) {
                    self.inner.pin_mut().build_var_load(ssa)
                } else {
                    cxx::UniquePtr::null() // FIXME: don't return nullptr
                }
            },
            Expression::String(s) => self.inner.pin_mut().build_constant_string(s.value.clone()),
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

    pub fn llvmgen(&mut self) -> LLVMGen<'sess> {
        LLVMGen {
            session: self.session,
            inner: self.inner.pin_mut().llvmgen(),
        }
    }
}

pub struct LLVMGen<'sess> {
    #[allow(dead_code)]
    session: &'sess Session,
    inner: cxx::UniquePtr<ffi::LLVMGen>,
}

impl<'sess> LLVMGen<'sess> {
    pub fn dump_to_string(&self) -> String {
        self.inner.dump_to_string()
    }

    pub fn compile_object_file(&self, out: String) -> String {
        self.inner.compile_object_file(out)
    }
}

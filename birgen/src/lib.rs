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
use session::{
    Session,
    interner::{
        Symbol,
        syms,
    },
};

#[cxx::bridge(namespace = "belalang::birgen")]
mod ffi {
    #[repr(u8)]
    enum BinOpKind {
        Add,
        Sub,
        Mul,
        Div,
        Mod,
        Lt,
        Le,
        Gt,
        Ge,
        Eq,
        Ne,
    }

    #[repr(u8)]
    enum TypeKind {
        String,
        Int,
        Float,
    }

    unsafe extern "C++" {
        include!("belalang/BIRGen/BIRGen.h");

        type BIRGuard;
        type BIRFunctionGuard;
        type BIRIfGuard;
        type BIRValue;
        type BIRGen;
        type LLVMGen;

        fn create_birgen() -> UniquePtr<BIRGen>;
        fn create_llvmgen(birgen: Pin<&mut BIRGen>) -> UniquePtr<LLVMGen>;

        fn build_constant_int(self: Pin<&mut BIRGen>, val: i64) -> UniquePtr<BIRValue>;
        fn build_constant_float(self: Pin<&mut BIRGen>, val: f64) -> UniquePtr<BIRValue>;
        fn build_constant_string(self: Pin<&mut BIRGen>, val: &str) -> UniquePtr<BIRValue>;
        fn build_constant_bool(self: Pin<&mut BIRGen>, val: bool) -> UniquePtr<BIRValue>;
        fn build_binop(self: Pin<&mut BIRGen>, kind: BinOpKind, lhs: &BIRValue, rhs: &BIRValue) -> UniquePtr<BIRValue>;
        fn build_print(self: Pin<&mut BIRGen>, val: &BIRValue);
        fn build_var_declare(self: Pin<&mut BIRGen>, v: &BIRValue, name: &str) -> UniquePtr<BIRValue>;
        fn build_var_declare_ty(self: Pin<&mut BIRGen>, v: TypeKind, name: &str) -> UniquePtr<BIRValue>;
        fn build_var_load(self: Pin<&mut BIRGen>, refValue: &BIRValue) -> UniquePtr<BIRValue>;
        fn build_var_store(self: Pin<&mut BIRGen>, v: &BIRValue, refv: &BIRValue);
        fn build_fn_expr(
            self: Pin<&mut BIRGen>,
            resultTy: TypeKind,
            paramTys: &[TypeKind],
        ) -> UniquePtr<BIRFunctionGuard>;
        fn build_return(self: Pin<&mut BIRGen>, val: &BIRValue);
        fn build_empty_return(self: Pin<&mut BIRGen>);
        fn build_main_return(self: Pin<&mut BIRGen>);
        fn build_if_expr(self: Pin<&mut BIRGen>, cond: &BIRValue) -> UniquePtr<BIRIfGuard>;
        fn build_yield(self: Pin<&mut BIRGen>, val: &BIRValue);
        fn build_empty_yield(self: Pin<&mut BIRGen>);
        fn start_then(self: Pin<&mut BIRIfGuard>);
        fn start_else(self: Pin<&mut BIRIfGuard>);
        fn get_value(self: &BIRIfGuard) -> UniquePtr<BIRValue>;
        fn optimize(self: Pin<&mut BIRGen>) -> bool;
        fn dump(self: &BIRGen);
        fn dump_to_string(self: &BIRGen) -> String;

        fn dump_to_string(self: &LLVMGen) -> String;
        fn compile_object_file(self: &LLVMGen, out: String) -> String;

        fn start_call(self: Pin<&mut BIRGen>, callee: &BIRValue);
        fn add_call_arg(self: Pin<&mut BIRGen>, arg: &BIRValue);
        fn finish_call(self: Pin<&mut BIRGen>) -> UniquePtr<BIRValue>;

        fn get_value(self: &BIRFunctionGuard) -> UniquePtr<BIRValue>;
        fn get_arg(self: &BIRFunctionGuard, index: usize) -> UniquePtr<BIRValue>;
    }
}

pub struct BIRGen<'sess> {
    #[allow(dead_code)]
    session: &'sess Session,
    inner: cxx::UniquePtr<ffi::BIRGen>,
    symbol_table: HashMap<Symbol, cxx::UniquePtr<ffi::BIRValue>>,
}

impl<'sess> BIRGen<'sess> {
    pub fn new(session: &'sess Session) -> Self {
        Self {
            session,
            inner: ffi::create_birgen(),
            symbol_table: HashMap::new(),
        }
    }

    pub fn generate_program<'ast>(&mut self, program: &Program<'ast>) {
        for stmt in program.statements {
            self.generate_statement(stmt);
        }
        self.inner.pin_mut().build_main_return();
    }

    pub fn generate_statement<'ast>(&mut self, stmt: &Statement<'ast>) {
        match stmt {
            Statement::Expression(expr_stmt) => {
                self.generate_expression(&expr_stmt.expression);
            },
            Statement::Return(s) => {
                if let Some(ref return_value) = s.return_value {
                    let expr = self.generate_expression(&return_value);
                    self.inner.pin_mut().build_return(&expr);
                } else {
                    self.inner.pin_mut().build_empty_return();
                }
            },
            Statement::While(_while_stmt) => {
                // TODO: Implement while
            },
            Statement::VarDecl(var) => {
                let value = &var.value;

                let Some(value) = value else {
                    let id = match var.explicit_ty.unwrap() {
                        syms::STRING => ffi::TypeKind::String,
                        syms::INT => ffi::TypeKind::Int,
                        syms::FLOAT => ffi::TypeKind::Float,
                        _ => todo!(),
                    };

                    let name = self.session.lookup_string(var.name.value);
                    let declare = self.inner.pin_mut().build_var_declare_ty(id, name);
                    self.symbol_table.insert(var.name.value, declare);
                    return;
                };

                match **value {
                    Expression::Integer(ref i) => {
                        let v = self.inner.pin_mut().build_constant_int(i.value);
                        let name = self.session.lookup_string(var.name.value);
                        let declare = self.inner.pin_mut().build_var_declare(&v, name);
                        self.inner.pin_mut().build_var_store(&v, &declare);
                        self.symbol_table.insert(var.name.value, declare);
                    },
                    Expression::Float(ref f) => {
                        let v = self.inner.pin_mut().build_constant_float(f.value);
                        let name = self.session.lookup_string(var.name.value);
                        let declare = self.inner.pin_mut().build_var_declare(&v, name);
                        self.inner.pin_mut().build_var_store(&v, &declare);
                        self.symbol_table.insert(var.name.value, declare);
                    },
                    Expression::Identifier(_)
                    | Expression::Infix(_)
                    | Expression::String(_)
                    | Expression::Function(_)
                    | Expression::Call(_) => {
                        let v = self.generate_expression(&value);
                        let name = self.session.lookup_string(var.name.value);
                        let declare = self.inner.pin_mut().build_var_declare(&v, name);
                        self.inner.pin_mut().build_var_store(&v, &declare);
                        self.symbol_table.insert(var.name.value, declare);
                    },
                    _ => todo!("Generation for expression {:?} not implemented", **value),
                }
            },
            Statement::StructDecl(s) => {
                // TODO: Implement struct declaration
            },
            Statement::Break(s) => {
                // TODO: Implement break
            },
            Statement::Continue(s) => {
                // TODO: Implement continue
            },
        }
    }

    pub fn generate_expression<'ast>(&mut self, expr: &Expression<'ast>) -> cxx::UniquePtr<ffi::BIRValue> {
        match expr {
            Expression::Integer(lit) => self.inner.pin_mut().build_constant_int(lit.value),
            Expression::Float(lit) => self.inner.pin_mut().build_constant_float(lit.value),
            Expression::Boolean(lit) => self.inner.pin_mut().build_constant_bool(lit.value),
            Expression::Infix(infix) => self.generate_infix(infix),
            Expression::Call(call) => {
                // HACK: this checks for the print function hardcoded-ly
                if let Expression::Identifier(ref ident) = *call.function
                    && ident.value == syms::PRINT
                {
                    // TODO: handle more than one arguments
                    let arg = self.generate_expression(&call.args[0]);
                    self.inner.pin_mut().build_print(&arg);

                    // TODO: maybe not return nullptr here
                    return cxx::UniquePtr::null();
                }

                let callee = self.generate_expression(call.function);
                self.inner.pin_mut().start_call(&callee);
                for arg in call.args {
                    let arg_val = self.generate_expression(arg);
                    self.inner.pin_mut().add_call_arg(&arg_val);
                }
                self.inner.pin_mut().finish_call()
            },
            Expression::Var(var) => {
                let name_sym = var.name.value;
                let val = self.generate_expression(var.value);
                let ssa = self.symbol_table.get(&name_sym).expect("Variable not declared");
                match var.kind {
                    AssignmentKind::Assign => {
                        self.inner.pin_mut().build_var_store(&val, ssa);
                        val
                    },
                    AssignmentKind::AddAssign
                    | AssignmentKind::SubAssign
                    | AssignmentKind::MulAssign
                    | AssignmentKind::DivAssign
                    | AssignmentKind::ModAssign => {
                        let op = match var.kind {
                            AssignmentKind::AddAssign => ffi::BinOpKind::Add,
                            AssignmentKind::SubAssign => ffi::BinOpKind::Sub,
                            AssignmentKind::MulAssign => ffi::BinOpKind::Mul,
                            AssignmentKind::DivAssign => ffi::BinOpKind::Div,
                            AssignmentKind::ModAssign => ffi::BinOpKind::Mod,
                            _ => unreachable!(),
                        };
                        let current_val = self.inner.pin_mut().build_var_load(ssa);
                        let new_val = self.inner.pin_mut().build_binop(op, &current_val, &val);
                        self.inner.pin_mut().build_var_store(&new_val, ssa);
                        new_val
                    },
                    _ => todo!("Assignment kind {:?} not supported", var.kind),
                }
            },
            Expression::Identifier(ident) => {
                if let Some(ssa) = self.symbol_table.get(&ident.value) {
                    self.inner.pin_mut().build_var_load(ssa)
                } else {
                    cxx::UniquePtr::null() // FIXME: don't return nullptr
                }
            },
            Expression::String(s) => {
                let v = self.session.lookup_string(s.value);
                self.inner.pin_mut().build_constant_string(v)
            },
            Expression::Function(func) => {
                let result = match func.explicit_ty.unwrap() {
                    syms::STRING => ffi::TypeKind::String,
                    syms::INT => ffi::TypeKind::Int,
                    syms::FLOAT => ffi::TypeKind::Float,
                    _ => todo!(),
                };

                let mut param_tys = Vec::new();
                for param in func.params {
                    let ty = match param.explicit_ty.unwrap() {
                        syms::STRING => ffi::TypeKind::String,
                        syms::INT => ffi::TypeKind::Int,
                        syms::FLOAT => ffi::TypeKind::Float,
                        _ => todo!(),
                    };
                    param_tys.push(ty);
                }

                let guard = self.inner.pin_mut().build_fn_expr(result, &param_tys);

                let mut saved_symbols = Vec::new();
                for (i, param) in func.params.iter().enumerate() {
                    let arg_val = guard.get_arg(i);
                    let name = self.session.lookup_string(param.name.value);
                    let declare = self.inner.pin_mut().build_var_declare(&arg_val, name);
                    self.inner.pin_mut().build_var_store(&arg_val, &declare);
                    let prev = self.symbol_table.insert(param.name.value, declare);
                    saved_symbols.push((param.name.value, prev));
                }

                for stmt in func.body.statements {
                    self.generate_statement(stmt);
                }

                for (sym, prev) in saved_symbols {
                    if let Some(prev_val) = prev {
                        self.symbol_table.insert(sym, prev_val);
                    } else {
                        self.symbol_table.remove(&sym);
                    }
                }

                guard.get_value()
            },
            Expression::If(if_expr) => {
                let cond = self.generate_expression(if_expr.condition);
                let mut guard = self.inner.pin_mut().build_if_expr(&cond);

                // TODO: handle yielding if expressions
                guard.pin_mut().start_then();
                for stmt in if_expr.consequence.statements {
                    self.generate_statement(stmt);
                }
                self.inner.pin_mut().build_empty_yield();

                if let Some(alt) = if_expr.alternative {
                    guard.pin_mut().start_else();
                    match alt {
                        Expression::Block(block) => {
                            for stmt in block.statements {
                                self.generate_statement(stmt);
                            }
                        },
                        _ => {
                            self.generate_expression(alt);
                        },
                    }
                    self.inner.pin_mut().build_empty_yield();
                }

                guard.get_value()
            },
            _ => todo!("Generation for expression {:?} not implemented", expr),
        }
    }

    fn generate_infix<'ast>(&mut self, infix: &InfixExpression<'ast>) -> cxx::UniquePtr<ffi::BIRValue> {
        let lhs = self.generate_expression(infix.left);
        let rhs = self.generate_expression(infix.right);

        let kind = match infix.operator {
            InfixKind::Add => ffi::BinOpKind::Add,
            InfixKind::Sub => ffi::BinOpKind::Sub,
            InfixKind::Mul => ffi::BinOpKind::Mul,
            InfixKind::Div => ffi::BinOpKind::Div,
            InfixKind::Mod => ffi::BinOpKind::Mod,
            InfixKind::Lt => ffi::BinOpKind::Lt,
            InfixKind::Le => ffi::BinOpKind::Le,
            InfixKind::Gt => ffi::BinOpKind::Gt,
            InfixKind::Ge => ffi::BinOpKind::Ge,
            InfixKind::Eq => ffi::BinOpKind::Eq,
            InfixKind::Ne => ffi::BinOpKind::Ne,
            _ => todo!("Infix operator {:?} not implemented", infix.operator),
        };
        self.inner.pin_mut().build_binop(kind, &lhs, &rhs)
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
            inner: ffi::create_llvmgen(self.inner.pin_mut()),
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

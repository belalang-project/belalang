mod error;
mod scope;

use belc_ast::{
    BlockExpression,
    Expression,
    Program,
    Statement,
};
use belc_lexer::{
    AssignmentKind,
    InfixKind,
    PrefixKind,
};
use belvm_bytecode::{
    Bytecode,
    Constant,
    opcode,
};
use scope::{
    ScopeLevel,
    ScopeManager,
};

use crate::error::CodegenError;

#[derive(Default)]
pub struct Compiler {
    prev_constants: usize,

    pub constants: Vec<Constant>,
    pub scope: ScopeManager,
}

impl Compiler {
    pub fn compile_program(&mut self, program: Program) -> Result<Bytecode, CodegenError> {
        for statement in program.statements {
            self.compile_statement(statement)?;
        }

        let mut instructions: Vec<_> = self.scope.main_scope.instructions.drain(..).collect();

        instructions.push(opcode::RETURN_VALUE);

        let constants = self.constants[self.prev_constants..].to_vec();
        self.prev_constants = self.constants.len();

        Ok(Bytecode {
            instructions,
            constants,
        })
    }

    pub fn compile_statement(&mut self, statement: Statement) -> Result<(), CodegenError> {
        match statement {
            Statement::Expression(statement) => {
                self.compile_expression(statement.expression)?;
                self.add_bytecode(opcode::POP);
            },

            Statement::Return(r#return) => {
                self.compile_expression(r#return.return_value)?;
                self.add_bytecode(opcode::RETURN_VALUE);
            },

            Statement::While(r#while) => {
                let start_of_while = self.scope.current().instructions.len();

                self.compile_expression(*r#while.condition)?;

                let jif = self.add_instruction(opcode::jump_if_false(0).to_vec());
                let jif_index = self.scope.current().instructions.len();

                // self.scope.enter();
                self.compile_block(r#while.block)?;
                // let scope = self.scope.leave();

                // self.scope
                //     .current_mut()
                //     .instructions
                //     .extend(scope.instructions);

                let jump = self.add_instruction(opcode::jump(0).to_vec());
                let current = self.scope.current().instructions.len();

                self.replace_u16_operand(jump, (start_of_while as isize - current as isize) as u16);
                self.replace_u16_operand(jif, (current as isize - jif_index as isize) as u16);

                self.add_bytecode(opcode::NOOP);
            },
        };

        Ok(())
    }

    pub fn compile_expression(&mut self, expression: Expression) -> Result<(), CodegenError> {
        match expression {
            Expression::Boolean(boolean) => {
                self.add_bytecode(if boolean.value { opcode::TRUE } else { opcode::FALSE });
            },

            Expression::Integer(integer) => {
                let integer = Constant::Integer(integer.value);
                let index = self.add_constant(integer) as u16;
                self.add_instruction(opcode::constant(index).to_vec());
            },

            Expression::Float(_float) => {
                // let float = Object::Float(float.value);
                // let index = self.add_constant(float) as u16;
                // self.add_instruction(opcode::constant(index).to_vec());
            },

            Expression::String(_string) => {
                // let string = Object::String(string.value);
                // let index = self.add_constant(string) as u16;
                // self.add_instruction(opcode::constant(index).to_vec());
            },

            Expression::Null(_) => {
                // let null = Object::Null;
                // let index = self.add_constant(null) as u16;
                // self.add_instruction(opcode::constant(index).to_vec());
            },

            Expression::Array(_array) => {
                // let array_len = array.elements.len() as u16;
                //
                // for element in array.elements.into_iter().rev() {
                //     self.compile_expression(element)?;
                // }
                //
                // self.add_instruction(opcode::array(array_len).to_vec());
            },

            Expression::Var(var) => match var.kind {
                AssignmentKind::ColonAssign => {
                    let symbol = self.scope.define(var.name.value)?;
                    let scope = symbol.scope;
                    let index = symbol.index;

                    self.compile_expression(*var.value)?;

                    self.set_variable(&scope, index);
                },
                _ => {
                    let symbol = self.scope.resolve(var.name.value)?;
                    let scope = symbol.scope;
                    let index = symbol.index;

                    match var.kind {
                        AssignmentKind::Assign => {
                            self.compile_expression(*var.value)?;
                        },
                        AssignmentKind::AddAssign => {
                            self.get_variable(&scope, index);
                            self.compile_expression(*var.value)?;
                            self.add_bytecode(opcode::ADD);
                        },
                        AssignmentKind::SubAssign => {
                            self.get_variable(&scope, index);
                            self.compile_expression(*var.value)?;
                            self.add_bytecode(opcode::SUB);
                        },
                        AssignmentKind::MulAssign => {
                            self.get_variable(&scope, index);
                            self.compile_expression(*var.value)?;
                            self.add_bytecode(opcode::MUL);
                        },
                        AssignmentKind::DivAssign => {
                            self.get_variable(&scope, index);
                            self.compile_expression(*var.value)?;
                            self.add_bytecode(opcode::DIV);
                        },
                        AssignmentKind::ModAssign => {
                            self.get_variable(&scope, index);
                            self.compile_expression(*var.value)?;
                            self.add_bytecode(opcode::MOD);
                        },
                        AssignmentKind::BitAndAssign => {
                            self.get_variable(&scope, index);
                            self.compile_expression(*var.value)?;
                            self.add_bytecode(opcode::BIT_AND);
                        },
                        AssignmentKind::BitOrAssign => {
                            self.get_variable(&scope, index);
                            self.compile_expression(*var.value)?;
                            self.add_bytecode(opcode::BIT_OR);
                        },
                        AssignmentKind::BitXorAssign => {
                            self.get_variable(&scope, index);
                            self.compile_expression(*var.value)?;
                            self.add_bytecode(opcode::BIT_XOR);
                        },
                        AssignmentKind::ShiftLeftAssign => {
                            self.get_variable(&scope, index);
                            self.compile_expression(*var.value)?;
                            self.add_bytecode(opcode::BIT_SL);
                        },
                        AssignmentKind::ShiftRightAssign => {
                            self.get_variable(&scope, index);
                            self.compile_expression(*var.value)?;
                            self.add_bytecode(opcode::BIT_SR);
                        },
                        _ => unreachable!(),
                    }

                    self.set_variable(&scope, index);
                },
            },

            Expression::Call(call) => {
                for arg in call.args.into_iter().rev() {
                    self.compile_expression(arg)?;
                }

                self.compile_expression(*call.function)?;
                self.add_bytecode(opcode::CALL);
            },

            Expression::Index(index) => {
                self.compile_expression(*index.left)?;
                self.compile_expression(*index.index)?;
                self.add_bytecode(opcode::INDEX);
            },

            Expression::Function(_function) => {
                todo!()
            },

            Expression::Identifier(ident) => {
                let symbol = self.scope.resolve(ident.value)?;
                let scope = symbol.scope;
                let index = symbol.index;

                self.get_variable(&scope, index);
            },

            Expression::If(r#if) => {
                self.compile_expression(*r#if.condition)?;

                let jif = self.add_instruction(opcode::jump_if_false(0).to_vec());
                let jif_index = self.scope.current().instructions.len();

                // self.scope.enter();
                self.compile_block(r#if.consequence)?;
                // let scope = self.scope.leave();

                // self.scope
                //     .current_mut()
                //     .instructions
                //     .extend(scope.instructions);

                let jump = self.add_instruction(opcode::jump(0).to_vec());
                let jump_index = self.scope.current().instructions.len();

                let post_consequence = self.scope.current().instructions.len();
                self.replace_u16_operand(jif, (post_consequence - jif_index) as u16);

                match r#if.alternative {
                    None => {
                        self.add_bytecode(opcode::NULL);
                    },
                    Some(alt) => match *alt {
                        Expression::Block(block) => {
                            // self.scope.enter();
                            self.compile_block(block)?;
                            // let scope = self.scope.leave();

                            // self.scope
                            //     .current_mut()
                            //     .instructions
                            //     .extend(scope.instructions);
                        },
                        _ => {
                            self.compile_expression(*alt)?;
                        },
                    },
                };

                let post_alternative = self.scope.current().instructions.len();
                self.replace_u16_operand(jump, (post_alternative - jump_index) as u16);
            },

            Expression::Infix(infix) => {
                match infix.operator {
                    InfixKind::Gt | InfixKind::Ge => {
                        self.compile_expression(*infix.right)?;
                        self.compile_expression(*infix.left)?;
                    },
                    _ => {
                        self.compile_expression(*infix.left)?;
                        self.compile_expression(*infix.right)?;
                    },
                }

                self.add_bytecode(match infix.operator {
                    InfixKind::Add => opcode::ADD,
                    InfixKind::Sub => opcode::SUB,
                    InfixKind::Mul => opcode::MUL,
                    InfixKind::Div => opcode::DIV,
                    InfixKind::Mod => opcode::MOD,
                    InfixKind::Eq => opcode::EQUAL,
                    InfixKind::Ne => opcode::NOT_EQUAL,
                    InfixKind::And => opcode::AND,
                    InfixKind::Or => opcode::OR,
                    InfixKind::BitAnd => opcode::BIT_AND,
                    InfixKind::BitOr => opcode::BIT_OR,
                    InfixKind::BitXor => opcode::BIT_XOR,
                    InfixKind::ShiftLeft => opcode::BIT_SL,
                    InfixKind::ShiftRight => opcode::BIT_SR,
                    InfixKind::Lt | InfixKind::Gt => opcode::LESS_THAN,
                    InfixKind::Le | InfixKind::Ge => opcode::LESS_THAN_EQUAL,
                });
            },

            Expression::Prefix(prefix) => {
                self.compile_expression(*prefix.right)?;
                self.add_bytecode(match prefix.operator {
                    PrefixKind::Sub => opcode::MINUS,
                    PrefixKind::Not => opcode::BANG,
                });
            },

            Expression::Block(block) => {
                // self.scope.enter();
                self.compile_block(block)?;
                // let scope = self.scope.leave();

                // self.scope
                //     .current_mut()
                //     .instructions
                //     .extend(scope.instructions);
            },
        };

        Ok(())
    }

    fn compile_block(&mut self, block: BlockExpression) -> Result<(), CodegenError> {
        for statement in block.statements {
            self.compile_statement(statement)?;
        }

        if let Some(&opcode::POP) = self.scope.current().instructions.last() {
            self.scope.current_mut().instructions.pop();
        }

        Ok(())
    }

    pub fn add_constant(&mut self, obj: Constant) -> usize {
        self.constants.push(obj);
        self.constants.len() - 1
    }

    pub fn add_bytecode(&mut self, byte: u8) -> usize {
        let scope = self.scope.current_mut();

        scope.instructions.push(byte);
        scope.instructions.len() - 1
    }

    pub fn add_instruction(&mut self, bytes: Vec<u8>) -> usize {
        let pos = self.scope.current_mut().instructions.len();

        for byte in bytes {
            self.add_bytecode(byte);
        }

        pos
    }

    pub fn replace_u16_operand(&mut self, index: usize, value: u16) {
        let scope = self.scope.current_mut();

        scope.instructions[index + 1] = (value >> 8) as u8;
        scope.instructions[index + 2] = (value & 0xFF) as u8;
    }

    fn get_variable(&mut self, scope: &ScopeLevel, index: usize) -> usize {
        match scope {
            ScopeLevel::Global => self.add_instruction(opcode::get_global(index as u16).to_vec()),
            ScopeLevel::Local => self.add_instruction(opcode::get_local(index as u8).to_vec()),
            ScopeLevel::Builtin => self.add_instruction(opcode::get_builtin(index as u8).to_vec()),
        }
    }

    fn set_variable(&mut self, scope: &ScopeLevel, index: usize) -> usize {
        match scope {
            ScopeLevel::Global => self.add_instruction(opcode::set_global(index as u16).to_vec()),
            ScopeLevel::Local => self.add_instruction(opcode::set_local(index as u8).to_vec()),
            ScopeLevel::Builtin => 0,
        }
    }
}

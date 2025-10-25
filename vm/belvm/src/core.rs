use belvm_bytecode::{
    Bytecode,
    Constant,
    opcode,
};

use crate::{
    errors::RuntimeError,
    io::VMIO,
    stack::{
        Stack,
        StackValue,
    },
};

/// The core Virtual Machine structure.
#[derive(Default)]
pub struct VM {
    /// The current instruction the VM is pointing to. This is why it's
    /// named `ip` (short for instruction pointer).
    ip: usize,

    /// The list of bytecode instructions the VM is executing. The value of
    /// this field is supplied through the [`Bytecode`] struct.
    instructions: Vec<u8>,

    /// The list of constants the VM is working with. The value of this field
    /// is supplied through the [`Bytecode`] struct.
    constants: Vec<Constant>,

    /// The stack memory of the VM.
    stack: Stack,

    /// I/O operation for VM.
    io: VMIO,
}

impl VM {
    /// Executes the provided [`Bytecode`] program.
    ///
    /// # Arguments
    /// * `code` -- The [`Bytecode`] to be executed.
    ///
    /// # Example
    /// ```rust,ignore
    /// let constants = vec![Constant::Integer(12), Constant::Integer(5)];
    ///
    /// let mut instructions = Vec::new();
    /// instructions.extend(opcode::constant(0));
    /// instructions.extend(opcode::constant(1));
    /// instructions.push(opcode::POP);
    ///
    /// let mut vm = VM::default();
    ///
    /// vm.run(Bytecode {
    ///     instructions,
    ///     constants,
    /// })
    /// ```
    pub fn run(&mut self, code: Bytecode) -> Result<(), RuntimeError> {
        self.constants.extend(code.constants);
        self.instructions.extend(code.instructions);

        while self.ip < self.instructions.len() {
            let op = self.instructions[self.ip];

            match op {
                opcode::NOOP => {},

                opcode::POP => {
                    self.stack.pop()?;
                },

                opcode::ADD => {
                    use StackValue::*;

                    let right = self.stack.pop()?;
                    let left = self.stack.pop()?;

                    let result = match (left, right) {
                        (Integer(a), Integer(b)) => Ok(Integer(a + b)),
                        (_, _) => Err(RuntimeError::TypeError),
                    }?;

                    self.stack.push(result)?;
                },

                opcode::SUB => {
                    use StackValue::*;

                    let right = self.stack.pop()?;
                    let left = self.stack.pop()?;

                    let result = match (left, right) {
                        (Integer(a), Integer(b)) => Ok(Integer(a - b)),
                        (_, _) => Err(RuntimeError::TypeError),
                    }?;

                    self.stack.push(result)?;
                },

                opcode::MUL => {
                    use StackValue::*;

                    let right = self.stack.pop()?;
                    let left = self.stack.pop()?;

                    let result = match (left, right) {
                        (Integer(a), Integer(b)) => Ok(Integer(a * b)),
                        (_, _) => Err(RuntimeError::TypeError),
                    }?;

                    self.stack.push(result)?;
                },

                opcode::DIV => {
                    use StackValue::*;

                    let right = self.stack.pop()?;
                    let left = self.stack.pop()?;

                    let result = match (left, right) {
                        (Integer(a), Integer(b)) => Ok(Integer(a / b)),
                        (_, _) => Err(RuntimeError::TypeError),
                    }?;

                    self.stack.push(result)?;
                },

                opcode::MOD => {
                    use StackValue::*;

                    let right = self.stack.pop()?;
                    let left = self.stack.pop()?;

                    let result = match (left, right) {
                        (Integer(a), Integer(b)) => Ok(Integer(a % b)),
                        (_, _) => Err(RuntimeError::TypeError),
                    }?;

                    self.stack.push(result)?;
                },

                opcode::CONSTANT => {
                    let index = self.read_u16();
                    let constant = self.constants[index as usize].clone();

                    let object = match constant {
                        Constant::Integer(int) => StackValue::Integer(int),
                        Constant::Boolean(boolean) => StackValue::Boolean(boolean),
                        Constant::String(string) => StackValue::String(string),
                        Constant::Null => todo!(),
                    };

                    self.stack.push(object)?;
                },

                opcode::TRUE => {
                    self.stack.push(StackValue::Boolean(true))?;
                },

                opcode::FALSE => {
                    self.stack.push(StackValue::Boolean(false))?;
                },

                opcode::NULL => {
                    self.stack.push(StackValue::Null)?;
                },

                opcode::EQUAL => {
                    use StackValue::*;

                    let right = self.stack.pop()?;
                    let left = self.stack.pop()?;

                    let result = match (left, right) {
                        (Integer(a), Integer(b)) => Ok(Boolean(a == b)),
                        (Boolean(a), Boolean(b)) => Ok(Boolean(a == b)),
                        (_, _) => Err(RuntimeError::TypeError),
                    }?;

                    self.stack.push(result)?;
                },

                opcode::NOT_EQUAL => {
                    use StackValue::*;

                    let right = self.stack.pop()?;
                    let left = self.stack.pop()?;

                    let result = match (left, right) {
                        (Integer(a), Integer(b)) => Ok(Boolean(a != b)),
                        (Boolean(a), Boolean(b)) => Ok(Boolean(a != b)),
                        (_, _) => Err(RuntimeError::TypeError),
                    }?;

                    self.stack.push(result)?;
                },

                opcode::LESS_THAN => {
                    use StackValue::*;

                    let right = self.stack.pop()?;
                    let left = self.stack.pop()?;

                    let result = match (left, right) {
                        (Integer(a), Integer(b)) => Ok(Boolean(a < b)),
                        (_, _) => Err(RuntimeError::TypeError),
                    }?;

                    self.stack.push(result)?;
                },

                opcode::LESS_THAN_EQUAL => {
                    use StackValue::*;

                    let right = self.stack.pop()?;
                    let left = self.stack.pop()?;

                    let result = match (left, right) {
                        (Integer(a), Integer(b)) => Ok(Boolean(a <= b)),
                        (_, _) => Err(RuntimeError::TypeError),
                    }?;

                    self.stack.push(result)?;
                },

                opcode::AND => {
                    use StackValue::*;

                    let right = self.stack.pop()?;
                    let left = self.stack.pop()?;

                    let result = match (left, right) {
                        (Boolean(a), Boolean(b)) => Ok(Boolean(a && b)),
                        (_, _) => Err(RuntimeError::TypeError),
                    }?;

                    self.stack.push(result)?;
                },

                opcode::OR => {
                    use StackValue::*;

                    let right = self.stack.pop()?;
                    let left = self.stack.pop()?;

                    let result = match (left, right) {
                        (Boolean(a), Boolean(b)) => Ok(Boolean(a || b)),
                        (_, _) => Err(RuntimeError::TypeError),
                    }?;

                    self.stack.push(result)?;
                },

                opcode::BIT_AND => {
                    use StackValue::*;

                    let right = self.stack.pop()?;
                    let left = self.stack.pop()?;

                    let result = match (left, right) {
                        (Integer(a), Integer(b)) => Ok(Integer(a & b)),
                        (_, _) => Err(RuntimeError::TypeError),
                    }?;

                    self.stack.push(result)?;
                },

                opcode::BIT_OR => {
                    use StackValue::*;

                    let right = self.stack.pop()?;
                    let left = self.stack.pop()?;

                    let result = match (left, right) {
                        (Integer(a), Integer(b)) => Ok(Integer(a | b)),
                        (_, _) => Err(RuntimeError::TypeError),
                    }?;

                    self.stack.push(result)?;
                },

                opcode::BIT_XOR => {
                    use StackValue::*;

                    let right = self.stack.pop()?;
                    let left = self.stack.pop()?;

                    let result = match (left, right) {
                        (Integer(a), Integer(b)) => Ok(Integer(a ^ b)),
                        (_, _) => Err(RuntimeError::TypeError),
                    }?;

                    self.stack.push(result)?;
                },

                opcode::BIT_SL => {
                    use StackValue::*;

                    let right = self.stack.pop()?;
                    let left = self.stack.pop()?;

                    let result = match (left, right) {
                        (Integer(a), Integer(b)) => Ok(Integer(a << b)),
                        (_, _) => Err(RuntimeError::TypeError),
                    }?;

                    self.stack.push(result)?;
                },

                opcode::BIT_SR => {
                    use StackValue::*;

                    let right = self.stack.pop()?;
                    let left = self.stack.pop()?;

                    let result = match (left, right) {
                        (Integer(a), Integer(b)) => Ok(Integer(a >> b)),
                        (_, _) => Err(RuntimeError::TypeError),
                    }?;

                    self.stack.push(result)?;
                },

                opcode::BANG => {
                    use StackValue::*;

                    let right = self.stack.pop()?;

                    let result = match right {
                        Boolean(a) => Ok(Boolean(!a)),
                        _ => Err(RuntimeError::TypeError),
                    }?;

                    self.stack.push(result)?;
                },

                opcode::MINUS => {
                    use StackValue::*;

                    let right = self.stack.pop()?;

                    let result = match right {
                        Integer(a) => Ok(Integer(-a)),
                        _ => Err(RuntimeError::TypeError),
                    }?;

                    self.stack.push(result)?;
                },

                opcode::JUMP => {
                    let relative = self.read_u16() as i16;
                    self.increment_ip(relative as usize);
                },

                opcode::JUMP_IF_FALSE => {
                    use StackValue::*;

                    let relative = self.read_u16() as i16;
                    let right = self.stack.pop()?;

                    let result = match right {
                        Integer(a) => Ok(a > 0),
                        Boolean(a) => Ok(a),
                        _ => Err(RuntimeError::TypeError),
                    }?;

                    if !result {
                        self.increment_ip(relative as usize);
                    }
                },

                opcode::PRINT => {
                    let value = self.stack.pop()?;
                    self.io.print(&value.to_string());
                    self.io.print("\n");
                },

                _ => return Err(RuntimeError::UnknownInstruction(op)),
            };

            self.increment_ip(1);
        }

        Ok(())
    }

    fn increment_ip(&mut self, value: usize) {
        self.ip = self.ip.checked_add_signed(value as isize).unwrap();
    }

    fn read_u16(&mut self) -> u16 {
        let hi = self.instructions[self.ip + 1];
        let lo = self.instructions[self.ip + 2];
        self.ip += 2;

        ((hi as u16) << 8) | (lo as u16)
    }

    #[allow(dead_code)]
    fn read_u8(&mut self) -> u8 {
        let v = self.instructions[self.ip + 1];
        self.ip += 1;

        v
    }

    pub fn stack_size(&self) -> usize {
        self.stack.size()
    }

    pub fn stack_pop(&mut self) -> Result<StackValue, RuntimeError> {
        self.stack.pop()
    }
}

impl Drop for VM {
    fn drop(&mut self) {
        self.instructions.clear();
        self.constants.clear();

        std::mem::drop(std::mem::take(&mut self.stack));
    }
}

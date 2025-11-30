use belalang_vm::{
    VM,
    stack::StackValue,
};
use belalang_bytecode::{
    Bytecode,
    Constant,
};

#[derive(Default)]
pub struct VMBuilder {
    instructions: Vec<u8>,
    constants: Vec<Constant>,
}

impl VMBuilder {
    pub fn with_instructions(mut self, instructions: Vec<u8>) -> Self {
        self.instructions = instructions;
        self
    }

    pub fn with_constants(mut self, constants: Vec<Constant>) -> Self {
        self.constants = constants;
        self
    }

    pub fn run_ok(self) -> VMRunner {
        let mut vm = VM::default();
        let result = vm.run(Bytecode {
            instructions: self.instructions,
            constants: self.constants,
        });

        result.expect("VM failed to run");
        VMRunner { vm }
    }
}

pub struct VMRunner {
    vm: VM,
}

impl VMRunner {
    #[track_caller]
    pub fn expect_stack_size(self, expected: usize) -> Self {
        assert_eq!(self.vm.stack_size(), expected);
        self
    }

    #[track_caller]
    pub fn expect_stack_top_is_int(mut self, expected: i64) -> Self {
        let obj = self.vm.stack_pop().expect("Failed popping from the stack!");
        let StackValue::Integer(value) = obj else {
            panic!("TOS is not an Integer!");
        };
        assert_eq!(value, expected, "Integer value mismatch on stack top!");
        self
    }

    #[track_caller]
    pub fn expect_stack_top_is_bool(mut self, expected: bool) -> Self {
        let obj = self.vm.stack_pop().expect("Failed popping from the stack!");
        let StackValue::Boolean(value) = obj else {
            panic!("TOS is not a Boolean!");
        };
        assert_eq!(value, expected, "Boolean value mismatch on stack top!");
        self
    }

    pub fn into_vm(self) -> VM {
        self.vm
    }
}

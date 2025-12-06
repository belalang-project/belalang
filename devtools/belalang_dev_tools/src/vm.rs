use belalang_bytecode::{
    Bytecode,
    Constant,
};
use belalang_vm::{
    VM,
    VMIO,
    stack::StackValue,
};

use crate::buf::SharedBuffer;

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
        let out_stream = SharedBuffer::default();
        let io = VMIO::new(Box::new(out_stream.clone()));
        let mut vm = VM::with_io(io);

        let result = vm.run(Bytecode {
            instructions: self.instructions,
            constants: self.constants,
        });

        result.expect("VM failed to run");
        VMRunner { out_stream, vm }
    }
}

pub struct VMRunner {
    out_stream: SharedBuffer,
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

    #[track_caller]
    pub fn expect_stdout(self, expected: String) -> Self {
        assert_eq!(expected, self.out_stream.get_string());
        self
    }

    pub fn into_vm(self) -> VM {
        self.vm
    }
}

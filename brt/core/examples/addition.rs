use belalang_dev_tools::{
    IntoInstructionBytes,
    instructions,
};
use brt_core::{
    bytecode::{
        Bytecode,
        Constant,
        opcode,
    },
    vm::VM,
};

fn main() {
    let constants = vec![Constant::Integer(10), Constant::Integer(100)];
    let instructions = instructions![opcode::constant(0), opcode::constant(1), opcode::ADD, opcode::PRINT];

    let mut vm = VM::default();
    vm.run(Bytecode {
        instructions,
        constants,
    })
    .unwrap();
}

use belalang_dev_tools::{
    IntoInstructionBytes,
    instructions,
};
use belalang_vm::VM;
use belalang_bytecode::{
    Bytecode,
    Constant,
    opcode,
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

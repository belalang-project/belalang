use belalang_dev_tools::{
    IntoInstructionBytes,
    instructions,
};
use belvm::VM;
use belvm_bytecode::{
    Bytecode,
    Constant,
    opcode,
};

fn main() {
    let s = String::from("Hello, World!");
    let constants = vec![Constant::String(s)];
    let instructions = instructions![opcode::constant(0), opcode::PRINT];

    let mut vm = VM::default();
    vm.run(Bytecode {
        instructions,
        constants,
    })
    .unwrap();
}

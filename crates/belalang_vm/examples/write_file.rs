use belalang_bytecode::{
    Bytecode,
    Constant,
    opcode,
};
use belalang_dev_tools::{
    IntoInstructionBytes,
    instructions,
};
use belalang_vm::VM;

fn main() {
    let s = String::from("Hello, World!");
    let f = String::from("test.txt");
    let constants = vec![Constant::String(s), Constant::String(f)];
    let instructions = instructions![opcode::constant(0), opcode::constant(1), opcode::FS_WRITE];

    let mut vm = VM::default();
    vm.run(Bytecode {
        instructions,
        constants,
    })
    .unwrap();
}

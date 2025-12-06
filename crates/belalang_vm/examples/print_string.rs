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
    let constants = vec![Constant::String(s)];
    let instructions = instructions![opcode::constant(0), opcode::PRINT];

    let mut vm = VM::default();
    vm.run(Bytecode {
        instructions,
        constants,
    })
    .unwrap();

    vm.collect_garbage();
}

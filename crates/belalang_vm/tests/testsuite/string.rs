use belalang_bytecode::{
    Constant,
    opcode,
};
use belalang_dev_tools::{
    IntoInstructionBytes,
    instructions,
};

#[test]
fn hello_world() {
    let constants = vec![Constant::String(String::from("Hello, World"))];

    let instructions = instructions![opcode::constant(0), opcode::PRINT];

    belalang_dev_tools::VMBuilder::default()
        .with_instructions(instructions)
        .with_constants(constants)
        .run_ok()
        .expect_stdout("Hello, World\n".to_string());
}

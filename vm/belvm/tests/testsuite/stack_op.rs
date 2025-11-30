use belalang_dev_tools::{
    IntoInstructionBytes,
    instructions,
};
use belalang_bytecode::{
    Constant,
    opcode,
};

#[test]
fn pop() {
    let constants = vec![Constant::Integer(12), Constant::Integer(5)];

    let instructions = instructions![opcode::constant(0), opcode::constant(1), opcode::POP,];

    belalang_dev_tools::VMBuilder::default()
        .with_instructions(instructions)
        .with_constants(constants)
        .run_ok()
        .expect_stack_size(1)
        .expect_stack_top_is_int(12);
}

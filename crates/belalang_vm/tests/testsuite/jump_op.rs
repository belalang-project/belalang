use belalang_bytecode::opcode;
use belalang_dev_tools::{
    IntoInstructionBytes,
    instructions,
};

#[test]
fn jump() {
    let constants = Vec::new();

    let instructions = instructions![opcode::jump(1), opcode::TRUE, opcode::FALSE,];

    belalang_dev_tools::VMBuilder::default()
        .with_instructions(instructions)
        .with_constants(constants)
        .run_ok()
        .expect_stack_size(1)
        .expect_stack_top_is_bool(false);
}

#[test]
fn jump_if_false_op() {
    let constants = Vec::new();

    let instructions = instructions![opcode::TRUE, opcode::jump_if_false(1), opcode::TRUE, opcode::FALSE,];

    belalang_dev_tools::VMBuilder::default()
        .with_instructions(instructions)
        .with_constants(constants)
        .run_ok()
        .expect_stack_size(2)
        .expect_stack_top_is_bool(false)
        .expect_stack_top_is_bool(true);
}

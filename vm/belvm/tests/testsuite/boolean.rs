use beltools_tests::IntoInstructionBytes;
use beltools_tests::instructions;
use belvm_bytecode::Constant;
use belvm_bytecode::opcode;

#[track_caller]
fn test_comparison_op(a: bool, b: bool, op: u8, c: bool) {
    let constants = vec![Constant::Boolean(a), Constant::Boolean(b)];

    let instructions = instructions![opcode::constant(0), opcode::constant(1), op,];

    beltools_tests::VMBuilder::default()
        .with_instructions(instructions)
        .with_constants(constants)
        .run_ok()
        .expect_stack_size(1)
        .expect_stack_top_is_bool(c);
}

#[test]
fn comparison_op_equal() {
    test_comparison_op(true, true, opcode::EQUAL, true);
}

#[test]
fn comparison_op_not_equal() {
    test_comparison_op(true, false, opcode::NOT_EQUAL, true);
}

#[track_caller]
fn test_logical_op(a: bool, b: bool, op: u8, c: bool) {
    let constants = vec![Constant::Boolean(a), Constant::Boolean(b)];

    let instructions = instructions![opcode::constant(0), opcode::constant(1), op,];

    beltools_tests::VMBuilder::default()
        .with_instructions(instructions)
        .with_constants(constants)
        .run_ok()
        .expect_stack_size(1)
        .expect_stack_top_is_bool(c);
}

#[test]
fn logical_op_and() {
    test_logical_op(true, false, opcode::AND, false);
}

#[test]
fn logical_op_or() {
    test_logical_op(true, false, opcode::OR, true);
}

#[test]
fn bang() {
    let constants = Vec::new();

    let instructions = instructions![opcode::TRUE, opcode::BANG,];

    beltools_tests::VMBuilder::default()
        .with_instructions(instructions)
        .with_constants(constants)
        .run_ok()
        .expect_stack_size(1)
        .expect_stack_top_is_bool(false);
}

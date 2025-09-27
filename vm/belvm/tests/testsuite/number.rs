use beltools_tests::IntoInstructionBytes;
use beltools_tests::instructions;
use belvm_bytecode::Constant;
use belvm_bytecode::opcode;

#[track_caller]
fn test_arithmetic_op(a: i64, b: i64, op: u8, c: i64) {
    let constants = vec![Constant::Integer(a), Constant::Integer(b)];

    let instructions = instructions![opcode::constant(0), opcode::constant(1), op,];

    beltools_tests::VMBuilder::default()
        .with_instructions(instructions)
        .with_constants(constants)
        .run_ok()
        .expect_stack_size(1)
        .expect_stack_top_is_int(c);
}

#[test]
fn arithmetic_op_addition() {
    test_arithmetic_op(12, 5, opcode::ADD, 17);
}

#[test]
fn arithmetic_op_subtraction() {
    test_arithmetic_op(12, 5, opcode::SUB, 7);
}

#[test]
fn arithmetic_op_multiplication() {
    test_arithmetic_op(12, 5, opcode::MUL, 60);
}

#[test]
fn arithmetic_op_division() {
    test_arithmetic_op(12, 5, opcode::DIV, 2);
}

#[test]
fn arithmetic_op_modulo() {
    test_arithmetic_op(12, 5, opcode::MOD, 2);
}

#[track_caller]
fn test_comparison_op(a: i64, b: i64, op: u8, c: bool) {
    let constants = vec![Constant::Integer(a), Constant::Integer(b)];

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
    test_comparison_op(12, 12, opcode::EQUAL, true);
}

#[test]
fn comparison_op_not_equal() {
    test_comparison_op(12, 12, opcode::NOT_EQUAL, false);
}

#[test]
fn comparison_op_less_than() {
    test_comparison_op(12, 13, opcode::LESS_THAN, true);
}

#[test]
fn comparison_op_less_than_equal() {
    test_comparison_op(12, 12, opcode::LESS_THAN_EQUAL, true);
}

#[track_caller]
fn test_bitwise_op(a: i64, b: i64, op: u8, c: i64) {
    let constants = vec![Constant::Integer(a), Constant::Integer(b)];

    let instructions = instructions![opcode::constant(0), opcode::constant(1), op,];

    beltools_tests::VMBuilder::default()
        .with_instructions(instructions)
        .with_constants(constants)
        .run_ok()
        .expect_stack_size(1)
        .expect_stack_top_is_int(c);
}

#[test]
fn bitwise_op_bit_and() {
    test_bitwise_op(12, 1, opcode::BIT_AND, 0);
}

#[test]
fn bitwise_op_bit_or() {
    test_bitwise_op(12, 1, opcode::BIT_OR, 13);
}

#[test]
fn bitwise_op_bit_xor() {
    test_bitwise_op(12, 1, opcode::BIT_OR, 13);
}

#[test]
fn bitwise_op_bit_sl() {
    test_bitwise_op(12, 1, opcode::BIT_SL, 24);
}

#[test]
fn bitwise_op_bit_sr() {
    test_bitwise_op(12, 1, opcode::BIT_SR, 6);
}

#[test]
fn minus() {
    let constants = vec![Constant::Integer(12)];

    let instructions = instructions![opcode::constant(0), opcode::MINUS,];

    beltools_tests::VMBuilder::default()
        .with_instructions(instructions)
        .with_constants(constants)
        .run_ok()
        .expect_stack_size(1)
        .expect_stack_top_is_int(-12);
}

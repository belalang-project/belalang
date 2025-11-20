use belalang_ast as ast;
use belalang_lexer::InfixKind;

use crate::{
    common::*,
    *,
};

#[test]
fn block() {
    let program = test_parse("fn() { 12; 14; 1 + 2; };");

    assert_eq!(program.statements.len(), 1);

    let expr = as_variant!(&program.statements[0], ast::Statement::Expression);

    let f = as_variant!(&expr.expression, ast::Expression::Function);

    assert_eq!(f.body.statements.len(), 3);

    // first statement
    let expr_0 = as_variant!(&f.body.statements[0], ast::Statement::Expression);

    let int_0 = as_variant!(&expr_0.expression, ast::Expression::Integer);

    assert_eq!(int_0.value, 12);

    // second statement
    let expr_1 = as_variant!(&f.body.statements[1], ast::Statement::Expression);

    let int_1 = as_variant!(&expr_1.expression, ast::Expression::Integer);

    assert_eq!(int_1.value, 14);

    // third statement
    let expr_2 = as_variant!(&f.body.statements[2], ast::Statement::Expression);

    expr_variant!(&expr_2.expression, Infix => (
        ast::Expression::Integer = 1,
        InfixKind::Add,
        ast::Expression::Integer = 2
    ));
}

#[test]
fn r#return() {
    let program = test_parse("return 12;");

    assert_eq!(program.statements.len(), 1);

    let ret = as_variant!(&program.statements[0], ast::Statement::Return);

    let val = as_variant!(&ret.return_value, ast::Expression::Integer);

    assert_eq!(val.value, 12);
}

#[test]
fn r#while() {
    let program = test_parse("while true { 12; }");

    assert_eq!(program.statements.len(), 1);

    let stmt = as_variant!(&program.statements[0], ast::Statement::While);

    expr_variant!(&*stmt.condition, ast::Expression::Boolean = true);

    assert_eq!(stmt.block.statements.len(), 1);

    let expr_0 = as_variant!(&stmt.block.statements[0], ast::Statement::Expression);

    expr_variant!(&expr_0.expression, ast::Expression::Integer = 12);
}

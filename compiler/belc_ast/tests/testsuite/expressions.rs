#![allow(clippy::bool_assert_comparison)]

use belc_ast as ast;
use belc_lexer::{AssignmentKind, InfixKind, PrefixKind};

use crate::common::*;
use crate::*;

#[track_caller]
fn test_booleans(input: &str, value: bool) {
    let program = test_parse(input);

    assert_eq!(program.statements.len(), 1);

    let expr = as_variant!(&program.statements[0], ast::Statement::Expression);

    let bool_expr = as_variant!(&expr.expression, ast::Expression::Boolean);

    assert_eq!(bool_expr.value, value);
}

#[test]
fn booleans_true() {
    test_booleans("true;", true);
}

#[test]
fn booleans_false() {
    test_booleans("false;", false);
}

#[test]
fn call() {
    let program = test_parse("add(1, 2 * 3, 4 + 5);");

    assert_eq!(program.statements.len(), 1);

    let stmt = as_variant!(&program.statements[0], ast::Statement::Expression);

    let expr = as_variant!(&stmt.expression, ast::Expression::Call);

    expr_variant!(&*expr.function, ast::Expression::Identifier = "add");

    assert_eq!(expr.args.len(), 3);
    expr_variant!(&expr.args[0], ast::Expression::Integer = 1);
    expr_variant!(
        &expr.args[1], Infix => (
            ast::Expression::Integer = 2,
            InfixKind::Mul,
            ast::Expression::Integer = 3
        )
    );
    expr_variant!(
        &expr.args[2], Infix => (
            ast::Expression::Integer = 4,
            InfixKind::Add,
            ast::Expression::Integer = 5
        )
    );
}

#[test]
fn call_with_function_literal() {
    let program = test_parse("fn(x, y) { x + y }(2, 3);");

    assert_eq!(program.statements.len(), 1);

    let stmt = as_variant!(&program.statements[0], ast::Statement::Expression);

    let expr = as_variant!(&stmt.expression, ast::Expression::Call);

    assert_eq!(expr.args.len(), 2);
    expr_variant!(&expr.args[0], ast::Expression::Integer = 2);
    expr_variant!(&expr.args[1], ast::Expression::Integer = 3);

    let function = as_variant!(&*expr.function, ast::Expression::Function);

    assert_eq!(function.params.len(), 2);
    ident_has_name!(function.params[0], "x");
    ident_has_name!(function.params[1], "y");

    assert_eq!(function.body.statements.len(), 1);

    let body_stmt = as_variant!(&function.body.statements[0], ast::Statement::Expression);

    expr_variant!(
        &body_stmt.expression, Infix => (
            ast::Expression::Identifier = "x",
            InfixKind::Add,
            ast::Expression::Identifier = "y"
        )
    );
}

#[test]
fn array() {
    let program = test_parse("[1, 2, 3];");

    assert_eq!(program.statements.len(), 1);

    let stmt = as_variant!(&program.statements[0], ast::Statement::Expression);
    let array = as_variant!(&stmt.expression, ast::Expression::Array);

    assert_eq!(array.elements.len(), 3);

    expr_variant!(&array.elements[0], ast::Expression::Integer = 1);
    expr_variant!(&array.elements[1], ast::Expression::Integer = 2);
    expr_variant!(&array.elements[2], ast::Expression::Integer = 3);
}

#[test]
fn array_indexing() {
    let program = test_parse("arr[1];");

    assert_eq!(program.statements.len(), 1);

    let stmt = as_variant!(&program.statements[0], ast::Statement::Expression);
    let index = as_variant!(&stmt.expression, ast::Expression::Index);

    expr_variant!(&*index.index, ast::Expression::Integer = 1);

    let ident = as_variant!(&*index.left, ast::Expression::Identifier);
    ident_has_name!(ident, "arr");
}

#[test]
fn function() {
    let program = test_parse("fn(x, y) { x + y; };");

    assert_eq!(program.statements.len(), 1);

    let stmt = as_variant!(&program.statements[0], ast::Statement::Expression);

    let function = as_variant!(&stmt.expression, ast::Expression::Function);

    assert_eq!(function.params.len(), 2);

    ident_has_name!(function.params[0], "x");
    ident_has_name!(function.params[1], "y");

    assert_eq!(function.body.statements.len(), 1);

    let body_stmt = as_variant!(&function.body.statements[0], ast::Statement::Expression);

    expr_variant!(
        &body_stmt.expression, Infix => (
            ast::Expression::Identifier = "x",
            InfixKind::Add,
            ast::Expression::Identifier = "y"
        )
    );
}

#[track_caller]
fn test_function_params(input: &str, output: Vec<&'static str>) {
    let program = test_parse(input);

    let stmt = as_variant!(&program.statements[0], ast::Statement::Expression);

    let function = as_variant!(&stmt.expression, ast::Expression::Function);

    assert_eq!(
        function
            .params
            .iter()
            .map(|param| param.value.clone())
            .collect::<Vec<_>>(),
        output
    )
}

#[test]
fn function_params_no_params() {
    test_function_params("fn() {};", Vec::new());
}

#[test]
fn function_params_one_params() {
    test_function_params("fn(x) {};", vec!["x"]);
}

#[test]
fn function_params_two_params() {
    test_function_params("fn(x, y) {};", vec!["x", "y"]);
}

#[test]
fn function_params_three_params() {
    test_function_params("fn(x, y, z) {};", vec!["x", "y", "z"]);
}

#[test]
fn identifier() {
    let program = test_parse("name;");

    assert_eq!(program.statements.len(), 1);

    let expr = as_variant!(&program.statements[0], ast::Statement::Expression);

    let ident = as_variant!(&expr.expression, ast::Expression::Identifier);

    ident_has_name!(ident, "name");
}

#[test]
fn if_without_else() {
    let program = test_parse("if x < y { x }");

    assert_eq!(program.statements.len(), 1);

    let stmt = as_variant!(&program.statements[0], ast::Statement::Expression);

    let if_expr = as_variant!(&stmt.expression, ast::Expression::If);

    // testing the condition
    expr_variant!(
        &*if_expr.condition, Infix => (
            ast::Expression::Identifier = "x",
            InfixKind::Lt,
            ast::Expression::Identifier = "y"
        )
    );

    // testing the consequence block
    let stmt_1 = as_variant!(&if_expr.consequence.statements[0], ast::Statement::Expression);
    expr_variant!(&stmt_1.expression, ast::Expression::Identifier = "x");

    // testing the alternative block
    assert!(if_expr.alternative.is_none());
}

#[test]
fn if_with_else() {
    let program = test_parse("if x < y { x } else { y }");

    assert_eq!(program.statements.len(), 1);

    let stmt = as_variant!(&program.statements[0], ast::Statement::Expression);

    let if_expr = as_variant!(&stmt.expression, ast::Expression::If);

    // testing the condition
    expr_variant!(
        &*if_expr.condition, Infix => (
            ast::Expression::Identifier = "x",
            InfixKind::Lt,
            ast::Expression::Identifier = "y"
        )
    );

    // testing the consequence block
    let stmt_0 = as_variant!(&if_expr.consequence.statements[0], ast::Statement::Expression);
    expr_variant!(&stmt_0.expression, ast::Expression::Identifier = "x");

    // testing the alternative block
    let alt = if_expr.alternative.clone().unwrap();
    let alt = as_variant!(*alt, ast::Expression::Block);

    let stmt_0 = as_variant!(&alt.statements[0], ast::Statement::Expression);
    expr_variant!(&stmt_0.expression, ast::Expression::Identifier = "y");
}

#[test]
fn if_with_multiple_statements() {
    let program = test_parse("if x < y { a := 10; x }");

    assert_eq!(program.statements.len(), 1);

    let stmt = as_variant!(&program.statements[0], ast::Statement::Expression);

    let if_expr = as_variant!(&stmt.expression, ast::Expression::If);

    expr_variant!(
        if_expr.condition.as_ref(), Infix => (
            ast::Expression::Identifier = "x",
            InfixKind::Lt,
            ast::Expression::Identifier = "y"
        )
    );

    assert!(if_expr.alternative.is_none());

    // testing consequence block
    let stmt_0 = as_variant!(&if_expr.consequence.statements[0], ast::Statement::Expression);
    let stmt_0 = as_variant!(&stmt_0.expression, ast::Expression::Var);
    ident_has_name!(stmt_0.name, "a");
    expr_variant!(&*stmt_0.value, ast::Expression::Integer = 10);

    let stmt_1 = as_variant!(&if_expr.consequence.statements[1], ast::Statement::Expression);
    expr_variant!(&stmt_1.expression, ast::Expression::Identifier = "x");
}

#[test]
fn infix() {
    let program = test_parse("1 + 2;");

    assert_eq!(program.statements.len(), 1);

    let expr = as_variant!(&program.statements[0], ast::Statement::Expression);

    expr_variant!(&expr.expression, Infix => (
        ast::Expression::Integer = 1,
        InfixKind::Add,
        ast::Expression::Integer = 2
    ));
}

#[test]
fn infix_var_declare() {
    let program = test_parse("x := 5;");

    assert_eq!(program.statements.len(), 1);

    let stmt = as_variant!(&program.statements[0], ast::Statement::Expression);
    let expr = as_variant!(&stmt.expression, ast::Expression::Var);

    assert_eq!(expr.kind, AssignmentKind::ColonAssign);
    ident_has_name!(expr.name, "x");

    expr_variant!(&*expr.value, ast::Expression::Integer = 5);
}

#[test]
fn infix_var_assign() {
    let program = test_parse("x = 5;");

    assert_eq!(program.statements.len(), 1);

    let stmt = as_variant!(&program.statements[0], ast::Statement::Expression);
    let expr = as_variant!(&stmt.expression, ast::Expression::Var);

    assert_eq!(expr.kind, AssignmentKind::Assign);
    ident_has_name!(expr.name, "x");

    expr_variant!(&*expr.value, ast::Expression::Integer = 5);
}

#[test]
#[should_panic]
fn infix_on_invalid_lhs() {
    test_parse("1 = 5;");
    test_parse("x + 2 = 5;");
    test_parse("x + 2 += 5;");
}

#[test]
#[rustfmt::skip]
fn infix_operator_precedence() {
    test_parse_to_string("a * b + c;", "((a * b) + c);");
    test_parse_to_string("!-a;", "(!(-a));");
    test_parse_to_string("a + b + c;", "((a + b) + c);");
    test_parse_to_string("a + b - c;", "((a + b) - c);");
    test_parse_to_string("a * b * c;", "((a * b) * c);");
    test_parse_to_string("a * b / c;", "((a * b) / c);");
    test_parse_to_string("a + b / c;", "(a + (b / c));");
    test_parse_to_string("a + b * c + d / e - f;", "(((a + (b * c)) + (d / e)) - f);");
    test_parse_to_string("3 + 4; -5 * 5;", "(3 + 4);((-5) * 5);");
    test_parse_to_string("5 > 4 == 3 < 4;", "((5 > 4) == (3 < 4));");
    test_parse_to_string("5 < 4 != 3 > 4;", "((5 < 4) != (3 > 4));");
    test_parse_to_string("3 + 4 * 5 == 3 * 1 + 4 * 5;", "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)));");
    test_parse_to_string("3 + 4 * 5 == 3 * 1 + 4 * 5;", "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)));");
    test_parse_to_string("true;", "true;");
    test_parse_to_string("false;", "false;");
    test_parse_to_string("3 > 5 == false;", "((3 > 5) == false);");
    test_parse_to_string("3 < 5 == true;", "((3 < 5) == true);");
    test_parse_to_string("1 + (2 + 3) + 4;", "((1 + (2 + 3)) + 4);");
    test_parse_to_string("(5 + 5) * 2;", "((5 + 5) * 2);");
    test_parse_to_string("2 / (5 + 5);", "(2 / (5 + 5));");
    test_parse_to_string("-(5 + 5);", "(-(5 + 5));");
    test_parse_to_string("!(true == true);", "(!(true == true));");
    test_parse_to_string("a + add(b * c) + d;", "((a + add((b * c))) + d);");
    test_parse_to_string("add(a, b, 1, 2 * 3, 4 + 5, add(6, 7 * 8));", "add(a, b, 1, (2 * 3), (4 + 5), add(6, (7 * 8)));");
    test_parse_to_string("add(a + b + c * d / f + g);", "add((((a + b) + ((c * d) / f)) + g));");
}

#[test]
fn integer() {
    let program = test_parse("12;");

    assert_eq!(program.statements.len(), 1);

    let expr = as_variant!(&program.statements[0], ast::Statement::Expression);

    let int = as_variant!(&expr.expression, ast::Expression::Integer);

    assert_eq!(int.value, 12);
}

#[test]
fn prefix_minus_number() {
    let program = test_parse("-12;");

    assert_eq!(program.statements.len(), 1);

    let expr = as_variant!(&program.statements[0], ast::Statement::Expression);

    expr_variant!(&expr.expression, Prefix => (
        PrefixKind::Sub,
        ast::Expression::Integer = 12
    ));
}

#[test]
fn prefix_bang_number() {
    let program = test_parse("!12;");

    assert_eq!(program.statements.len(), 1);

    let expr = as_variant!(&program.statements[0], ast::Statement::Expression);

    expr_variant!(&expr.expression, Prefix => (
        PrefixKind::Not,
        ast::Expression::Integer = 12
    ));
}

#[test]
fn prefix_minus_boolean() {
    let program = test_parse("-true;");

    assert_eq!(program.statements.len(), 1);

    let expr = as_variant!(&program.statements[0], ast::Statement::Expression);

    expr_variant!(&expr.expression, Prefix => (
        PrefixKind::Sub,
        ast::Expression::Boolean = true
    ));
}

#[test]
fn prefix_bang_boolean() {
    let program = test_parse("!true;");

    assert_eq!(program.statements.len(), 1);

    let expr = as_variant!(&program.statements[0], ast::Statement::Expression);

    expr_variant!(&expr.expression, Prefix => (
        PrefixKind::Not,
        ast::Expression::Boolean = true
    ));
}

#[test]
fn string() {
    let program = test_parse("\"Hello, World!\";");

    assert_eq!(program.statements.len(), 1);

    let expr = as_variant!(&program.statements[0], ast::Statement::Expression);

    expr_variant!(&expr.expression, ast::Expression::String = "Hello, World!");
}

use super::{
    ArrayLiteral,
    BlockExpression,
    BooleanExpression,
    CallExpression,
    Expression,
    ExpressionStatement,
    FloatLiteral,
    FunctionLiteral,
    Identifier,
    IfExpression,
    IndexExpression,
    InfixExpression,
    IntegerLiteral,
    NullLiteral,
    PrefixExpression,
    Program,
    ReturnStatement,
    Statement,
    StringLiteral,
    VarDeclExpression,
    VarExpression,
    WhileStatement,
};

pub trait Visitor {
    fn visit_program(&mut self, program: &Program) {
        self.walk_program(program);
    }

    fn visit_statement(&mut self, stmt: &Statement) {
        self.walk_statement(stmt);
    }

    fn visit_expression(&mut self, expr: &Expression) {
        self.walk_expression(expr);
    }

    fn visit_boolean(&mut self, _node: &BooleanExpression) {}

    fn visit_integer(&mut self, _node: &IntegerLiteral) {}

    fn visit_float(&mut self, _node: &FloatLiteral) {}

    fn visit_string(&mut self, _node: &StringLiteral) {}

    fn visit_null(&mut self, _node: &NullLiteral) {}

    fn visit_identifier(&mut self, _node: &Identifier) {}

    fn visit_array(&mut self, node: &ArrayLiteral) {
        self.walk_array(node);
    }

    fn visit_var(&mut self, node: &VarExpression) {
        self.walk_var(node);
    }

    fn visit_var_decl(&mut self, node: &VarDeclExpression) {
        self.walk_var_decl(node);
    }

    fn visit_call(&mut self, node: &CallExpression) {
        self.walk_call(node);
    }

    fn visit_index(&mut self, node: &IndexExpression) {
        self.walk_index(node);
    }

    fn visit_function(&mut self, node: &FunctionLiteral) {
        self.walk_function(node);
    }

    fn visit_if(&mut self, node: &IfExpression) {
        self.walk_if(node);
    }

    fn visit_infix(&mut self, node: &InfixExpression) {
        self.walk_infix(node);
    }

    fn visit_prefix(&mut self, node: &PrefixExpression) {
        self.walk_prefix(node);
    }

    fn visit_block(&mut self, node: &BlockExpression) {
        self.walk_block(node);
    }

    fn visit_expression_statement(&mut self, node: &ExpressionStatement) {
        self.walk_expression_statement(node);
    }

    fn visit_return_statement(&mut self, node: &ReturnStatement) {
        self.walk_return_statement(node);
    }

    fn visit_while_statement(&mut self, node: &WhileStatement) {
        self.walk_while_statement(node);
    }

    fn walk_program(&mut self, program: &Program) {
        for stmt in &program.statements {
            self.visit_statement(stmt);
        }
    }

    fn walk_statement(&mut self, stmt: &Statement) {
        match stmt {
            Statement::Expression(v) => self.visit_expression_statement(v),
            Statement::Return(v) => self.visit_return_statement(v),
            Statement::While(v) => self.visit_while_statement(v),
        }
    }

    fn walk_expression(&mut self, expr: &Expression) {
        match expr {
            Expression::Boolean(v) => self.visit_boolean(v),
            Expression::Integer(v) => self.visit_integer(v),
            Expression::Float(v) => self.visit_float(v),
            Expression::String(v) => self.visit_string(v),
            Expression::Null(v) => self.visit_null(v),
            Expression::Array(v) => self.visit_array(v),
            Expression::Var(v) => self.visit_var(v),
            Expression::VarDecl(v) => self.visit_var_decl(v),
            Expression::Call(v) => self.visit_call(v),
            Expression::Index(v) => self.visit_index(v),
            Expression::Function(v) => self.visit_function(v),
            Expression::Identifier(v) => self.visit_identifier(v),
            Expression::If(v) => self.visit_if(v),
            Expression::Infix(v) => self.visit_infix(v),
            Expression::Prefix(v) => self.visit_prefix(v),
            Expression::Block(v) => self.visit_block(v),
        }
    }

    fn walk_array(&mut self, node: &ArrayLiteral) {
        for elem in &node.elements {
            self.visit_expression(elem);
        }
    }

    fn walk_var(&mut self, node: &VarExpression) {
        self.visit_identifier(&node.name);
        self.visit_expression(&node.value);
    }

    fn walk_var_decl(&mut self, node: &VarDeclExpression) {
        self.visit_identifier(&node.name);
        if let Some(value) = &node.value {
            self.visit_expression(value);
        }
    }

    fn walk_call(&mut self, node: &CallExpression) {
        self.visit_expression(&node.function);
        for arg in &node.args {
            self.visit_expression(arg);
        }
    }

    fn walk_index(&mut self, node: &IndexExpression) {
        self.visit_expression(&node.left);
        self.visit_expression(&node.index);
    }

    fn walk_function(&mut self, node: &FunctionLiteral) {
        for param in &node.params {
            self.visit_identifier(param);
        }
        self.visit_block(&node.body);
    }

    fn walk_if(&mut self, node: &IfExpression) {
        self.visit_expression(&node.condition);
        self.visit_block(&node.consequence);
        if let Some(alt) = &node.alternative {
            self.visit_expression(alt);
        }
    }

    fn walk_infix(&mut self, node: &InfixExpression) {
        self.visit_expression(&node.left);
        self.visit_expression(&node.right);
    }

    fn walk_prefix(&mut self, node: &PrefixExpression) {
        self.visit_expression(&node.right);
    }

    fn walk_block(&mut self, node: &BlockExpression) {
        for stmt in &node.statements {
            self.visit_statement(stmt);
        }
    }

    fn walk_expression_statement(&mut self, node: &ExpressionStatement) {
        self.visit_expression(&node.expression);
    }

    fn walk_return_statement(&mut self, node: &ReturnStatement) {
        self.visit_expression(&node.return_value);
    }

    fn walk_while_statement(&mut self, node: &WhileStatement) {
        self.visit_expression(&node.condition);
        self.visit_block(&node.block);
    }
}

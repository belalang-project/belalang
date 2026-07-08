use super::{
    ArrayLiteral,
    BlockExpression,
    BooleanExpression,
    BreakStatement,
    CallExpression,
    ContinueStatement,
    Expression,
    ExpressionKind,
    ExpressionStatement,
    FloatLiteral,
    FunctionLiteral,
    Identifier,
    IfExpression,
    IndexExpression,
    InfixExpression,
    IntegerLiteral,
    MemberAccessExpression,
    NullLiteral,
    PrefixExpression,
    Program,
    ReturnStatement,
    Statement,
    StatementKind,
    StringLiteral,
    StructDeclStatement,
    StructLiteral,
    VarDeclStatement,
    VarExpression,
    WhileStatement,
};

pub trait Visitor<'ast> {
    fn visit_program(&mut self, program: &Program<'ast>) {
        self.walk_program(program);
    }

    fn visit_statement(&mut self, stmt: &Statement<'ast>) {
        self.walk_statement(stmt);
    }

    fn visit_expression(&mut self, expr: &Expression<'ast>) {
        self.walk_expression(expr);
    }

    fn visit_boolean(&mut self, _node: &BooleanExpression) {}

    fn visit_integer(&mut self, _node: &IntegerLiteral) {}

    fn visit_float(&mut self, _node: &FloatLiteral) {}

    fn visit_string(&mut self, _node: &StringLiteral) {}

    fn visit_null(&mut self, _node: &NullLiteral) {}

    fn visit_identifier(&mut self, _node: &Identifier) {}

    fn visit_array(&mut self, node: &ArrayLiteral<'ast>) {
        self.walk_array(node);
    }

    fn visit_var(&mut self, node: &VarExpression<'ast>) {
        self.walk_var(node);
    }

    fn visit_call(&mut self, node: &CallExpression<'ast>) {
        self.walk_call(node);
    }

    fn visit_index(&mut self, node: &IndexExpression<'ast>) {
        self.walk_index(node);
    }

    fn visit_member_access(&mut self, node: &MemberAccessExpression<'ast>) {
        self.walk_member_access(node);
    }

    fn visit_function(&mut self, node: &FunctionLiteral<'ast>) {
        self.walk_function(node);
    }

    fn visit_if(&mut self, node: &IfExpression<'ast>) {
        self.walk_if(node);
    }

    fn visit_infix(&mut self, node: &InfixExpression<'ast>) {
        self.walk_infix(node);
    }

    fn visit_prefix(&mut self, node: &PrefixExpression<'ast>) {
        self.walk_prefix(node);
    }

    fn visit_block(&mut self, node: &BlockExpression<'ast>) {
        self.walk_block(node);
    }

    fn visit_struct_literal(&mut self, node: &StructLiteral<'ast>) {
        self.walk_struct_literal(node);
    }

    fn visit_expression_statement(&mut self, node: &ExpressionStatement<'ast>) {
        self.walk_expression_statement(node);
    }

    fn visit_return_statement(&mut self, node: &ReturnStatement<'ast>) {
        self.walk_return_statement(node);
    }

    fn visit_while_statement(&mut self, node: &WhileStatement<'ast>) {
        self.walk_while_statement(node);
    }

    fn visit_break_statement(&mut self, node: &BreakStatement) {}

    fn visit_continue_statement(&mut self, node: &ContinueStatement) {}

    fn visit_var_decl_statement(&mut self, node: &VarDeclStatement<'ast>) {
        self.walk_var_decl_statement(node);
    }

    fn visit_struct_decl_statement(&mut self, node: &StructDeclStatement<'ast>) {
        self.walk_struct_decl_statement(node);
    }

    fn walk_program(&mut self, program: &Program<'ast>) {
        for stmt in program.statements {
            self.visit_statement(stmt);
        }
    }

    fn walk_statement(&mut self, stmt: &Statement<'ast>) {
        match &stmt.kind {
            StatementKind::Expression(v) => self.visit_expression_statement(v),
            StatementKind::Return(v) => self.visit_return_statement(v),
            StatementKind::While(v) => self.visit_while_statement(v),
            StatementKind::VarDecl(v) => self.visit_var_decl_statement(v),
            StatementKind::StructDecl(v) => self.visit_struct_decl_statement(v),
            StatementKind::Break(v) => self.visit_break_statement(v),
            StatementKind::Continue(v) => self.visit_continue_statement(v),
        }
    }

    fn walk_expression(&mut self, expr: &Expression<'ast>) {
        match &expr.kind {
            ExpressionKind::Boolean(v) => self.visit_boolean(v),
            ExpressionKind::Integer(v) => self.visit_integer(v),
            ExpressionKind::Float(v) => self.visit_float(v),
            ExpressionKind::String(v) => self.visit_string(v),
            ExpressionKind::Null(v) => self.visit_null(v),
            ExpressionKind::Array(v) => self.visit_array(v),
            ExpressionKind::Var(v) => self.visit_var(v),
            ExpressionKind::Call(v) => self.visit_call(v),
            ExpressionKind::Index(v) => self.visit_index(v),
            ExpressionKind::Function(v) => self.visit_function(v),
            ExpressionKind::Identifier(v) => self.visit_identifier(v),
            ExpressionKind::If(v) => self.visit_if(v),
            ExpressionKind::Infix(v) => self.visit_infix(v),
            ExpressionKind::Prefix(v) => self.visit_prefix(v),
            ExpressionKind::Block(v) => self.visit_block(v),
            ExpressionKind::MemberAccess(v) => self.visit_member_access(v),
            ExpressionKind::StructLiteral(v) => self.visit_struct_literal(v),
        }
    }

    fn walk_array(&mut self, node: &ArrayLiteral<'ast>) {
        for elem in node.elements {
            self.visit_expression(elem);
        }
    }

    fn walk_var(&mut self, node: &VarExpression<'ast>) {
        self.visit_identifier(&node.name);
        self.visit_expression(node.value);
    }

    fn walk_call(&mut self, node: &CallExpression<'ast>) {
        self.visit_expression(node.function);
        for arg in node.args {
            self.visit_expression(arg);
        }
    }

    fn walk_index(&mut self, node: &IndexExpression<'ast>) {
        self.visit_expression(node.left);
        self.visit_expression(node.index);
    }

    fn walk_member_access(&mut self, node: &MemberAccessExpression<'ast>) {
        self.visit_expression(node.source);
        self.visit_identifier(&node.member);
    }

    fn walk_function(&mut self, node: &FunctionLiteral<'ast>) {
        for param in node.params {
            self.visit_var_decl_statement(param);
        }
        self.visit_block(&node.body);
    }

    fn walk_if(&mut self, node: &IfExpression<'ast>) {
        self.visit_expression(node.condition);
        self.visit_block(&node.consequence);
        if let Some(alt) = node.alternative {
            self.visit_expression(alt);
        }
    }

    fn walk_infix(&mut self, node: &InfixExpression<'ast>) {
        self.visit_expression(node.left);
        self.visit_expression(node.right);
    }

    fn walk_prefix(&mut self, node: &PrefixExpression<'ast>) {
        self.visit_expression(node.right);
    }

    fn walk_block(&mut self, node: &BlockExpression<'ast>) {
        for stmt in node.statements {
            self.visit_statement(stmt);
        }
    }

    fn walk_expression_statement(&mut self, node: &ExpressionStatement<'ast>) {
        self.visit_expression(&node.expression);
    }

    fn walk_return_statement(&mut self, node: &ReturnStatement<'ast>) {
        if let Some(ref return_value) = node.return_value {
            self.visit_expression(return_value);
        }
    }

    fn walk_while_statement(&mut self, node: &WhileStatement<'ast>) {
        self.visit_expression(&node.condition);
        self.visit_block(&node.block);
    }

    fn walk_var_decl_statement(&mut self, node: &VarDeclStatement<'ast>) {
        self.visit_identifier(&node.name);
        if let Some(value) = node.value {
            self.visit_expression(value);
        }
    }

    fn walk_struct_decl_statement(&mut self, node: &StructDeclStatement<'ast>) {
        self.visit_identifier(&node.name);
        for v in node.fields {
            self.visit_var_decl_statement(v);
        }
    }

    fn walk_struct_literal(&mut self, node: &StructLiteral<'ast>) {
        self.visit_identifier(&node.name);
        for field in node.fields {
            self.visit_var(field);
        }
    }
}

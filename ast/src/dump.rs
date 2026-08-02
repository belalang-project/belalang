use session::Session;

use super::{
    ArrayLiteral,
    BlockExpression,
    BooleanExpression,
    CallExpression,
    ExpressionStatement,
    FloatLiteral,
    FunctionLiteral,
    Identifier,
    IfExpression,
    ImportStatement,
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
    Visitor,
    WhileStatement,
};

pub struct ASTDumper<'sess> {
    session: &'sess Session,
    indent: usize,
}

impl<'sess> ASTDumper<'sess> {
    pub fn new(session: &'sess Session) -> Self {
        Self { indent: 0, session }
    }
}

impl<'ast> Visitor<'ast> for ASTDumper<'_> {
    fn visit_statement(&mut self, stmt: &Statement<'ast>) {
        match &stmt.kind {
            StatementKind::VarDecl(_) | StatementKind::StructDecl(_) => {
                println!("{:indent$}DeclStmt", "", indent = self.indent);
                self.indent += 2;
                self.walk_statement(stmt);
                self.indent -= 2;
            },
            _ => self.walk_statement(stmt),
        }
    }

    fn visit_program(&mut self, program: &Program<'ast>) {
        println!("{:indent$}Program", "", indent = self.indent);
        self.indent += 2;
        self.walk_program(program);
        self.indent -= 2;
    }

    fn visit_expression_statement(&mut self, node: &ExpressionStatement<'ast>) {
        println!("{:indent$}ExprStmt", "", indent = self.indent);
        self.indent += 2;
        self.visit_expression(&node.expression);
        self.indent -= 2;
    }

    fn visit_return_statement(&mut self, node: &ReturnStatement<'ast>) {
        println!("{:indent$}ReturnStmt", "", indent = self.indent);
        self.indent += 2;
        if let Some(return_value) = node.return_value {
            self.visit_expression(&return_value);
        }
        self.indent -= 2;
    }

    fn visit_while_statement(&mut self, node: &WhileStatement<'ast>) {
        println!("{:indent$}WhileStmt", "", indent = self.indent);
        self.indent += 2;
        self.visit_expression(&node.condition);
        self.visit_block(&node.block);
        self.indent -= 2;
    }

    fn visit_break_statement(&mut self, _node: &crate::BreakStatement) {
        println!("{:indent$}BreakStmt", "", indent = self.indent);
    }

    fn visit_continue_statement(&mut self, _node: &crate::ContinueStatement) {
        println!("{:indent$}ContinueStmt", "", indent = self.indent);
    }

    fn visit_import_statement(&mut self, node: &ImportStatement) {
        let module = self.session.lookup_string(node.module);
        println!("{:indent$}ImportStmt", "", indent = self.indent);
        println!("{:indent$}StringLitExpr \"{}\"", "", module, indent = self.indent + 2);
    }

    fn visit_var_decl_statement(&mut self, node: &VarDeclStatement<'ast>) {
        let name = self.session.lookup_string(node.name.value);
        print!("{:indent$}VarDecl {}", "", name, indent = self.indent);
        if let Some(ty) = node.explicit_ty {
            print!(" {}", self.session.lookup_string(ty));
        }
        println!();
        self.indent += 2;
        if let Some(value) = node.value {
            self.visit_expression(value);
        }
        self.indent -= 2;
    }

    fn visit_struct_decl_statement(&mut self, node: &StructDeclStatement<'ast>) {
        let name = self.session.lookup_string(node.name.value);
        println!("{:indent$}StructDecl {}", "", name, indent = self.indent);
        self.indent += 2;
        for field in node.fields {
            self.visit_var_decl_statement(field);
        }
        self.indent -= 2;
    }

    fn visit_boolean(&mut self, node: &BooleanExpression) {
        println!("{:indent$}BoolExpr {}", "", node.value, indent = self.indent);
    }

    fn visit_integer(&mut self, node: &IntegerLiteral) {
        println!("{:indent$}IntLitExpr {}", "", node.value, indent = self.indent);
    }

    fn visit_float(&mut self, node: &FloatLiteral) {
        println!("{:indent$}FloatLitExpr {}", "", node.value, indent = self.indent);
    }

    fn visit_string(&mut self, node: &StringLiteral) {
        let v = self.session.lookup_string(node.value);
        println!("{:indent$}StringLitExpr \"{}\"", "", v, indent = self.indent);
    }

    fn visit_null(&mut self, _node: &NullLiteral) {}

    fn visit_identifier(&mut self, node: &Identifier) {
        let v = self.session.lookup_string(node.value);
        println!("{:indent$}IdentifierExpr {}", "", v, indent = self.indent);
    }

    fn visit_infix(&mut self, node: &InfixExpression<'ast>) {
        println!("{:indent$}InfixExpr {:?}", "", node.operator, indent = self.indent);
        self.indent += 2;
        self.walk_infix(node);
        self.indent -= 2;
    }

    fn visit_prefix(&mut self, node: &PrefixExpression<'ast>) {
        println!("{:indent$}PrefixExpr {:?}", "", node.operator, indent = self.indent);
        self.indent += 2;
        self.walk_prefix(node);
        self.indent -= 2;
    }

    fn visit_var(&mut self, node: &VarExpression<'ast>) {
        let name = self.session.lookup_string(node.name.value);
        println!("{:indent$}VarExpr {} {:?}", "", name, node.kind, indent = self.indent);
        self.indent += 2;
        self.visit_expression(node.value);
        self.indent -= 2;
    }

    fn visit_call(&mut self, node: &CallExpression<'ast>) {
        println!("{:indent$}CallExpr", "", indent = self.indent);
        self.indent += 2;
        self.walk_call(node);
        self.indent -= 2;
    }

    fn visit_index(&mut self, node: &IndexExpression<'ast>) {
        println!("{:indent$}IndexExpr", "", indent = self.indent);
        self.indent += 2;
        self.walk_index(node);
        self.indent -= 2;
    }

    fn visit_member_access(&mut self, node: &MemberAccessExpression<'ast>) {
        let member = self.session.lookup_string(node.member.value);
        println!("{:indent$}MemberAccessExpr .{}", "", member, indent = self.indent);
        self.indent += 2;
        self.visit_expression(node.source);
        self.indent -= 2;
    }

    fn visit_if(&mut self, node: &IfExpression<'ast>) {
        println!("{:indent$}IfExpr", "", indent = self.indent);
        self.indent += 2;
        self.visit_expression(node.condition);
        self.visit_block(&node.consequence);
        if let Some(alt) = node.alternative {
            self.visit_expression(alt);
        }
        self.indent -= 2;
    }

    fn visit_function(&mut self, node: &FunctionLiteral<'ast>) {
        print!("{:indent$}FunctionLitExpr", "", indent = self.indent);
        if let Some(ty) = node.explicit_ty {
            print!(" -> {}", self.session.lookup_string(ty));
        }
        println!();
        self.indent += 2;
        for param in node.params {
            self.visit_var_decl_statement(param);
        }
        self.visit_block(&node.body);
        self.indent -= 2;
    }

    fn visit_array(&mut self, node: &ArrayLiteral<'ast>) {
        println!("{:indent$}ArrayLitExpr", "", indent = self.indent);
        self.indent += 2;
        self.walk_array(node);
        self.indent -= 2;
    }

    fn visit_block(&mut self, node: &BlockExpression<'ast>) {
        println!("{:indent$}BlockExpr", "", indent = self.indent);
        self.indent += 2;
        self.walk_block(node);
        self.indent -= 2;
    }

    fn visit_struct_literal(&mut self, node: &StructLiteral<'ast>) {
        let name = self.session.lookup_string(node.name.value);
        println!("{:indent$}StructLiteralExpr {}", "", name, indent = self.indent);
        self.indent += 2;
        for field in node.fields {
            self.visit_var(field);
        }
        self.indent -= 2;
    }
}

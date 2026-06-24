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
    IndexExpression,
    InfixExpression,
    IntegerLiteral,
    NullLiteral,
    PrefixExpression,
    Program,
    ReturnStatement,
    StringLiteral,
    VarDeclExpression,
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

impl Visitor for ASTDumper<'_> {
    fn visit_program(&mut self, program: &Program) {
        println!("{:indent$}Program", "", indent = self.indent);
        self.indent += 2;
        self.walk_program(program);
        self.indent -= 2;
    }

    fn visit_expression_statement(&mut self, _node: &ExpressionStatement) {
        println!("{:indent$}ExpressionStatement", "", indent = self.indent);
        self.indent += 2;
        self.walk_expression_statement(_node);
        self.indent -= 2;
    }

    fn visit_return_statement(&mut self, node: &ReturnStatement) {
        println!("{:indent$}ReturnStatement", "", indent = self.indent);
        self.indent += 2;
        self.walk_return_statement(node);
        self.indent -= 2;
    }

    fn visit_while_statement(&mut self, node: &WhileStatement) {
        println!("{:indent$}WhileStatement", "", indent = self.indent);
        self.indent += 2;
        self.walk_while_statement(node);
        self.indent -= 2;
    }

    fn visit_boolean(&mut self, node: &BooleanExpression) {
        println!("{:indent$}Boolean({})", "", node.value, indent = self.indent);
    }

    fn visit_integer(&mut self, node: &IntegerLiteral) {
        println!("{:indent$}Integer({})", "", node.value, indent = self.indent);
    }

    fn visit_float(&mut self, node: &FloatLiteral) {
        println!("{:indent$}Float({})", "", node.value, indent = self.indent);
    }

    fn visit_string(&mut self, node: &StringLiteral) {
        let v = self.session.lookup_string(node.value);
        println!("{:indent$}String({:?})", "", v, indent = self.indent);
    }

    fn visit_null(&mut self, _node: &NullLiteral) {
        println!("{:indent$}Null", "", indent = self.indent);
    }

    fn visit_identifier(&mut self, node: &Identifier) {
        let v = self.session.lookup_string(node.value);
        println!("{:indent$}Identifier({})", "", v, indent = self.indent);
    }

    fn visit_infix(&mut self, node: &InfixExpression) {
        println!(
            "{:indent$}InfixExpression({:?})",
            "",
            node.operator,
            indent = self.indent
        );
        self.indent += 2;
        self.walk_infix(node);
        self.indent -= 2;
    }

    fn visit_prefix(&mut self, node: &PrefixExpression) {
        println!(
            "{:indent$}PrefixExpression({:?})",
            "",
            node.operator,
            indent = self.indent
        );
        self.indent += 2;
        self.walk_prefix(node);
        self.indent -= 2;
    }

    fn visit_var(&mut self, node: &VarExpression) {
        println!("{:indent$}VarExpression({:?})", "", node.kind, indent = self.indent);
        self.indent += 2;
        self.walk_var(node);
        self.indent -= 2;
    }

    fn visit_var_decl(&mut self, node: &VarDeclExpression) {
        println!(
            "{:indent$}VarDeclExpression({:?})",
            "",
            node.explicit_ty,
            indent = self.indent
        );
        self.indent += 2;
        self.walk_var_decl(node);
        self.indent -= 2;
    }

    fn visit_call(&mut self, node: &CallExpression) {
        println!("{:indent$}CallExpression", "", indent = self.indent);
        self.indent += 2;
        self.walk_call(node);
        self.indent -= 2;
    }

    fn visit_index(&mut self, node: &IndexExpression) {
        println!("{:indent$}IndexExpression", "", indent = self.indent);
        self.indent += 2;
        self.walk_index(node);
        self.indent -= 2;
    }

    fn visit_if(&mut self, node: &IfExpression) {
        println!("{:indent$}IfExpression", "", indent = self.indent);
        self.indent += 2;
        self.walk_if(node);
        self.indent -= 2;
    }

    fn visit_function(&mut self, node: &FunctionLiteral) {
        println!("{:indent$}FunctionLiteral", "", indent = self.indent);
        self.indent += 2;
        self.walk_function(node);
        self.indent -= 2;
    }

    fn visit_array(&mut self, node: &ArrayLiteral) {
        println!("{:indent$}ArrayLiteral", "", indent = self.indent);
        self.indent += 2;
        self.walk_array(node);
        self.indent -= 2;
    }

    fn visit_block(&mut self, node: &BlockExpression) {
        println!("{:indent$}BlockExpression", "", indent = self.indent);
        self.indent += 2;
        self.walk_block(node);
        self.indent -= 2;
    }
}

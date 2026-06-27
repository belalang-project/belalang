use super::Statement;

#[derive(Default)]
pub struct Program {
    pub statements: Vec<Statement>,
}

impl Program {
    pub fn add_stmt(&mut self, stmt: Statement) {
        self.statements.push(stmt);
    }
}

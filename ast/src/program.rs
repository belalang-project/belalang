use super::Statement;

pub struct Program<'ast> {
    pub statements: &'ast [Statement<'ast>],
}

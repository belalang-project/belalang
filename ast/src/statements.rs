use super::{
    BlockExpression,
    Expression,
    Identifier,
    Type,
};

#[derive(Debug, Clone, Copy)]
pub struct ExpressionStatement<'ast> {
    pub expression: Expression<'ast>,
}

#[derive(Debug, Clone, Copy)]
pub struct ReturnStatement<'ast> {
    pub return_value: Expression<'ast>,
}

#[derive(Debug, Clone, Copy)]
pub struct WhileStatement<'ast> {
    pub condition: Expression<'ast>,
    pub block: BlockExpression<'ast>,
}

#[derive(Debug, Clone, Copy)]
pub struct VarDeclStatement<'ast> {
    pub name: Identifier,
    pub value: Option<&'ast Expression<'ast>>,
    pub explicit_ty: Option<Type>,
}

#[derive(Debug, Clone, Copy)]
pub struct StructDeclStatement<'ast> {
    pub name: Identifier,
    pub fields: &'ast [VarDeclStatement<'ast>],
}

#[derive(Debug, Clone, Copy)]
pub enum Statement<'ast> {
    Expression(ExpressionStatement<'ast>),
    Return(ReturnStatement<'ast>),
    While(WhileStatement<'ast>),
    VarDecl(VarDeclStatement<'ast>),
    StructDecl(StructDeclStatement<'ast>),
}

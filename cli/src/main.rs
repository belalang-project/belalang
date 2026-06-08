use std::{
    fs,
    path::PathBuf,
};

use ast::Parser;
use birgen::BIRGen;
use clap::{
    Parser as ClapParser,
    ValueEnum,
};
use lexer::Lexer;

#[derive(ValueEnum, Clone, Debug, Default)]
enum EmitTarget {
    #[default]
    Bir,
    Ast,
}

#[derive(ClapParser)]
#[command(version, about, long_about = None)]
struct Belalang {
    /// Path to the .bel file to compile
    path: PathBuf,

    /// What to emit
    #[arg(long, value_enum, default_value_t = EmitTarget::Bir)]
    emit: EmitTarget,
}

fn main() -> anyhow::Result<()> {
    let belalang = Belalang::parse();

    let source = fs::read_to_string(&belalang.path)?;

    let lexer = Lexer::new(&source);
    let mut parser = Parser::new(lexer);
    let program = parser.parse_program().map_err(|e| anyhow::anyhow!("{}", e))?;

    match belalang.emit {
        EmitTarget::Bir => {
            let mut generator = BIRGen::new();
            generator.generate_program(&program);
            generator.optimize();
            println!("{}", generator.dump_to_string());
        },
        EmitTarget::Ast => {
            println!("{:#?}", program.statements);
        },
    }

    Ok(())
}

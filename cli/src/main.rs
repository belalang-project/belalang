use std::{
    env,
    path::PathBuf,
};

use anyhow::Context;
use ast::{
    Parser,
    Visitor,
};
use birgen::BIRGen;
use clap::{
    Parser as ClapParser,
    ValueEnum,
};
use lexer::Lexer;
use session::Session;

#[derive(ValueEnum, Clone, Debug, Default)]
enum EmitTarget {
    Bir,
    Ast,
    Tokens,
    Llvm,
    Obj,
    #[default]
    Exe,
}

#[derive(ClapParser)]
#[command(version, about, long_about = None)]
struct Belalang {
    /// Path to the .bel file to compile
    path: PathBuf,

    /// Path to the output file
    #[arg(long, short)]
    out: Option<PathBuf>,

    /// What to emit
    #[arg(long, value_enum, default_value_t = EmitTarget::Exe)]
    emit: EmitTarget,
}

impl Belalang {
    fn get_out_path(&self) -> Option<PathBuf> {
        if let Some(out) = &self.out {
            return Some(out.to_path_buf());
        }

        if let EmitTarget::Obj = self.emit {
            return Some(self.path.with_added_extension("o"));
        }

        None
    }
}

fn main() -> anyhow::Result<()> {
    let belalang = Belalang::parse();

    let session = Session::for_file(belalang.path.clone())?;

    if let EmitTarget::Tokens = belalang.emit {
        let mut lexer = Lexer::new(&session);
        loop {
            let token = lexer.next_token().map_err(|e| anyhow::anyhow!("{}", e))?;
            if token.kind == lexer::TokenKind::EOF {
                break;
            }
            println!("{:?}", token);
        }
        return Ok(());
    }

    let lexer = Lexer::new(&session);
    let mut parser = Parser::new(&session, lexer);
    let program = parser.parse_program().map_err(|e| anyhow::anyhow!("{}", e))?;

    match belalang.emit {
        EmitTarget::Bir => {
            let mut generator = BIRGen::new(&session);
            generator.generate_program(&program);
            generator.optimize();
            println!("{}", generator.dump_to_string());
        },
        EmitTarget::Ast => {
            let mut dumper = ast::ASTDumper::new();
            dumper.visit_program(&program);
        },
        EmitTarget::Llvm => {
            let mut birgen = BIRGen::new(&session);
            birgen.generate_program(&program);
            birgen.optimize();

            let llvmgen = birgen.llvmgen();
            println!("{}", llvmgen.dump_to_string());
        },
        EmitTarget::Obj => {
            let mut birgen = BIRGen::new(&session);
            birgen.generate_program(&program);
            birgen.optimize();

            let llvmgen = birgen.llvmgen();
            let out = belalang
                .get_out_path()
                .context("Path is None")?
                .to_str()
                .context("Path contains invalid UTF-8 data")?
                .to_string();
            println!("{}", llvmgen.compile_object_file(out));
        },
        EmitTarget::Exe => {
            let mut birgen = BIRGen::new(&session);
            birgen.generate_program(&program);
            birgen.optimize();

            let llvmgen = birgen.llvmgen();
            let obj_out = belalang
                .path
                .with_added_extension("o")
                .to_str()
                .context("invalid UTF-8 data")?
                .to_string();
            let _ = llvmgen.compile_object_file(obj_out.clone());

            let cc = env::var("CC").unwrap_or("cc".to_string());
            let brt = env::var("BRT_DIR").unwrap_or_else(|_| "/usr/local/lib".to_string());

            let status = std::process::Command::new(cc)
                .arg(obj_out)
                .arg(format!("-L{}", brt))
                .arg("-lbrt")
                .arg("-o")
                .arg(belalang.path.with_extension(""))
                .status()?;

            if !status.success() {
                anyhow::bail!("linker failed with exit code: {}", status);
            }
        },
        EmitTarget::Tokens => unreachable!(),
    }

    Ok(())
}

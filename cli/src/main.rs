use std::{
    env,
    io::{
        self,
        IsTerminal,
    },
    os::unix::process::CommandExt,
    path::PathBuf,
};

use anyhow::Context;
use ast::{
    Parser,
    TypeInferer,
    Visitor,
};
use birgen::BIRGen;
use clap::{
    Parser as ClapParser,
    Subcommand,
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

#[derive(ValueEnum, Clone, Debug, Default)]
enum ColorChoice {
    Always,
    Never,
    #[default]
    Auto,
}

#[derive(clap::Args)]
struct BuildArgs {
    /// Path to the .bel file to compile
    path: PathBuf,

    /// Path to the output file
    #[arg(long, short)]
    out: Option<PathBuf>,

    /// What to emit
    #[arg(long, value_enum, default_value_t = EmitTarget::Exe)]
    emit: EmitTarget,
}

impl BuildArgs {
    fn get_out_path(&self) -> Option<PathBuf> {
        if let Some(out) = &self.out {
            return Some(out.to_path_buf());
        }

        if let EmitTarget::Obj = self.emit {
            return Some(self.path.with_added_extension("o"));
        }

        if let EmitTarget::Exe = self.emit {
            return Some(self.path.with_added_extension("o"));
        }

        None
    }
}

#[derive(clap::Args)]
struct RunArgs {
    /// Path to the .bel file to run
    path: PathBuf,
}

#[derive(Subcommand)]
enum Commands {
    Build(BuildArgs),
    Run(RunArgs),
}

#[derive(ClapParser)]
#[command(version, about, long_about = None)]
struct Belalang {
    #[command(subcommand)]
    command: Commands,

    /// Use color
    #[arg(long, value_enum, default_value_t = ColorChoice::Auto)]
    color: ColorChoice,
}

impl Belalang {
    fn use_color(&self) -> bool {
        match self.color {
            ColorChoice::Always => true,
            ColorChoice::Never => false,
            ColorChoice::Auto => io::stdout().is_terminal(),
        }
    }
}

fn main() -> anyhow::Result<()> {
    let belalang = Belalang::parse();
    let use_color = belalang.use_color();

    match belalang.command {
        Commands::Build(args) => build(args, use_color),
        Commands::Run(args) => run(args, use_color),
    }
}

fn build(args: BuildArgs, use_color: bool) -> anyhow::Result<()> {
    let session = Session::for_file(args.path.clone())?;

    let mut lexer = Lexer::new(&session);

    if let EmitTarget::Tokens = args.emit {
        let mut dumper = lexer::TokensDumper::new(&session, &mut lexer);
        let res = dumper.dump();
        check_errors(&session, use_color)?;
        res?;
        return Ok(());
    }

    let mut parser = Parser::new(&session, lexer);
    let Ok(program) = parser.parse_program() else {
        check_errors(&session, use_color)?;
        return Ok(());
    };
    check_errors(&session, use_color)?;

    let mut ty_infer = TypeInferer::new(&session);
    ty_infer.infer(&program);
    check_errors(&session, use_color)?;

    if let EmitTarget::Ast = args.emit {
        let mut dumper = ast::ASTDumper::new(&session);
        dumper.visit_program(&program);
        return Ok(());
    }

    let mut birgen = BIRGen::new(&session);
    birgen.generate_program(&program);
    birgen.optimize();

    if let EmitTarget::Bir = args.emit {
        println!("{}", birgen.dump_to_string());
        return Ok(());
    }

    let llvmgen = birgen.llvmgen();

    if let EmitTarget::Llvm = args.emit {
        println!("{}", llvmgen.dump_to_string());
        return Ok(());
    }

    let out = args
        .get_out_path()
        .context("Path is None")?
        .to_str()
        .context("Path contains invalid UTF-8 data")?
        .to_string();
    println!("{}", llvmgen.compile_object_file(out.clone()));

    if let EmitTarget::Obj = args.emit {
        return Ok(());
    }

    let cc = env::var("CC").unwrap_or("cc".to_string());
    let brt = env::var("BRT_DIR").unwrap_or_else(|_| "/usr/local/lib".to_string());

    let status = std::process::Command::new(cc)
        .arg(out)
        .arg(format!("-L{}", brt))
        .arg("-lbrt")
        .arg("-o")
        .arg(args.path.with_extension(""))
        .status()?;

    if !status.success() {
        anyhow::bail!("linker failed with exit code: {}", status);
    }

    Ok(())
}

fn run(args: RunArgs, use_color: bool) -> anyhow::Result<()> {
    let session = Session::for_file(args.path.clone())?;

    let lexer = Lexer::new(&session);

    let mut parser = Parser::new(&session, lexer);
    let Ok(program) = parser.parse_program() else {
        check_errors(&session, use_color)?;
        return Ok(());
    };
    check_errors(&session, use_color)?;

    let mut birgen = BIRGen::new(&session);
    birgen.generate_program(&program);
    birgen.optimize();

    let llvmgen = birgen.llvmgen();
    let obj_out = args
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
        .arg(args.path.with_extension(""))
        .status()?;

    if !status.success() {
        anyhow::bail!("linker failed with exit code: {}", status);
    }

    let exe = std::fs::canonicalize(args.path.with_extension("")).context("Failed to canonicalize exe path")?;
    let err = std::process::Command::new(exe).exec();
    anyhow::bail!("Failed to exec: {}", err);
}

fn check_errors(session: &Session, use_color: bool) -> anyhow::Result<()> {
    if session.has_errors() {
        for d in session.take_diagnostics() {
            diag::print_diagnostics(&session.source_text, session.get_source_file(), &d, use_color);
        }
        anyhow::bail!("compilation failed due to previous errors");
    }
    Ok(())
}

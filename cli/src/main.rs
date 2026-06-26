use std::{
    io::{
        self,
        IsTerminal,
    },
    path::PathBuf,
};

use clap::{
    Parser as ClapParser,
    Subcommand,
    ValueEnum,
};

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
    let bb = bbuild::BBuild::new(&args.path, use_color)?;

    if let EmitTarget::Tokens = args.emit {
        bb.dump_tokens()?;
        return Ok(());
    }

    let program = bb.parse_program()?;
    bb.infer_types(&program)?;

    if let EmitTarget::Ast = args.emit {
        bb.dump_ast(&program)?;
        return Ok(());
    }

    if let EmitTarget::Bir = args.emit {
        println!("{}", bb.dump_bir(&program));
        return Ok(());
    }

    if let EmitTarget::Llvm = args.emit {
        println!("{}", bb.dump_llvm(&program));
        return Ok(());
    }

    let compiled_msg = bb.compile_object_file(&program)?;
    println!("{}", compiled_msg);

    if let EmitTarget::Obj = args.emit {
        return Ok(());
    }

    bb.link_objects()?;

    Ok(())
}

fn run(args: RunArgs, use_color: bool) -> anyhow::Result<()> {
    let bb = bbuild::BBuild::new(&args.path, use_color)?;

    let program = bb.parse_program()?;
    bb.infer_types(&program)?;
    bb.compile_object_file(&program)?;
    bb.link_objects()?;
    bb.execute_artifact();

    Ok(())
}

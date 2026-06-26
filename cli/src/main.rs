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
    /// Path to the output file
    #[arg(long, short)]
    out: Option<PathBuf>,

    /// What to emit
    #[arg(long, value_enum, default_value_t = EmitTarget::Exe)]
    emit: EmitTarget,
}

#[derive(clap::Args)]
struct RunArgs {}

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

    /// Path to the .bel file to run
    path: PathBuf,
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
    let bctx = bbuild::BuildContext {
        use_color: belalang.use_color(),
    };

    let bb = bbuild::BBuild::new(&belalang.path, bctx)?;

    match belalang.command {
        Commands::Build(args) => build(args.emit, bb),
        Commands::Run(..) => run(bb),
    }
}

fn build(emit: EmitTarget, bb: bbuild::BBuild) -> anyhow::Result<()> {
    if let EmitTarget::Tokens = emit {
        bb.dump_tokens()?;
        return Ok(());
    }

    let program = bb.parse_program()?;
    bb.infer_types(&program)?;

    if let EmitTarget::Ast = emit {
        bb.dump_ast(&program)?;
        return Ok(());
    }

    if let EmitTarget::Bir = emit {
        println!("{}", bb.dump_bir(&program));
        return Ok(());
    }

    if let EmitTarget::Llvm = emit {
        println!("{}", bb.dump_llvm(&program));
        return Ok(());
    }

    let compiled_msg = bb.compile_object_file(&program)?;
    println!("{}", compiled_msg);

    if let EmitTarget::Obj = emit {
        return Ok(());
    }

    bb.link_objects()?;

    Ok(())
}

fn run(bb: bbuild::BBuild) -> anyhow::Result<()> {
    let program = bb.parse_program()?;
    bb.infer_types(&program)?;
    bb.compile_object_file(&program)?;
    bb.link_objects()?;
    bb.execute_artifact();

    Ok(())
}

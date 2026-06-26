use std::{
    io::{
        self,
        IsTerminal,
    },
    path::PathBuf,
    str::FromStr,
};

use bbuild::{
    BuildContext,
    EmitTarget,
};
use lexopt::{
    Arg,
    ValueExt,
};

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
enum Subcommands {
    Build,
    Run,
    None,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
enum ColorChoice {
    Always,
    Never,
    Auto,
}

impl FromStr for ColorChoice {
    type Err = String;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s.to_lowercase().as_str() {
            "auto" => Ok(ColorChoice::Auto),
            "always" => Ok(ColorChoice::Always),
            "never" => Ok(ColorChoice::Never),
            _ => Err(format!("invalid style '{}' [pick from: auto, always, never]", s)),
        }
    }
}

fn parse_emit_target(s: &str) -> Result<EmitTarget, String> {
    match s.to_lowercase().as_str() {
        "bir" => Ok(EmitTarget::Bir),
        "ast" => Ok(EmitTarget::Ast),
        "tokens" => Ok(EmitTarget::Tokens),
        "llvm" => Ok(EmitTarget::Llvm),
        "obj" => Ok(EmitTarget::Obj),
        "exe" => Ok(EmitTarget::Exe),
        _ => Err(format!(
            "invalid emit target '{}' [pick from: bir, ast, tokens, llvm, obj, exe]",
            s
        )),
    }
}

fn print_help() {
    println!(
        "belalang

Usage: belalang [OPTIONS] <COMMAND> <PATH>

Commands:
  build    Compile a .bel file
  run      Run a .bel file

Options:
      --color <auto|always|never>  Control color output [default: auto]
      --emit <target>              What build target to emit [default: exe]
                                   [choices: bir, ast, tokens, llvm, obj, exe]
  -h, --help                       Print help"
    );
}

fn main() -> anyhow::Result<()> {
    let mut subcommand = Subcommands::None;
    let mut path = None;
    let mut color = ColorChoice::Auto;
    let mut emit = EmitTarget::Exe;

    let mut parser = lexopt::Parser::from_env();
    while let Some(arg) = parser.next()? {
        match arg {
            Arg::Long("color") => {
                let val = parser.value()?;
                let s = val.string()?;
                color = ColorChoice::from_str(&s).map_err(|e| anyhow::anyhow!("{}", e))?;
            },
            Arg::Long("emit") => {
                let val = parser.value()?;
                let s = val.string()?;
                emit = parse_emit_target(&s).map_err(|e| anyhow::anyhow!("{}", e))?;
            },
            Arg::Long("help") | Arg::Short('h') => {
                print_help();
                return Ok(());
            },
            Arg::Value(value) => {
                let value_str = value.string()?;
                match subcommand {
                    Subcommands::None => match value_str.as_str() {
                        "build" => {
                            subcommand = Subcommands::Build;
                        },
                        "run" => {
                            subcommand = Subcommands::Run;
                        },
                        _ => {
                            anyhow::bail!("unknown subcommand '{}'", value_str);
                        },
                    },
                    Subcommands::Build | Subcommands::Run => {
                        if path.is_some() {
                            anyhow::bail!("unexpected argument '{}'", value_str);
                        }
                        path = Some(PathBuf::from(value_str));
                    },
                }
            },
            _ => return Err(arg.unexpected().into()),
        }
    }

    if subcommand == Subcommands::None {
        print_help();
        return Ok(());
    }

    let path = match path {
        Some(p) => p,
        None => {
            anyhow::bail!("missing path to the .bel file");
        },
    };

    let use_color = match color {
        ColorChoice::Always => true,
        ColorChoice::Never => false,
        ColorChoice::Auto => io::stdout().is_terminal(),
    };

    if subcommand == Subcommands::Run {
        emit = EmitTarget::Exe;
    }

    let bctx = BuildContext { use_color, emit };
    let bb = bbuild::BBuild::new(&path, bctx)?;

    match subcommand {
        Subcommands::Build => build(bb),
        Subcommands::Run => run(bb),
        Subcommands::None => unreachable!(),
    }
}

fn build(bb: bbuild::BBuild) -> anyhow::Result<()> {
    if let EmitTarget::Tokens = bb.emit() {
        bb.dump_tokens()?;
        return Ok(());
    }

    let program = bb.parse_program()?;
    bb.infer_types(&program)?;

    if let EmitTarget::Ast = bb.emit() {
        bb.dump_ast(&program)?;
        return Ok(());
    }

    if let EmitTarget::Bir = bb.emit() {
        println!("{}", bb.dump_bir(&program));
        return Ok(());
    }

    if let EmitTarget::Llvm = bb.emit() {
        println!("{}", bb.dump_llvm(&program));
        return Ok(());
    }

    let compiled_msg = bb.compile_object_file(&program)?;
    println!("{}", compiled_msg);

    if let EmitTarget::Obj = bb.emit() {
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

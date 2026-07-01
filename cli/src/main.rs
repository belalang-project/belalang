use std::{
    env,
    fs,
    io::{
        self,
        IsTerminal,
    },
    path::{
        Path,
        PathBuf,
    },
    process,
    str::FromStr,
    time,
};

use anyhow::Context;
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

#[derive(Clone, Debug, PartialEq, Eq)]
enum Source {
    File(PathBuf),
    Stdin,
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
    let mut src = None;
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
                        if src.is_some() {
                            anyhow::bail!("unexpected argument '{}'", value_str);
                        }
                        if value_str == "-" {
                            src = Some(Source::Stdin);
                        } else {
                            src = Some(Source::File(PathBuf::from(value_str)));
                        }
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

    let src = match src {
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

    let mut _temp_dir_guard = None;
    let out_dir = if let Subcommands::Run = subcommand {
        let guard = TempDirGuard::new()?;
        let path = guard.path.to_path_buf();
        _temp_dir_guard = Some(guard);
        path
    } else {
        env::current_dir().context("failed to get current directory")?
    };

    let bctx = BuildContext {
        use_color,
        emit,
        out_dir,
    };
    let bb = if let Source::File(path) = src {
        bbuild::BBuild::new(&path, bctx)
    } else {
        bbuild::BBuild::from_stdin(bctx)
    }?;

    let res = match subcommand {
        Subcommands::Build => build(bb),
        Subcommands::Run => run(bb),
        Subcommands::None => unreachable!(),
    };

    if res.is_err() {
        // the error messages are already handled by diagnostics
        std::process::exit(1);
    }

    Ok(())
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

struct TempDirGuard {
    path: PathBuf,
}

impl TempDirGuard {
    pub fn new() -> anyhow::Result<Self> {
        let mut path = env::temp_dir();

        let pid = process::id();
        let nanos = time::SystemTime::now()
            .duration_since(time::UNIX_EPOCH)
            .unwrap_or_default()
            .as_nanos();

        path.push(format!("belalang_out_{}_{}", pid, nanos));
        fs::create_dir_all(&path)?;

        Ok(Self { path })
    }
}

impl std::ops::Deref for TempDirGuard {
    type Target = Path;

    fn deref(&self) -> &Self::Target {
        &self.path
    }
}

impl Drop for TempDirGuard {
    fn drop(&mut self) {
        let _ = fs::remove_dir_all(&self.path);
    }
}

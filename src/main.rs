use clap::{
    Parser,
    Subcommand,
};

pub mod commands;

#[derive(Parser)]
#[command(version, about, long_about = None)]
struct Cli {
    #[command(subcommand)]
    command: Commands,
}

#[derive(Subcommand)]
enum Commands {
    Dis(commands::dis::Args),
    Compile(commands::compile::Args),
}

fn main() {
    let cli = Cli::parse();

    match cli.command {
        Commands::Dis(args) => args.exec(),
        Commands::Compile(args) => args.exec(),
    }
}

use std::{
    fs,
    path::PathBuf,
};

#[derive(clap::Args)]
pub struct Args {
    path: PathBuf,
}

impl Args {
    pub fn exec(self) {
        let source = fs::read_to_string(self.path).unwrap();
        let bytecode = belc::compile(&source);

        let dis = belc::disassemble(bytecode.instructions);

        println!("{dis}");
    }
}

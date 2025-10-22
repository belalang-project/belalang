use std::{
    fs::{
        self,
        File,
    },
    io::Write,
    path::PathBuf,
};

#[derive(clap::Args)]
pub struct Args {
    path: PathBuf,
}

impl Args {
    pub fn exec(mut self) {
        let source = fs::read_to_string(self.path.clone()).unwrap();
        let bytecode = belc::compile(&source);

        self.path.set_extension("belc");
        let mut file = File::create(self.path).unwrap();

        file.write_all(&bytecode.into_bytes()).unwrap();
    }
}

use std::path::PathBuf;

#[derive(clap::Args)]
pub struct Args {
    path: PathBuf,
}

impl Args {
    pub fn exec(self) {
        unimplemented!()
    }
}

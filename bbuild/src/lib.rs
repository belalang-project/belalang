use std::{
    env,
    path::{
        Path,
        PathBuf,
    },
    process::Command,
};

pub struct BBuild {
    cc: String,
    brt_dir: String,

    out_obj: PathBuf,
    out_exe: PathBuf,
}

impl BBuild {
    pub fn new(source_path: &Path) -> Self {
        let cc = env::var("CC").unwrap_or("cc".to_string());
        let brt_dir = env::var("BRT_DIR").unwrap_or_else(|_| "/usr/local/lib".to_string());

        let out_obj = source_path.with_added_extension("o");
        let out_exe = source_path.with_added_extension("");

        Self {
            cc,
            brt_dir,
            out_obj,
            out_exe,
        }
    }

    pub fn link_objects(&self) -> anyhow::Result<()> {
        let status = Command::new(&self.cc)
            .arg(&self.out_obj)
            .arg(format!("-L{}", self.brt_dir))
            .arg("-lbrt")
            .arg("-o")
            .arg(&self.out_exe)
            .status()?;

        if !status.success() {
            anyhow::bail!("linker failed with exit code: {}", status);
        }

        Ok(())
    }
}

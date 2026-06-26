use std::{
    env,
    os::unix::process::CommandExt,
    path::{
        Path,
        PathBuf,
    },
    process::Command,
};

use anyhow::Context;
use ast::{
    Parser,
    Program,
};
use birgen::BIRGen;
use lexer::Lexer;
use session::Session;

pub struct BBuild {
    cc: String,
    brt_dir: String,

    out_obj: PathBuf,
    out_exe: PathBuf,

    session: Session,
    use_color: bool,
}

impl BBuild {
    pub fn new(source_path: &Path, use_color: bool) -> anyhow::Result<Self> {
        let cc = env::var("CC").unwrap_or("cc".to_string());
        let brt_dir = env::var("BRT_DIR").unwrap_or_else(|_| "/usr/local/lib".to_string());

        let out_obj = source_path.with_added_extension("o");
        let out_exe = source_path.with_added_extension("");

        let session = Session::for_file(source_path.to_path_buf())?;

        Ok(Self {
            cc,
            brt_dir,
            out_obj,
            out_exe,
            session,
            use_color,
        })
    }

    pub fn parse_program(&self) -> anyhow::Result<Program> {
        let lexer = Lexer::new(&self.session);
        let mut parser = Parser::new(&self.session, lexer);

        let Ok(program) = parser.parse_program() else {
            check_errors(&self.session, self.use_color)?;
            anyhow::bail!("compilation failed due to previous errors");
        };
        check_errors(&self.session, self.use_color)?;

        Ok(program)
    }

    pub fn compile_object_file(&self, program: Program) -> anyhow::Result<()> {
        let mut birgen = BIRGen::new(&self.session);
        birgen.generate_program(&program);
        birgen.optimize();

        let llvmgen = birgen.llvmgen();
        let obj_out = self.out_obj.to_str().context("invalid UTF-8 data")?.to_string();
        let _ = llvmgen.compile_object_file(obj_out);

        Ok(())
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

    pub fn execute_artifact(&self) {
        let exe = std::fs::canonicalize(&self.out_exe).unwrap();
        let err = Command::new(exe).exec();
    }
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

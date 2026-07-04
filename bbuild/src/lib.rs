use std::{
    env,
    io::{
        self,
        Read,
    },
    os::unix::process::CommandExt,
    path::{
        Path,
        PathBuf,
    },
    process::Command,
};

use anyhow::Context;
use ast::{
    ASTDumper,
    Ast,
    Parser,
    Program,
    Visitor,
};
use birgen::BIRGen;
use lexer::Lexer;
use session::Session;
use ty::TypeInferer;

pub struct BuildContext {
    pub use_color: bool,
    pub emit: EmitTarget,
    pub out_dir: PathBuf,
}

#[derive(Clone, Copy, Debug, Default, PartialEq, Eq)]
pub enum EmitTarget {
    Bir,
    Ast,
    Tokens,
    Llvm,
    Obj,
    #[default]
    Exe,
}

pub struct BBuild {
    bctx: BuildContext,

    cc: String,
    brt_dir: String,

    out_obj: PathBuf,
    out_exe: PathBuf,

    session: Session,
    ast: Ast,
}

impl BBuild {
    pub fn emit(&self) -> EmitTarget {
        self.bctx.emit
    }

    pub fn from_stdin(bctx: BuildContext) -> anyhow::Result<Self> {
        let cc = env::var("CC").unwrap_or("cc".to_string());
        let brt_dir = env::var("BRT_DIR").unwrap_or_else(|_| "/usr/local/lib".to_string());

        let out_obj = bctx.out_dir.join("belalang_out.o");
        let out_exe = bctx.out_dir.join("belalang_out");

        let mut source_text = String::new();
        io::stdin()
            .read_to_string(&mut source_text)
            .context("unable to read input from stdin")?;

        let session = Session::for_text(source_text)?;

        Ok(Self {
            cc,
            brt_dir,
            out_obj,
            out_exe,
            session,
            bctx,
            ast: Ast::new(),
        })
    }

    pub fn new(source_path: &Path, bctx: BuildContext) -> anyhow::Result<Self> {
        let cc = env::var("CC").unwrap_or("cc".to_string());
        let brt_dir = env::var("BRT_DIR").unwrap_or_else(|_| "/usr/local/lib".to_string());

        let mut out_obj = source_path.with_added_extension("o");
        let mut out_exe = source_path.with_extension("");

        if let Some(file_name) = out_obj.file_name() {
            out_obj = bctx.out_dir.join(file_name);
        }
        if let Some(file_name) = out_exe.file_name() {
            out_exe = bctx.out_dir.join(file_name);
        }

        let session = Session::for_file(source_path.to_path_buf())?;

        Ok(Self {
            cc,
            brt_dir,
            out_obj,
            out_exe,
            session,
            bctx,
            ast: Ast::new(),
        })
    }

    pub fn parse_program<'ast>(&'ast self) -> anyhow::Result<Program<'ast>> {
        let lexer = Lexer::new(&self.session);
        let mut parser = Parser::new(&self.session, lexer, &self.ast);

        let program = parser.parse_program();
        self.check_errors()?;
        Ok(program?)
    }

    pub fn dump_tokens(&self) -> anyhow::Result<()> {
        let mut lexer = Lexer::new(&self.session);
        let mut dumper = lexer::TokensDumper::new(&self.session, &mut lexer);
        let res = dumper.dump();
        self.check_errors()?;
        res?;
        Ok(())
    }

    pub fn infer_types<'ast>(&self, program: &Program<'ast>) -> anyhow::Result<()> {
        let mut ty_infer = TypeInferer::new(&self.session);
        ty_infer.infer(program);
        self.check_errors()?;
        Ok(())
    }

    pub fn dump_ast<'ast>(&self, program: &Program<'ast>) -> anyhow::Result<()> {
        let mut dumper = ASTDumper::new(&self.session);
        dumper.visit_program(program);
        Ok(())
    }

    pub fn dump_bir<'ast>(&self, program: &Program<'ast>) -> String {
        let mut birgen = BIRGen::new(&self.session);
        birgen.generate_program(program);
        birgen.optimize();
        birgen.dump_to_string()
    }

    pub fn dump_llvm<'ast>(&self, program: &Program<'ast>) -> String {
        let mut birgen = BIRGen::new(&self.session);
        birgen.generate_program(program);
        birgen.optimize();
        let llvmgen = birgen.llvmgen();
        llvmgen.dump_to_string()
    }

    pub fn compile_object_file<'ast>(&self, program: &Program<'ast>) -> anyhow::Result<String> {
        let mut birgen = BIRGen::new(&self.session);
        birgen.generate_program(program);
        birgen.optimize();

        let llvmgen = birgen.llvmgen();
        let obj_out = self.out_obj.to_str().context("invalid UTF-8 data")?.to_string();
        let compiled = llvmgen.compile_object_file(obj_out);

        Ok(compiled)
    }

    pub fn link_objects(&self) -> anyhow::Result<()> {
        let status = Command::new(&self.cc)
            .arg(&self.out_obj)
            .arg(format!("-L{}", self.brt_dir))
            .arg("-lbrt")
            .arg("-lbdwgc")
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

    fn check_errors(&self) -> anyhow::Result<()> {
        if self.session.has_errors() {
            for d in self.session.take_diagnostics() {
                diag::print_diagnostics(
                    &self.session.source_text,
                    self.session.get_source_file(),
                    &d,
                    self.bctx.use_color,
                );
            }
            anyhow::bail!("");
        }
        Ok(())
    }
}

use std::{
    fs,
    path::{
        Path,
        PathBuf,
    },
};

use belalang_ast::Parser;
use belalang_lexer::Lexer;
use clap::Parser as ClapParser;
use console::style;
use glob::glob;
use similar::{
    ChangeTag,
    TextDiff,
};
use walkdir::WalkDir;

#[derive(ClapParser)]
struct Args {
    filepath: PathBuf,

    #[arg(long)]
    bless: bool,
}

fn main() {
    if run().is_err() {
        std::process::exit(1);
    }
}

fn run() -> anyhow::Result<()> {
    let args = Args::parse();
    let mut btest = BTest::new(&args.filepath)?;

    if args.bless {
        btest.bless()?;
    } else {
        btest.runtest()?;
    }

    Ok(())
}

#[derive(Debug)]
struct TestCase {
    source_path: PathBuf,
    source_str: String,
    ast_path: PathBuf,
}

impl TestCase {
    fn new(source_path: PathBuf) -> anyhow::Result<TestCase> {
        let source_str = fs::read_to_string(&source_path)?.replace("\r\n", "\n");

        let mut ast_path = source_path.clone();
        ast_path.add_extension("ast");

        Ok(TestCase {
            source_path,
            source_str,
            ast_path,
        })
    }
}

#[derive(Debug)]
struct BTest {
    cases: Vec<TestCase>,
}

impl BTest {
    fn new(path: &PathBuf) -> anyhow::Result<BTest> {
        let mut cases = Vec::new();

        if path.exists() {
            if path.is_dir() {
                for entry in WalkDir::new(path) {
                    let entry = entry?;
                    let path = entry.path();

                    if path.is_file()
                        && let Some(ext) = path.extension()
                        && ext == "bel"
                    {
                        cases.push(TestCase::new(path.to_path_buf())?);
                    }
                }
            } else {
                cases.push(TestCase::new(path.to_path_buf())?);
            }
        } else {
            let pattern = path.to_string_lossy();
            for entry in glob(&pattern)? {
                let entry = entry?;
                if entry.is_file()
                    && let Some(ext) = path.extension()
                    && ext == "bel"
                {
                    cases.push(TestCase::new(path.to_path_buf())?);
                }
            }
        }

        Ok(BTest { cases })
    }

    fn bless(&mut self) -> anyhow::Result<()> {
        for case in &mut self.cases {
            let lexer = Lexer::new(&case.source_str);
            let mut parser = Parser::new(lexer);

            let program = parser.parse_program()?;
            let program_str = format!("{:#?}\n", program.statements);

            fs::write(&case.ast_path, program_str)?;
            eprintln!(
                "{} {}",
                style("bless").bold().blue(),
                case.source_path.to_string_lossy()
            );
        }

        Ok(())
    }

    fn runtest(&mut self) -> anyhow::Result<()> {
        let mut has_fail = false;
        let mut results = Vec::new();

        for case in &mut self.cases {
            let old_ast = fs::read_to_string(&case.ast_path)
                .unwrap_or_default()
                .replace("\r\n", "\n");

            let lexer = Lexer::new(&case.source_str);
            let mut parser = Parser::new(lexer);

            let program = match parser.parse_program() {
                Ok(program) => program,
                Err(err) => {
                    has_fail = true;
                    results.push((false, &case.ast_path));

                    eprintln!("[{}]", style(&case.source_path.to_string_lossy()).bold());
                    eprintln!("{}", err);
                    eprintln!();

                    continue;
                },
            };

            let new_ast = format!("{:#?}\n", program.statements);
            let has_diff = diff(&case.ast_path, &old_ast, &new_ast);
            results.push((!has_diff, &case.ast_path));
            if has_diff {
                has_fail = true;
            }
        }

        for (success, path) in &results {
            let path = path.to_string_lossy();
            let status = if *success {
                style("pass").green()
            } else {
                style("fail").red()
            };
            eprintln!("{} {}", status, path);
        }

        let (npass, nfail) = results.iter().fold(
            (0, 0),
            |acc, &x| {
                if x.0 { (acc.0 + 1, acc.1) } else { (acc.0, acc.1 + 1) }
            },
        );
        eprintln!(
            "\n{} tests run: {} {}, {} {}",
            results.len(),
            npass,
            style("passed").bold().green(),
            nfail,
            style("failed").bold().red()
        );

        if has_fail {
            anyhow::bail!("");
        }

        Ok(())
    }
}

/// utility function to output the diff between two strings
///
/// returns true if a diff was found and printed
fn diff(name: &Path, old: &String, new: &String) -> bool {
    let diff = TextDiff::from_lines(old, new);

    // two strings are equal
    if diff.ratio() == 1.0 {
        return false;
    }

    let name = name.to_str().unwrap_or("unknown");

    for hunk in diff.grouped_ops(3) {
        let first_op = hunk.first().unwrap();
        let line_number = first_op.new_range().start + 1;
        eprintln!("[{}:{}]", style(name).bold(), style(line_number).yellow());

        for op in hunk {
            for change in diff.iter_changes(&op) {
                let old_lineno = change.old_index();
                let new_lineno = change.new_index();

                let (sign, color) = match change.tag() {
                    ChangeTag::Delete => ("-", console::Color::Red),
                    ChangeTag::Insert => ("+", console::Color::Green),
                    ChangeTag::Equal => (" ", console::Color::White),
                };

                let gutter = format!(
                    "{:>4} {:>4}",
                    old_lineno.map(|n| (n + 1).to_string()).unwrap_or_default(),
                    new_lineno.map(|n| (n + 1).to_string()).unwrap_or_default()
                );

                eprint!(
                    "{} {} {}",
                    style(gutter).dim(),
                    style(sign).fg(color),
                    style(change).fg(color)
                );
            }
        }

        eprintln!();
    }

    true
}

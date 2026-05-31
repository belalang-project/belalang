default:
	just --list

lsp:
	bazelisk run //:refresh_compile_commands
	bazelisk run @rules_rust//tools/rust_analyzer:gen_rust_project

fmt:
	bazelisk run @rules_rust//:rustfmt

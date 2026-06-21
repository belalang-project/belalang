alias b := build
alias t := test
alias opt := bir-opt
alias tra := bir-translate
alias bel := belalang

default:
	@just --list

build:
	bazelisk build //...

test:
	bazelisk test //...

bir-opt *args:
	./bazel-bin/bir/bir-opt {{args}}

bir-translate *args:
	./bazel-bin/bir/bir-translate {{args}}

belalang *args:
	./bazel-bin/cli/belalang {{args}}

alias b := build
alias t := test
alias opt := bir-opt
alias bel := belalang

default:
	@just --list

build:
	bazelisk build //...

test:
	bazelisk test //...

bir-opt *args:
	./bazel-bin/bir/bir-opt {{args}}

belalang *args:
	./bazel-bin/cli/belalang {{args}}

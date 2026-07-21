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
	./bazel-bin/tools/bir-opt/bir-opt {{args}}

bir-translate *args:
	./bazel-bin/tools/bir-translate/bir-translate {{args}}

belalang *args:
	./bazel-bin/bin/belalang/belalang {{args}}

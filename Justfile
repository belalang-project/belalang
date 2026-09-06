alias b := build
alias t := test
alias opt := bir-opt
alias tra := bir-translate
alias bel := belalang

default:
	@just --list

build:
	cmake --build build

test:
	ctest --test-dir build --output-on-failure

bir-opt *args:
	./build/tools/bir-opt/bir-opt {{args}}

bir-translate *args:
	./build/tools/bir-translate/bir-translate {{args}}

belalang *args:
	./build/bin/belalang/belalang {{args}}

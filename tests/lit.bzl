load("@pypi//:requirements.bzl", "requirement")
load("@rules_python//python:defs.bzl", "py_test")

def lit_test(name, tests, data = [], env = {}, **kwargs):
    py_test(
        name = name,
        srcs = ["//tests:run_lit.py"],
        main = "//tests:run_lit.py",
        args = ["$(execpath %s)" % f for f in tests],
        data = tests + data + [
            "//tests:lit.cfg.py",
            "@llvm-project//llvm:FileCheck",
        ],
        env = dict({
            "FILECHECK": "$(rlocationpath @llvm-project//llvm:FileCheck)",
        }, **env),
        deps = [requirement("lit")],
        **kwargs
    )

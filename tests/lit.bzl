load("@pypi//:requirements.bzl", "requirement")
load("@rules_python//python:defs.bzl", "py_test")

def lit_test(name, tests, data = [], env = {}, **kwargs):
    individual_targets = []
    for test in tests:
        target_name = test.replace(".", "_").replace("/", "_")

        py_test(
            name = target_name,
            srcs = ["//tests:run_lit.py"],
            main = "//tests:run_lit.py",
            args = ["$(execpath %s)" % test],
            data = [test] + data + [
                "//tests:lit.cfg.py",
                "@llvm-project//llvm:FileCheck",
                "@llvm-project//llvm:not",
            ],
            env = dict({
                "FILECHECK": "$(rlocationpath @llvm-project//llvm:FileCheck)",
                "NOT": "$(rlocationpath @llvm-project//llvm:not)",
            }, **env),
            deps = [requirement("lit")],
            **kwargs
        )
        individual_targets.append(":" + target_name)

    native.test_suite(
        name = name,
        tests = individual_targets,
    )

load("@rules_shell//shell:sh_test.bzl", "sh_test")

def btest_test(name, test_dir, args = [], **kwargs):
    native.filegroup(
        name = name + "_data",
        srcs = native.glob([test_dir + "/**"]),
    )

    sh_test(
        name = name,
        srcs = ["//devtools/btest:run_test.sh"],
        data = [
            "//devtools/btest:btest",
            ":" + name + "_data",
        ],
        args = [
            "$(location //devtools/btest:btest)",
            test_dir,
        ] + args,
        **kwargs,
    )

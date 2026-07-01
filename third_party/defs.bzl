load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def _third_party_deps_impl(_ctx):
    http_archive(
        name = "bdwgc",
        build_file = "//third_party:bdwgc.BUILD",
        strip_prefix = "gc-8.2.12",
        urls = ["https://github.com/bdwgc/bdwgc/releases/download/v8.2.12/gc-8.2.12.tar.gz"],
    )

third_party_deps = module_extension(implementation = _third_party_deps_impl)

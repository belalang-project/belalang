load("@bazel_tools//tools/build_defs/repo:git.bzl", "new_git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def _third_party_deps_impl(_ctx):
    http_archive(
        name = "bdwgc",
        build_file = "//third_party:bdwgc.BUILD",
        strip_prefix = "gc-8.2.12",
        urls = ["https://github.com/bdwgc/bdwgc/releases/download/v8.2.12/gc-8.2.12.tar.gz"],
    )

    LLVM_COMMIT = "030e74c2808a9af58c6b4ef461fd0c2c7039d647"

    new_git_repository(
        name = "llvm-raw",
        build_file_content = "# empty",
        commit = LLVM_COMMIT,
        init_submodules = False,
        remote = "https://github.com/llvm/llvm-project.git",
    )

third_party_deps = module_extension(implementation = _third_party_deps_impl)

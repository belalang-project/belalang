import os
import sys
import lit
import lit.formats

config.test_source_root = os.path.dirname(os.path.abspath(__file__))
config.test_format = lit.formats.ShTest(False)

config.suffixes = [".mlir", ".bel"]

workspace_root = os.path.dirname(os.path.dirname(config.test_source_root))

def resolve_path(path):
    if not path or os.path.isabs(path):
        return path
    for var in ["RUNFILES_DIR", "TEST_SRCDIR"]:
        base = os.environ.get(var)
        if base:
            return os.path.normpath(os.path.join(base, path))
    return os.path.abspath(path)

def add_sub(name, env_var):
    val = os.environ.get(env_var)
    if val:
        config.substitutions.append((name, resolve_path(val)))

add_sub("%bir-opt", "BIR_OPT")
add_sub("%bir-translate", "BIR_TRANSLATE")
add_sub("%belalang", "BELALANG")
add_sub("%FileCheck", "FILECHECK")
add_sub("%not", "NOT")

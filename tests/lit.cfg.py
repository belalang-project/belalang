import os
import sys
import lit
import lit.formats

config.test_source_root = os.path.dirname(os.path.abspath(__file__))
config.test_format = lit.formats.ShTest(True)

config.suffixes = [".mlir", ".bel"]

workspace_root = os.path.dirname(os.path.dirname(config.test_source_root))

def add_sub(name, env_var):
    val = os.environ.get(env_var)
    if val:
        config.substitutions.append((name, val))

add_sub("%bir-opt", "BIR_OPT")
add_sub("%bir-translate", "BIR_TRANSLATE")
add_sub("%belalang", "BELALANG")
add_sub("%FileCheck", "FILECHECK")
add_sub("%not", "NOT")

import os
import sys
import lit
import lit.formats

config.test_source_root = os.path.dirname(os.path.abspath(__file__))
config.test_format = lit.formats.ShTest(True)
config.suffixes = [".mlir"]

workspace_root = os.path.dirname(os.path.dirname(os.path.dirname(config.test_source_root)))

bir_opt = os.environ.get("BIR_OPT")
if not bir_opt:
    bir_opt = os.path.join(workspace_root, "target", "bir-opt")
    if not os.path.exists(bir_opt):
        bir_opt = "bir-opt"

config.substitutions.append(("%bir-opt", bir_opt))
config.substitutions.append(("%FileCheck", os.environ.get("FILECHECK", "FileCheck")))

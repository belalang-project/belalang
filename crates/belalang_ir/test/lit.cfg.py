import os
import sys
import lit
import lit.formats

config.test_source_root = os.path.dirname(os.path.abspath(__file__))
config.test_format = lit.formats.ShTest(True)
config.suffixes = [".mlir"]

config.substitutions.append(("%mlir-opt", os.environ.get("MLIR_OPT", "mlir-opt")))
config.substitutions.append(("%FileCheck", os.environ.get("FILECHECK", "FileCheck")))

import os
import sys
import lit
import lit.formats

config.test_source_root = os.path.dirname(os.path.abspath(__file__))
config.test_format = lit.formats.ShTest(True)
config.suffixes = [".bel"]

workspace_root = os.path.dirname(os.path.dirname(os.path.dirname(config.test_source_root)))

config.substitutions.append(("%belalang", os.environ.get("BELALANG")))
config.substitutions.append(("%FileCheck", os.environ.get("FILECHECK", "FileCheck")))

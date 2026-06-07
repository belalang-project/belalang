#!/usr/bin/env python3

import os
import sys

import lit.main

def main():
    test_dir = os.path.dirname(os.path.abspath(__file__))

    # Resolve tool paths using TEST_SRCDIR.
    # $(rlocationpath ...) already includes the workspace/repo prefix
    # (e.g. "_main/bir/bir-opt" or
    # "llvm++llvm+llvm-project/llvm/FileCheck"), so just join with TEST_SRCDIR.
    # We need the absolute path, so join with cwd (the execroot).
    test_srcdir = os.environ.get("TEST_SRCDIR", "")
    for var in ("BELALANG", "FILECHECK"):
        val = os.environ.get(var)
        if val and test_srcdir:
            resolved = os.path.normpath(os.path.join(os.getcwd(), test_srcdir, val))
            os.environ[var] = resolved

    sys.argv = [sys.argv[0], "-v", test_dir] + sys.argv[1:]
    sys.exit(lit.main.main())

if __name__ == "__main__":
    main()

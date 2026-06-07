#!/usr/bin/env python3

import os
import sys

import lit.main

def main():
    test_dir = os.path.dirname(os.path.abspath(__file__))

    test_srcdir = os.environ.get("TEST_SRCDIR", "")
    cwd = os.getcwd()

    tool_vars = ["BIR_OPT", "BELALANG", "FILECHECK"]

    for var in tool_vars:
        val = os.environ.get(var)
        if val and test_srcdir:
            if os.path.isabs(val):
                continue
            resolved = os.path.normpath(os.path.join(cwd, test_srcdir, val))
            os.environ[var] = resolved

    sys.argv = [sys.argv[0], "-v"] + sys.argv[1:]
    sys.exit(lit.main.main())

if __name__ == "__main__":
    main()

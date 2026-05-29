#!/usr/bin/env python3
"""Self-contained MLIR test runner for Bazel.

Parses RUN lines from .mlir files, substitutes tool paths, and executes them.
Drop-in replacement for lit that avoids the lit Python package dependency.
"""

import glob as globmod
import os
import re
import subprocess
import sys


def _find_tool(name):
    runfiles = os.environ.get("RUNFILES_DIR")
    if not runfiles:
        runfiles = os.path.abspath(__file__ + ".runfiles")
    for root, _dirs, files in os.walk(runfiles):
        if name in files:
            return os.path.join(root, name)
    print("ERROR: Could not find '%s' in runfiles" % name, file=sys.stderr)
    sys.exit(1)


def _find_mlir_files(directory):
    return sorted(globmod.glob(os.path.join(directory, "**", "*.mlir"), recursive=True))


def _run_test(mlir_file, substitutions):
    with open(mlir_file) as f:
        content = f.read()

    run_lines = re.findall(r"//\s*RUN:\s*(.*)", content)
    if not run_lines:
        return True

    passed = True
    for line in run_lines:
        cmd = line.strip()
        for placeholder, value in substitutions.items():
            cmd = cmd.replace(placeholder, value)
        cmd = cmd.replace("%s", mlir_file)

        result = subprocess.run(
            cmd, shell=True, capture_output=True, text=True,
        )
        if result.returncode != 0:
            print("FAIL: %s" % mlir_file)
            print("  RUN: %s" % line.strip())
            if result.stdout:
                print("  stdout: %s" % result.stdout.strip())
            if result.stderr:
                print("  stderr: %s" % result.stderr.strip())
            passed = False

    if passed:
        print("PASS: %s" % mlir_file)
    return passed


def main():
    bir_opt = _find_tool("bir-opt")
    filecheck = _find_tool("FileCheck")

    substitutions = {
        "%bir-opt": bir_opt,
        "%FileCheck": filecheck,
    }

    test_dir = os.path.dirname(os.path.abspath(__file__))
    mlir_files = _find_mlir_files(test_dir)
    if not mlir_files:
        print("ERROR: No .mlir files found in %s" % test_dir, file=sys.stderr)
        sys.exit(1)

    failed = 0
    for f in mlir_files:
        if not _run_test(f, substitutions):
            failed += 1

    if failed:
        print("\n%d test(s) failed" % failed)
        sys.exit(1)
    print("\n%d test(s) passed" % len(mlir_files))


if __name__ == "__main__":
    main()

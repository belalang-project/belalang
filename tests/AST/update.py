#!/usr/bin/env python3
import os
import sys
import glob
import subprocess

def main():
    belalang_exe = os.path.abspath(sys.argv[1])
    workspace_dir = os.environ["BUILD_WORKSPACE_DIRECTORY"]
    tokens_dir = os.path.join(workspace_dir, "tests", "AST")

    for file_path in glob.glob(os.path.join(tokens_dir, "*.bel")):
        with open(file_path, "r", encoding="utf-8") as f:
            content = f.read()

        lines = content.splitlines()

        cmd = [belalang_exe, "build", "--emit=ast", file_path]
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)

        new_nodes = [line for line in result.stdout.splitlines() if line.strip()]

        new_lines = [line for line in lines if not line.lstrip().startswith("# CHECK")]
        while new_lines and not new_lines[-1].strip():
            new_lines.pop()
        new_lines.append("")

        new_lines.append(f"# CHECK:      {new_nodes[0]}")
        for token in new_nodes[1:]:
            new_lines.append(f"# CHECK-NEXT: {token}")

        with open(file_path, "w", encoding="utf-8") as f:
            f.write("\n".join(new_lines) + "\n")

if __name__ == "__main__":
    main()

#!/usr/bin/env python3
import os
import sys
import glob
import subprocess

def main():
    belalang_exe = os.path.abspath(sys.argv[1])
    workspace_dir = os.environ["BUILD_WORKSPACE_DIRECTORY"]
    tokens_dir = os.path.join(workspace_dir, "tests", "Tokens")

    for file_path in glob.glob(os.path.join(tokens_dir, "*.bel")):
        # FIXME: skip unclosed-string because of current limitations of string
        if os.path.basename(file_path) == "unclosed-string.bel":
            continue
        with open(file_path, "r", encoding="utf-8") as f:
            content = f.read()

        lines = content.splitlines()

        cmd = [belalang_exe, "build", "--emit=tokens", file_path]
        result = subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

        new_tokens = []
        for line in result.stdout.splitlines():
            if line.strip():
                line = line.replace(workspace_dir.rstrip("/"), "{{.*}}")
                new_tokens.append(line)

        new_lines = [line for line in lines if not line.lstrip().startswith("# CHECK:")]
        while new_lines and not new_lines[-1].strip():
            new_lines.pop()

        for token in new_tokens:
            new_lines.append(f"# CHECK: {token}")

        with open(file_path, "w", encoding="utf-8") as f:
            f.write("\n".join(new_lines) + "\n")

if __name__ == "__main__":
    main()

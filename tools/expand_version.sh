#!/bin/bash

set -e

TEMPLATE=$1
OUTPUT=$2
STATUS_FILE="bazel-out/stable-status.txt"

if [ -f "$STATUS_FILE" ]; then
    GIT_HASH=$(awk '/^STABLE_GIT_COMMIT/ {print $2}' "$STATUS_FILE")
fi

if [ -z "$GIT_HASH" ]; then
    GIT_HASH="unknown"
fi

sed "s/@GIT_HASH@/$GIT_HASH/g" "$TEMPLATE" > "$OUTPUT"

#!/bin/bash

echo "STABLE_GIT_COMMIT $(git rev-parse HEAD 2>/dev/null || echo unknown)"

#!/bin/bash

# Copy pre-commit hook if it doesn't exist
cp -n pre-commit .git/hooks/pre-commit

find netlib -iname '*.hpp' -o -iname '*.cpp' | xargs clang-format -Werror -i
echo "Formatting repository."

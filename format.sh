#!/bin/bash

# Copy pre-commit hook if it doesn't exist
cp -n pre-commit .git/hooks/pre-commit

echo "Formatting..."

# Format repository
find netlib -iname '*.hpp' -o -iname '*.cpp' | xargs clang-format -Werror -i
find app -iname '*.hpp' -o -iname '*.cpp' | xargs clang-format -Werror -i

echo "Formatted repository."
echo "If it didn't work, make sure you are running this script from the top-level Mercury directory."

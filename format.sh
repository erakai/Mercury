#!/bin/bash

# Copy pre-commit hook if it doesn't exist
file=".git/hooks/pre-commit"
if [ -f "$file" ] ; then
    rm ${file}
fi
cp -n pre-commit ${file}

# Format repository
find netlib -iname '*.h*' -o -iname '*.cpp' | xargs clang-format -Werror -i
find app -iname '*.h*' -o -iname '*.cpp' | xargs clang-format -Werror -i

echo "Formatting repository."

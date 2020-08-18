#!/bin/bash -xe

cd $(dirname $0)/..
git diff -U0 HEAD^ | clang-tidy-diff.py -p1 -path $BUILD_DIR -regex "core/.*\.hpp"
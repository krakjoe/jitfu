#!/bin/sh

set -e

cd $TRAVIS_BUILD_DIR

TEST_PHP_ARGS="-q --show-diff" make test

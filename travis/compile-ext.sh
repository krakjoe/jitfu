#!/bin/sh -x

set -e

cd $TRAVIS_BUILD_DIR

$HOME/bin/phpize

./configure --with-php-config=$HOME/bin/php-config --with-jitfu=$HOME

make -j2 --quiet

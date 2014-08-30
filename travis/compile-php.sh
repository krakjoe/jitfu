#!/usr/bin/env sh
set -e
mkdir -p $HOME/php
git clone --depth 1 https://github.com/php/php-src $HOME/php/src
git checkout PHP-5.6
cd $HOME/php/src
./buildconf --force
./configure --prefix=$HOME --disable-all --enable-debug
make -j2 --quiet install


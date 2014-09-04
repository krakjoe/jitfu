#!/usr/bin/env sh
set -e
mkdir -p $HOME/php
git clone https://github.com/php/php-src $HOME/php/src
cd $HOME/php/src
git checkout PHP-5.6
./buildconf --force
./configure \
    --prefix=$HOME \
    --disable-all \
    --enable-debug \
    --with-config-file-path=$HOME \
    --with-config-file-scan-dir=$HOME/php.d \
    --enable-json \
    --enable-phar \
    --enable-filter \
    --enable-hash \
    --enable-ctype \
    --enable-dom \
    --enable-tokenizer \
    --enable-libxml \
    --with-mysql=mysqlnd \
    --with-mysqli=mysqlnd \
    --with-openssl \
    --with-pdo-mysql
make -j2 --quiet install
cp php.ini-development $HOME
mkdir $HOME/php.d


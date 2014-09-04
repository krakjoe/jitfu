#!/bin/sh -x
set -e
$HOME/bin/phpize
./configure --with-php-config=$HOME/bin/php-config --with-jitfu=$HOME
make -j2 --quiet
make install
echo "extension=jitfu.so" > $HOME/php.d/jitfu.ini

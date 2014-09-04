#!/bin/sh
set -e
cd $HOME
git clone https://github.com/google/recki-ct
cd recki-ct
curl -sS https://getcomposer.org/installer | $HOME/bin/php
$HOME/bin/php composer.phar install --dev
$HOME/bin/php vendor/bin/phpunit test/Integration


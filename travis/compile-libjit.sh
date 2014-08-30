#!/usr/bin/env sh
set -e
git clone --depth 1 git://git.sv.gnu.org/libjit.git $HOME/libjit
cd $HOME/libjit
./auto_gen.sh
./configure --prefix=$HOME
make --quiet
make install

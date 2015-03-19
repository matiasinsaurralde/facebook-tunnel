#! /bin/sh
rm -rfv build
mkdir -p build && cd build
cmake ..
./configure
make

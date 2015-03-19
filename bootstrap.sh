#! /bin/sh
#cd gumbo
#./autogen.sh
#./configure
#make
#cd ..
rm -rfv build
mkdir -p build && cd build
cmake ..
./configure
make

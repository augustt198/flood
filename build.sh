#/usr/bin/env/sh
set -e

git clone https://github.com/nevali/uriparser.git
cd uriparser
./autogen.sh
./configure --disable-test
make
sudo make install
cd ..

make

#!/bin/bash
rm -rf build
mkdir build
cd build
cmake ..
make

pyenv local 3.5.1
virtualenv -p python3.5 guivenv
cd guivenv
. bin/activate
cp ../../gui/* .
pip install -r requirements.txt
cd ../..


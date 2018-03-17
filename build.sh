#!/bin/bash
deactivate
rm -rf build
mkdir build
cd build
cmake ..
make -j8
export LD_LIBRARY_PATH=/usr/xenomai/lib:$LD_LIBRARY_PATH

pyenv local 3.5.1
virtualenv -p python3.5 guivenv
cd guivenv
. bin/activate
#cp ../../gui/* .
ln -s ../../gui/control.kv control.kv
ln -s ../../gui/requirements.txt requirements.txt
ln -s ../../gui/ycoe_gui.py ycoe_gui.py
pip install -r requirements.txt
cd ../..


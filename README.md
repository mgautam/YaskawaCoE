# YaskawaCoE
Canopen over Ethercat stack is developed specifically for Yaskawa drive control.

Dependencies
------------

 * CMake 2.8.0 or later
 * Xilinx SDK 2017.3
 * Python 3.5.1 or later
 * Xenomai 3.0.5 on linux-4.9.38

Build Status
------------
[![Build Status](https://travis-ci.org/mgautam/YaskawaCoE.svg?branch=master)](https://travis-ci.org/mgautam/YaskawaCoE)

Building
--------
To build & install the project, run the following from this project's root
directory:
```bash
mkdir build
cd build
cmake ..
make
make install
```

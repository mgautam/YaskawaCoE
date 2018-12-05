cd libsodium-1.0.3
./configure
make
#sudo make install
cd ../zeromq-4.1.3
./configure
make
#sudo make install
#sudo ldconfig
cd ../osal
make
cd ../oshw
make
cd ../soem
make
cd ../ycoe
make
cd ..
make

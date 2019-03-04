#Install Libsodium
#cd libsodium-1.0.3/
#./configure
#make
#sudo make install
#cd ..

#Install Zeromq
#cd zeromq-4.1.3/
#./configure
#make
#sudo make install
#sudo ldconfig
#cd ..

cd osal
make
cd ../oshw
make
cd ../soem
make
cd ../ycoe
make
cd ..
make
gcc zmqsend.c -I./ycoe -I./soem -I./osal -lzmq ./ycoe/ycoe_math.o -lm -o zmqsend
gcc zmqgateway.c -I./ycoe -I./soem -I./osal -lzmq ./ycoe/ycoe_math.o -lm -o zmqgateway

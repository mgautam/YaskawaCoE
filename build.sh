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
gcc zmqsend.c -I./ycoe -I./soem -I./osal -lzmq ./ycoe/ycoe_math.o -lm

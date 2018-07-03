#include <stdio.h>
#include "ethercat.h"
#include "ycoetype.h"
#include "rio_574X0.h"
/*
int rio_setup_analog_inputs (int slavenum) {
    //USINT usintbuff;
    //int usintsize = USINT_SIZE;
    UINT uintbuff;
    int uintsize = UINT_SIZE;

    // Disable the assignment of Sync manager and PDO
    //usintbuff=0;
    //ec_SDOwrite(slavenum,0x1C12,0,0,USINT_SIZE,&usintbuff,EC_TIMEOUTRXM);

    uintbuff=3;
    ec_SDOwrite(slavenum,0x9003,1,0,UINT_SIZE,&uintbuff,EC_TIMEOUTRXM);
    //uintbuff=2;
    //ec_SDOwrite(slavenum,0x9003,2,0,UINT_SIZE,&uintbuff,EC_TIMEOUTRXM);

    // Enable the assignment of Sync manager and PDO
    //usintbuff=0;
    //ec_SDOwrite(slavenum,0x1C12,0,0,USINT_SIZE,&usintbuff,EC_TIMEOUTRXM);
    return 0;
}*/
int rio_set_digital_output(int slavenum, int io_port) {
    UINT *digital_outputs = (UINT *)ec_slave[slavenum].outputs;
    if (io_port < 1) return -1;// io_port is 1 indexed, but starts from 17
    *digital_outputs = *digital_outputs | ((1 << (io_port-1)) & 0xFFFF);
    return 0;
}

int rio_reset_digital_output(int slavenum, int io_port) {
    UINT *digital_outputs = (UINT *)ec_slave[slavenum].outputs;
    if (io_port < 1) return -1;// io_port is 1 indexed, but starts from 17
    *digital_outputs = *digital_outputs & ~((1 << (io_port-1)) & 0xFFFF);
    return 0;
}
int rio_toggle_dout(int slavenum, int io_port) {
    printf("Toggle dout:%d ",io_port);
    UINT *digital_outputs = (UINT *)ec_slave[slavenum].outputs;
    if (io_port < 1) return -1;// io_port is 1 indexed, but starts from 17

    if((*digital_outputs & (1 << (io_port - 1))) != 0) {
      printf("off\n\r");
      *digital_outputs = *digital_outputs & ~((1 << (io_port-1)) & 0xFFFF);
    }
    else {
      printf("on\n\r");
      *digital_outputs = *digital_outputs | ((1 << (io_port-1)) & 0xFFFF);
    }
    return 0;
}

int rio_set_analog_output(int slavenum, int io_port, UDINT value) {
    if (io_port < 1 || io_port > 8) return -1;
    UINT *analog_output = (UINT *)(ec_slave[slavenum].outputs+(4+((io_port-1)*2)));
    *analog_output = value;
    return 0;
}


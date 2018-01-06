#include <stdio.h>
#include "ethercat.h"
#include "yaskawacoe.h"

int ycoe_get_profile_position_parameters (void) {
    uint8 sintbuff;
    UDINT dintbuff;
    int sintsize = SINT_SIZE;
    int dintsize = DINT_SIZE;

    ec_SDOread(1,0x6060,0,0,&sintsize,&sintbuff,EC_TIMEOUTRXM);
    printf("Mode of Operation set to %d\r\n",sintbuff);
    ec_SDOread(1,0x6061,0,0,&sintsize,&sintbuff,EC_TIMEOUTRXM);
    printf("Mode of Operation Display: %d\r\n",sintbuff);

    ec_SDOread(1,0x6081,0,0,&dintsize,&dintbuff,EC_TIMEOUTRXM);
    printf("Profile Velocity: %x [vel. units](incs/sec)\r\n",dintbuff);
    ec_SDOread(1,0x607F,0,0,&dintsize,&dintbuff,EC_TIMEOUTRXM);
    printf("Max Profile Velocity: %d [vel. units](incs/sec)\r\n",dintbuff);

    ec_SDOread(1,0x6083,0,0,&dintsize,&dintbuff,EC_TIMEOUTRXM);
    printf("Profile Acceleration: %d [acc. units](incs/sec2)\r\n",dintbuff);
    ec_SDOread(1,0x6084,0,0,&dintsize,&dintbuff,EC_TIMEOUTRXM);
    printf("Profile Deceleration: %d [acc. units](incs/sec2)\r\n",dintbuff);
    ec_SDOread(1,0x6085,0,0,&dintsize,&dintbuff,EC_TIMEOUTRXM);
    printf("Quick Stop Deceleration: %d [acc. units](incs/sec2)\r\n",dintbuff);
}

/** \file
 * \brief Yaskawa CoE generic object 
 *
 * (c)Gautam Manoharan 2017 - 2018
 */

#include <stdio.h>
#include "ethercat.h"
#include "ycoetype.h"


int ycoe_get_mode_of_operation (int slavenum) {
    SINT sintbuff;
    int sintsize = SINT_SIZE;

    ec_SDOread(slavenum,0x6060,0,0,&sintsize,&sintbuff,EC_TIMEOUTRXM);
    printf("Mode of Operation set to %d\r\n",sintbuff);
    ec_SDOread(slavenum,0x6061,0,0,&sintsize,&sintbuff,EC_TIMEOUTRXM);
    printf("Mode of Operation Display: %d\r\n",sintbuff);

    return 0;
}


int ycoe_set_mode_of_operation (int slavenum, SINT mode_of_operation) {
    ec_SDOwrite(slavenum,0x6060,0,0,SINT_SIZE,&mode_of_operation,EC_TIMEOUTRXM);
    return 0;
}



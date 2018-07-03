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

UDINT ycoe_get_vendor_id (int slavenum) {
    UDINT udintbuff;
    int udintsize = UDINT_SIZE;
    ec_SDOread(slavenum,0x1018,1,0,&udintsize,&udintbuff,EC_TIMEOUTRXM);
    printf("Slave %d VendorID %x\n\r",slavenum, udintbuff);
    return udintbuff;
}

UDINT ycoe_get_product_code (int slavenum) {
    UDINT udintbuff;
    int udintsize = UDINT_SIZE;
    ec_SDOread(slavenum,0x1018,2,0,&udintsize,&udintbuff,EC_TIMEOUTRXM);
    printf("Slave %d ProductCode %x\n\r",slavenum, udintbuff);
    return udintbuff;
}

void ycoe_print_identity (int slavenum) {
    printf("Slave: %d VendorID:%x ProductCode:%x\n\r",slavenum, ycoe_get_vendor_id(slavenum), ycoe_get_product_code(slavenum));
}

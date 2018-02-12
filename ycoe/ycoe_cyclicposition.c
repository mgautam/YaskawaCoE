/** \file
 * \brief YaskawaCoE Cyclic Synchronous Position mode specific access
 *
 * (c)Gautam Manoharan 2017 - 2018
 */

#include <stdio.h>
#include "ethercat.h"
#include "ycoetype.h"
#include "ycoe_registers.h"
#include "ycoe_cyclicposition.h"

int ycoe_csp_setup(int slavenum) {
    //USINT usintbuff;
    //UINT  uintbuff;
    //UDINT udintbuff;

    printf("Slave:%d CoE State: %x\n\r",slavenum,ycoe_readreg_int(slavenum, 0x130));

    /* Enable DC Mode with Sync0 Generation */
    ec_dcsync0(slavenum,1,2000000,0);//CycleTime=2ms, CycleShift=0
    printf("Slave:%d CoE State: %x\n\r",slavenum,ycoe_readreg_int(slavenum, 0x130));
    return 0;
}


int ycoe_csp_checkstatus (int slavenum, UINT targetstatus) {
  UINT *statusword = (UINT *)(ec_slave[slavenum].inputs+2);
  int retval = 0;

  if (targetstatus & *statusword) retval++;
  return retval;
}

int ycoe_csp_get_parameters (int slavenum) {
    UDINT udintbuff;
    int udintsize = UDINT_SIZE;
    USINT usintbuff;
    int usintsize = USINT_SIZE;
    SINT sintbuff;
    int sintsize = SINT_SIZE;

    ec_SDOread(slavenum,0x60B1,0,0,&udintsize,&udintbuff,EC_TIMEOUTRXM);
    printf("Velocity Offset: %d [vel. units](incs/sec)\r\n",udintbuff);
    ec_SDOread(slavenum,0x60B2,0,0,&udintsize,&udintbuff,EC_TIMEOUTRXM);
    printf("Torque Offset: %d [units](incs/sec2)\r\n",udintbuff);

    ec_SDOread(slavenum,0x60C2,1,0,&usintsize,&usintbuff,EC_TIMEOUTRXM);
    printf("Interpolation time period: %d \r\n",usintbuff);
    ec_SDOread(slavenum,0x60C2,2,0,&sintsize,&sintbuff,EC_TIMEOUTRXM);
    printf("Interpolation time index: %d \r\n",sintbuff);

    ec_SDOread(slavenum,0x6084,0,0,&udintsize,&udintbuff,EC_TIMEOUTRXM);
    printf("Profile Deceleration: %d [acc. units](incs/sec2)\r\n",udintbuff);
    ec_SDOread(slavenum,0x6085,0,0,&udintsize,&udintbuff,EC_TIMEOUTRXM);
    printf("Quick Stop Deceleration: %d [acc. units](incs/sec2)\r\n",udintbuff);

    return 0;
}

int ycoe_csp_set_veloffset (int slavenum, UDINT velocity_offset) {
    ec_SDOwrite(slavenum,0x60B1,0,0,UDINT_SIZE,&velocity_offset,EC_TIMEOUTRXM);
    return 0;
}
int ycoe_csp_set_trqoffset (int slavenum, UDINT torque_offset) {
    ec_SDOwrite(slavenum,0x60B2,0,0,UDINT_SIZE,&torque_offset,EC_TIMEOUTRXM);
    return 0;
}
int ycoe_csp_set_deceleration (int slavenum, UDINT profile_deceleration) {
    ec_SDOwrite(slavenum,0x6084,0,0,UDINT_SIZE,&profile_deceleration,EC_TIMEOUTRXM);
    return 0;
}
int ycoe_csp_set_quick_stop_deceleration (int slavenum, UDINT quick_stop_deceleration) {
    ec_SDOwrite(slavenum,0x6085,0,0,UDINT_SIZE,&quick_stop_deceleration,EC_TIMEOUTRXM);
    return 0;
}

int ycoe_csp_set_parameters (int slavenum, UDINT velocity_offset, UDINT torque_offset, UDINT profile_deceleration, UDINT quick_stop_deceleration) {
    ycoe_csp_set_veloffset (slavenum, velocity_offset);
    ycoe_csp_set_trqoffset (slavenum, torque_offset);
    ycoe_csp_set_deceleration (slavenum,profile_deceleration);
    ycoe_csp_set_quick_stop_deceleration (slavenum,quick_stop_deceleration);

    return 0;
}

int ycoe_csp_set_position (int slavenum, DINT target_position) {
    DINT *position_pdo_address = (DINT *)(ec_slave[slavenum].outputs+2);
    *position_pdo_address = target_position;
    return 0;
}
int ycoe_csp_add_position (int slavenum, DINT position) {
    DINT *target_position_pdo = (DINT *)(ec_slave[slavenum].outputs+2);
    DINT *current_position_pdo = (DINT *)(ec_slave[slavenum].inputs+2);
    *target_position_pdo = position + *current_position_pdo;

    return 0;
}
int ycoe_csp_goto_position (int slavenum, DINT target_position) {
    DINT *current_position_pdo = (DINT *)(ec_slave[slavenum].inputs+2);
    DINT *target_position_pdo = (DINT *)(ec_slave[slavenum].outputs+2);

    DINT velocity = 1600000; /* 1600000 counts per 2ms */
    //if (ycoe_csp_checkstatus(slavenum, SW_CSP_TARGET_REACHED)) {
      if ((target_position - *current_position_pdo) > velocity) {
        *target_position_pdo = *current_position_pdo + velocity;
        //printf("Goto target request: %d\n\r", *target_position_pdo);
        return 0;
      } else if ((target_position - *current_position_pdo) < -velocity) {
        *target_position_pdo = *current_position_pdo - velocity;
        //printf("Goto target request: %d\n\r", *target_position_pdo);
        return 0;
      } else {
        *target_position_pdo = target_position;
        //printf("Goto target request: %d\n\r", *target_position_pdo);
        return 1;
      }
    //}
}
int ycoe_csp_goto_possync (int slavenum, DINT target_position) {
    DINT *current_position_pdo1 = (DINT *)(ec_slave[1].inputs+2);
    DINT *current_position_pdo2 = (DINT *)(ec_slave[2].inputs+2);
    DINT current_position_pdo;
    if (*current_position_pdo1 > *current_position_pdo2) current_position_pdo = *current_position_pdo2;
    else current_position_pdo = *current_position_pdo1;

    DINT *target_position_pdo = (DINT *)(ec_slave[slavenum].outputs+2);

    DINT velocity = 1600000; /* 1600000 counts per 2ms */
    //if (ycoe_csp_checkstatus(slavenum, SW_CSP_TARGET_REACHED)) {
      if ((target_position - current_position_pdo) > velocity) {
        *target_position_pdo = current_position_pdo + velocity;
        //printf("Goto target request: %d\n\r", *target_position_pdo);
        return 0;
      } else if ((target_position - current_position_pdo) < -velocity) {
        *target_position_pdo = current_position_pdo - velocity;
        //printf("Goto target request: %d\n\r", *target_position_pdo);
        return 0;
      } else {
        *target_position_pdo = target_position;
        //printf("Goto target request: %d\n\r", *target_position_pdo);
        return 1;
      }
    //}
}

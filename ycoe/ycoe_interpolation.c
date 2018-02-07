/** \file
 * \brief YaskawaCoE Interpolation mode specific access
 *
 * (c)Gautam Manoharan 2017 - 2018
 */

#include <stdio.h>
#include "ethercat.h"
#include "ycoetype.h"
#include "ycoe_registers.h"
#include "ycoe_interpolation.h"

int ycoe_ipm_setup(int slavenum) {
    USINT usintbuff;
    UINT  uintbuff;
    UDINT udintbuff;

    printf("Slave:%d CoE State: %x\n\r",slavenum,ycoe_readreg_int(slavenum, 0x130));

    /* Disable the assignment of Sync manager and PDO */
    usintbuff=0;
    ec_SDOwrite(slavenum,0x1C12,0,0,USINT_SIZE,&usintbuff,EC_TIMEOUTRXM);
    ec_SDOwrite(slavenum,0x1C13,0,0,USINT_SIZE,&usintbuff,EC_TIMEOUTRXM);

    /* Control Word mapping entry for RxPDO */
    udintbuff=0x60400010;
    ec_SDOwrite(slavenum,0x1602,1,0,UDINT_SIZE,&udintbuff,EC_TIMEOUTRXM);
    /* Set Interpolation data for target position in RxPDO */
    udintbuff=0x60C10120;
    ec_SDOwrite(slavenum,0x1602,2,0,UDINT_SIZE,&udintbuff,EC_TIMEOUTRXM);
    /* Set Number of Mapping Entries */
    usintbuff=2;
    ec_SDOwrite(slavenum,0x1602,0,0,USINT_SIZE,&usintbuff,EC_TIMEOUTRXM);

    /* Status Word mapping entry for TxPDO */
    udintbuff=0x60410010;
    ec_SDOwrite(slavenum,0x1A02,1,0,UDINT_SIZE,&udintbuff,EC_TIMEOUTRXM);
    /* Set current position in TxPDO */
    udintbuff=0x60640020;
    ec_SDOwrite(slavenum,0x1A02,2,0,UDINT_SIZE,&udintbuff,EC_TIMEOUTRXM);
    /* Set Number of Mapping Entries */
    usintbuff=2;
    ec_SDOwrite(slavenum,0x1A02,0,0,USINT_SIZE,&usintbuff,EC_TIMEOUTRXM);

    /* Assignment of Sync manager and PDO */
    uintbuff=0x1602;
    ec_SDOwrite(slavenum,0x1C12,1,0,UINT_SIZE,&uintbuff,EC_TIMEOUTRXM);
    uintbuff=0x1A02;
    ec_SDOwrite(slavenum,0x1C13,1,0,UINT_SIZE,&uintbuff,EC_TIMEOUTRXM);

    /* Enable the assignment of Sync manager and PDO */
    usintbuff=1;
    ec_SDOwrite(slavenum,0x1C12,0,0,USINT_SIZE,&usintbuff,EC_TIMEOUTRXM);
    ec_SDOwrite(slavenum,0x1C13,0,0,USINT_SIZE,&usintbuff,EC_TIMEOUTRXM);

    /* Enable DC Mode with Sync0 Generation */
    ec_dcsync0(slavenum,1,4000000,0);//CycleTime=4ms, CycleShift=0
    printf("Slave:%d CoE State: %x\n\r",slavenum,ycoe_readreg_int(slavenum, 0x130));
    return 0;
}


int ycoe_ipm_checkcontrol (int slavenum, UINT targetcontrol) {
  UINT *controlword = (UINT *)ec_slave[slavenum].outputs;
  int retval = 0;

    if ((targetcontrol == CW_IPM_ENABLE) || (targetcontrol == CW_IPM_DISABLE)) {
      if ((*controlword & CW_IPM_MASK_BIT4) == targetcontrol)
        retval++;
    }

  return retval;
}
int ycoe_ipm_checkstatus (int slavenum, UINT targetstatus) {
  UINT *statusword = (UINT *)(ec_slave[slavenum].inputs+2);
  int retval = 0;

  if (targetstatus & *statusword) retval++;
  return retval;
}

int ycoe_ipm_get_parameters (int slavenum) {
    UDINT udintbuff;
    int udintsize = UDINT_SIZE;
    USINT usintbuff;
    int usintsize = USINT_SIZE;
    SINT sintbuff;
    int sintsize = SINT_SIZE;

    ec_SDOread(slavenum,0x60C1,1,0,&udintsize,&udintbuff,EC_TIMEOUTRXM);
    printf("Interpolation Data: %d [pos. units](incs)\r\n",udintbuff);
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

int ycoe_ipm_set_deceleration (int slavenum, UDINT profile_deceleration) {
    ec_SDOwrite(slavenum,0x6084,0,0,UDINT_SIZE,&profile_deceleration,EC_TIMEOUTRXM);
    return 0;
}
int ycoe_ipm_set_quick_stop_deceleration (int slavenum, UDINT quick_stop_deceleration) {
    ec_SDOwrite(slavenum,0x6085,0,0,UDINT_SIZE,&quick_stop_deceleration,EC_TIMEOUTRXM);
    return 0;
}

int ycoe_ipm_set_parameters (int slavenum, UDINT profile_deceleration, UDINT quick_stop_deceleration) {
    ycoe_ipm_set_deceleration (slavenum,profile_deceleration);
    ycoe_ipm_set_quick_stop_deceleration (slavenum,quick_stop_deceleration);

    return 0;
}

int ycoe_ipm_set_position (int slavenum, DINT target_position) {
    DINT *position_pdo_address = (DINT *)(ec_slave[slavenum].outputs+2);
    *position_pdo_address = target_position;

    ec_SDOwrite(slavenum,0x60C1,1,0,DINT_SIZE,&target_position,EC_TIMEOUTRXM);

    return 0;
}
int ycoe_ipm_add_position (int slavenum, DINT position) {
    DINT *target_position_pdo = (DINT *)(ec_slave[slavenum].outputs+2);
    DINT *current_position_pdo = (DINT *)(ec_slave[slavenum].inputs+2);
    *target_position_pdo = position + *current_position_pdo;

    ec_SDOwrite(slavenum,0x60C1,1,0,DINT_SIZE,target_position_pdo,EC_TIMEOUTRXM);

    return 0;
}
int ycoe_ipm_goto_position (int slavenum, DINT target_position) {
    DINT *current_position_pdo = (DINT *)(ec_slave[slavenum].inputs+2);
    DINT *target_position_pdo = (DINT *)(ec_slave[slavenum].outputs+2);

    DINT velocity = 20000; /* 2000 counts per 4ms */
    if ((target_position - *current_position_pdo) > velocity) {
        *target_position_pdo = *current_position_pdo + velocity;
        ec_SDOwrite(slavenum,0x60C1,1,0,DINT_SIZE,target_position_pdo,EC_TIMEOUTRXM);
        //printf("Goto target request: %d\n\r", *target_position_pdo);
        return 0;
    } else if ((target_position - *current_position_pdo) < -velocity) {
        *target_position_pdo = *current_position_pdo - velocity;
        ec_SDOwrite(slavenum,0x60C1,1,0,DINT_SIZE,target_position_pdo,EC_TIMEOUTRXM);
        //printf("Goto target request: %d\n\r", *target_position_pdo);
        return 0;
    } else {
        *target_position_pdo = target_position;
        ec_SDOwrite(slavenum,0x60C1,1,0,DINT_SIZE,target_position_pdo,EC_TIMEOUTRXM);
        //printf("Goto target request: %d\n\r", *target_position_pdo);
        return 1;
    }
}

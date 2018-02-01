#include <stdio.h>
#include "ethercat.h"
#include "ycoetype.h"
#include "ycoe_registers.h"
#include "ycoe_profile_position.h"

int ycoe_ppm_setup(int slavenum) {
    USINT usintbuff;
    int usintsize = USINT_SIZE;
    UDINT udintbuff;
    int udintsize = UDINT_SIZE;

    /* Disable the assignment of Sync manager and PDO */
    usintbuff=0;
    ec_SDOwrite(slavenum,0x1C12,0,0,USINT_SIZE,&usintbuff,EC_TIMEOUTRXM);
    ec_SDOwrite(slavenum,0x1C13,0,0,USINT_SIZE,&usintbuff,EC_TIMEOUTRXM);

    /* Set Target Position for target position in RxPDO */
    udintbuff=0x607A0020;
    ec_SDOwrite(slavenum,0x1601,2,0,UDINT_SIZE,&udintbuff,EC_TIMEOUTRXM);

    /* Enable the assignment of Sync manager and PDO */
    usintbuff=1;
    ec_SDOwrite(slavenum,0x1C12,0,0,USINT_SIZE,&usintbuff,EC_TIMEOUTRXM);
    ec_SDOwrite(slavenum,0x1C13,0,0,USINT_SIZE,&usintbuff,EC_TIMEOUTRXM);

    /* Enable DC Mode with Sync0 Generation */
    ycoe_writereg(slavenum, 0x980, UINT_SIZE, 0x0000);
    return 0;
}



int ycoe_ppm_checkcontrol (int slavenum, UINT targetcontrol) {
  UINT *controlword = (UINT *)ec_slave[slavenum].outputs;
  int retval = 0;

  //if (targetcontrol & CW_PPM_MASK_SNPI == CW_PPM_SNPI1 || targetcontrol & CW_PPM_MASK_SNPI == CW_PPM_SNPI2)
    if ((targetcontrol == CW_PPM_SNPI1) || (targetcontrol == CW_PPM_SNPI2)) {
      if ((*controlword & CW_PPM_MASK_SNPI) == targetcontrol)
        retval++;
    }

  return retval;
}
int ycoe_ppm_checkstatus (int slavenum, UINT targetstatus) {
  UINT *statusword = (UINT *)(ec_slave[slavenum].inputs+2);
  int retval = 0;

  if (targetstatus & *statusword) retval++;
  return retval;
}

int ycoe_ppm_get_parameters (void) {
    UDINT udintbuff;
    int udintsize = UDINT_SIZE;

    ec_SDOread(1,0x6081,0,0,&udintsize,&udintbuff,EC_TIMEOUTRXM);
    printf("Profile Velocity: %x [vel. units](incs/sec)\r\n",udintbuff);
    ec_SDOread(1,0x607F,0,0,&udintsize,&udintbuff,EC_TIMEOUTRXM);
    printf("Max Profile Velocity: %d [vel. units](incs/sec)\r\n",udintbuff);

    ec_SDOread(1,0x6083,0,0,&udintsize,&udintbuff,EC_TIMEOUTRXM);
    printf("Profile Acceleration: %d [acc. units](incs/sec2)\r\n",udintbuff);
    ec_SDOread(1,0x6084,0,0,&udintsize,&udintbuff,EC_TIMEOUTRXM);
    printf("Profile Deceleration: %d [acc. units](incs/sec2)\r\n",udintbuff);
    ec_SDOread(1,0x6085,0,0,&udintsize,&udintbuff,EC_TIMEOUTRXM);
    printf("Quick Stop Deceleration: %d [acc. units](incs/sec2)\r\n",udintbuff);

    return 0;
}

int ycoe_ppm_set_velocity (UDINT profile_velocity) {
    ec_SDOwrite(1,0x6081,0,0,UDINT_SIZE,&profile_velocity,EC_TIMEOUTRXM);
    return 0;
}
int ycoe_ppm_set_acceleration (UDINT profile_acceleration) {
    ec_SDOwrite(1,0x6083,0,0,UDINT_SIZE,&profile_acceleration,EC_TIMEOUTRXM);
    return 0;
}
int ycoe_ppm_set_deceleration (UDINT profile_deceleration) {
    ec_SDOwrite(1,0x6084,0,0,UDINT_SIZE,&profile_deceleration,EC_TIMEOUTRXM);
    return 0;
}
int ycoe_ppm_set_quick_stop_deceleration (UDINT quick_stop_deceleration) {
    ec_SDOwrite(1,0x6085,0,0,UDINT_SIZE,&quick_stop_deceleration,EC_TIMEOUTRXM);
    return 0;
}

int ycoe_ppm_set_parameters (UDINT profile_velocity, UDINT profile_acceleration, UDINT profile_deceleration, UDINT quick_stop_deceleration) {
    ycoe_ppm_set_velocity (profile_velocity);
    ycoe_ppm_set_acceleration (profile_acceleration);
    ycoe_ppm_set_deceleration (profile_deceleration);
    ycoe_ppm_set_quick_stop_deceleration (quick_stop_deceleration);

    return 0;
}

int ycoe_set_slave_position (int slavenum, DINT position) {
    DINT *position_pdo_address = (DINT *)(ec_slave[slavenum].outputs+2);
    *position_pdo_address = position;
    return 0;
}

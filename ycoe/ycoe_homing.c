/** \file
 * \brief YaskawaCoE Profile Position mode specific access
 *
 * (c)Gautam Manoharan 2017 - 2018
 */

#include <stdio.h>
#include "ethercat.h"
#include "ycoetype.h"
#include "ycoe_registers.h"
#include "ycoe_homing.h"

int ycoe_homing_setup(int slavenum, UDINT home_offset, UDINT acceleration, UDINT search_switch_speed, UDINT search_zero_speed, SINT homing_method) {
    /* Set Homing Offset */
    ec_SDOwrite(slavenum,0x607C,0,0,UDINT_SIZE,&home_offset,EC_TIMEOUTRXM);

    /* Set Homing Acceleration */
    ec_SDOwrite(slavenum,0x609A,0,0,UDINT_SIZE,&acceleration,EC_TIMEOUTRXM);

    /* Set Homing Speeds */
    // Speed during search for switch
    ec_SDOwrite(slavenum,0x6099,1,0,UDINT_SIZE,&search_switch_speed,EC_TIMEOUTRXM);
    // Speed during serach for zero
    ec_SDOwrite(slavenum,0x6099,2,0,UDINT_SIZE,&search_zero_speed,EC_TIMEOUTRXM);

   /* Set Homing Method */
    ec_SDOwrite(slavenum,0x6098,0,0,USINT_SIZE,&homing_method,EC_TIMEOUTRXM);

    return 0;
}

int ycoe_home_at_curpos (int slavenum) {
  // Homing on current position Method Value = 35
  return ycoe_homing_setup(slavenum, 0,0,0,0,35);
}

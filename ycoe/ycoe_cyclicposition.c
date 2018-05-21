/** \file
 * \brief YaskawaCoE Cyclic Synchronous Position mode specific access
 *
 * (c)Gautam Manoharan 2017 - 2018
 */

#include <stdio.h>
#include "ethercat.h"
#include "ycoetype.h"
#include "ycoe_registers.h"
#include "ycoe_math.h"
#include "ycoe_cyclicposition.h"

int ycoe_csp_setup(int slavenum) {
    //USINT usintbuff;
    //UINT  uintbuff;
    //UDINT udintbuff;

    //printf("Slave:%d CoE State: %x\n\r",slavenum,ycoe_readreg_int(slavenum, 0x130));

    /* Enable DC Mode with Sync0 Generation */
    ec_dcsync0(slavenum,1,2000000,0);//CycleTime=2ms, CycleShift=0
    //printf("Slave:%d CoE State: %x\n\r",slavenum,ycoe_readreg_int(slavenum, 0x130));
    return 0;
}


int ycoe_csp_checkstatus (int slavenum, UINT targetstatus) {
  UINT *statusword = (UINT *)(ec_slave[slavenum].inputs);
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

/* Square root integer approximation */
static UDINT sqroot(UDINT x){
  UDINT a,b;
  b = x;
  a = x = 0x3f;
  x = b/x;
  a = x = (x+a)>>1;
  x = b/x;
  a = x = (x+a)>>1;
  x = b/x;
  a = x = (x+a)>>1;
  x = b/x;
  a = x = (x+a)>>1;
  x = b/x;
  a = x = (x+a)>>1;
  x = b/x;
  x = (x+a)>>1;
  return(x);
}


static DINT acceleration = 320; //=3129rpm/5sec
static DINT acceleration50 = 16000; //=acceleration*50
static DINT peak_velocity[3] = {0};
static DINT ramp_distance[3] = {0};
static DINT target_position[3] = {0};
int ycoe_csp_set_position (int slavenum, DINT final_position) {
    DINT *current_position = (DINT *)(ec_slave[slavenum].inputs+2);
    target_position[slavenum] = final_position;

    DINT max_2ramp_distance = 547000000;//273500000*2;//(actual_velocity^2)/(2*actual_acceleration);//(109400^2)/(2*21.88);
    DINT position_difference = final_position - *current_position;
    if (position_difference < 0) position_difference = -position_difference;
    if (position_difference > max_2ramp_distance)
    {
      peak_velocity[slavenum] = 1600000;
      ramp_distance[slavenum] = 273500000;
    } else {
      // Scale all the square roots up by 16 times then reduce it after final multiplication for integer method
      peak_velocity[slavenum] = sqroot(acceleration << 4) << 2;//*sqrt(1600000 / 109400)//Scaled up by 16 before sqroot
      // sqroot approximation is less than 1unit of 0-1048576
      if (position_difference > 1048576) {
        peak_velocity[slavenum] *= (sqroot(position_difference >> 16) << 10);//Scaled up by 16 before sqroot
      }
      else
      {
        peak_velocity[slavenum] *= sqroot(position_difference << 4);//Scaled up by 16 before sqroot
      }
      peak_velocity[slavenum] = peak_velocity[slavenum] >> 4;//reducing finally
      ramp_distance[slavenum] = position_difference >> 1;
    }
    //printf("Peak velocity %d=%d\n\r",slavenum, peak_velocity[slavenum]);
    return 0;
}
static DINT demand_velocity[3] = {0};
static DINT previous_position[3] = {0};
void ycoe_csp_accel_ramp (int slavenum, DINT target_velocity) {
    DINT *current_position_pdo = (DINT *)(ec_slave[slavenum].inputs+2);

    if (previous_position[slavenum] != *current_position_pdo) {
      if ((target_velocity - demand_velocity[slavenum]) > acceleration) {
        demand_velocity[slavenum] += acceleration;
      } else if ((target_velocity - demand_velocity[slavenum]) < -acceleration) {
        demand_velocity[slavenum] -= acceleration;
      } else {
        demand_velocity[slavenum] = target_velocity;
      }
      //printf("demand_velocity=%d\n\r",demand_velocity[slavenum]);
    }
    previous_position[slavenum] = *current_position_pdo;
}
int ycoe_csp_goto_possync (int slavenum) {
    DINT *current_position_pdo1 = (DINT *)(ec_slave[1].inputs+2);
    DINT *current_position_pdo2 = (DINT *)(ec_slave[2].inputs+2);
    DINT current_position_pdo;
    //current_position_pdo = *(DINT *)(ec_slave[slavenum].inputs+2);

    // Sync both axes with the lowest position reached
    if (*current_position_pdo1 > *current_position_pdo2) current_position_pdo = *current_position_pdo2;
    else current_position_pdo = *current_position_pdo1;

    DINT *target_position_pdo = (DINT *)(ec_slave[slavenum].outputs+2);
    if ((target_position[slavenum] - current_position_pdo) > ramp_distance[slavenum]) {
      ycoe_csp_accel_ramp(slavenum,peak_velocity[slavenum]);
      *target_position_pdo = current_position_pdo + demand_velocity[slavenum];
      //printf("Goto target request: %d\n\r", *target_position_pdo);
      return 0;
    } else if ((target_position[slavenum] - current_position_pdo) < -ramp_distance[slavenum]) {
      ycoe_csp_accel_ramp(slavenum,-peak_velocity[slavenum]);
      *target_position_pdo = current_position_pdo + demand_velocity[slavenum];
      //printf("Goto target request: %d\n\r", *target_position_pdo);
      return 0;
    } else {
      if ((target_position[slavenum] - current_position_pdo) > acceleration50) {
         ycoe_csp_accel_ramp(slavenum, acceleration50);
        *target_position_pdo = current_position_pdo + demand_velocity[slavenum];
        return 0;
      }
      else if ((target_position[slavenum] - current_position_pdo) < -acceleration50) {
         ycoe_csp_accel_ramp(slavenum, -acceleration50);
        *target_position_pdo = current_position_pdo + demand_velocity[slavenum];
        return 0;
      }
      else {
        *target_position_pdo = target_position[slavenum];
        demand_velocity[slavenum] = 0; // motor is going to reach target so set demand velocity to 0
        return 1;
      }
      //printf("Goto target request: %d\n\r", *target_position_pdo);
    }
}


//extern DINT **position_array;
static DINT **position_array;
//extern unsigned int period_in_cycles;
static unsigned int period_in_cycles = 0;
static unsigned int position_index = 0;
static DINT start_position[3] = {0};
int ycoe_csp_setup_posarray(int num_slaves, unsigned int samples_per_second, unsigned int period_in_secs) {
  position_array = malloc((num_slaves+1) * sizeof(DINT *));

  period_in_cycles = samples_per_second * period_in_secs;
  int i;
  for (i=1; i<=num_slaves; i++)
    position_array[i] = (DINT *) malloc(sizeof(DINT) * period_in_cycles);

  // Circular Interpolation
  sinfill(position_array[1], 6400000.0, period_in_cycles);//6400000=100000counts/s
  sinfill(position_array[2], 6400000.0, period_in_cycles);// 800000=12500counts/s

  // Other graphs can be used to fill these position arrays
  // The two arrays have to be equal in size 
  // also keep in mind the max velocities and accelerations

  return 0;
}
int ycoe_csp_follow_posarray (int slavenum) {
    DINT *current_position_pdo1 = (DINT *)(ec_slave[1].inputs+2);
    DINT *current_position_pdo2 = (DINT *)(ec_slave[2].inputs+2);

    DINT tolerance = 10;//counts

    // Sync both axes with the position array
    /*if (((((*current_position_pdo1 - position_array[1][position_index]) < tolerance) && (*current_position_pdo1 > position_array[1][position_index])) ||
         (((*current_position_pdo1 - position_array[1][position_index]) > -tolerance) && (*current_position_pdo1 < position_array[1][position_index]))) &&
        ((((*current_position_pdo2 - position_array[2][position_index]) < tolerance) && (*current_position_pdo2 > position_array[2][position_index])) ||
         (((*current_position_pdo2 - position_array[2][position_index]) > -tolerance) && (*current_position_pdo2 < position_array[2][position_index]))))*/
    {
      position_index++;
      if (position_index >= period_in_cycles) position_index = 0;
    }

    DINT *target_position_pdo1 = (DINT *)(ec_slave[1].outputs+2);
    *target_position_pdo1 = position_array[1][position_index];

    DINT *target_position_pdo2 = (DINT *)(ec_slave[2].outputs+2);
    *target_position_pdo2 = position_array[2][position_index];

    return 0;
}

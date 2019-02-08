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
    ec_dcsync0(slavenum,1,4000000,0);//CycleTime=4ms, CycleShift=0
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

    if (*current_position_pdo1 > *current_position_pdo2) current_position_pdo = *current_position_pdo2;
    else current_position_pdo = *current_position_pdo1;

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
static DINT **position_array[2];
static DINT *prev_slave_pos[2];
static int *position_index[2];
static int fillposarr,runposarr, continuerun=0;
//extern unsigned int period_in_cycles;
static unsigned int period_in_cycles = 0;
int ycoe_csp_setup_posarray(int num_slaves, unsigned int max_array_len) {
    int i,j;
    //for (i=1; i<=2; i++) {
      position_index[0] = malloc((num_slaves+1) * sizeof(INT));
      prev_slave_pos[0] = malloc((num_slaves+1) * sizeof(DINT));
      position_array[0] = malloc((num_slaves+1) * sizeof(DINT *));
      for (j=1; j<=num_slaves; j++) {
        position_index[0][j]=-1;
        position_array[0][j] = (DINT *) malloc(sizeof(DINT) * max_array_len);
       }
position_index[1] = malloc((num_slaves+1) * sizeof(INT));
      prev_slave_pos[1] = malloc((num_slaves+1) * sizeof(DINT));
      position_array[1] = malloc((num_slaves+1) * sizeof(DINT *));
      for (j=1; j<=num_slaves; j++) {
        position_index[1][j]=-1;
        position_array[1][j] = (DINT *) malloc(sizeof(DINT) * max_array_len);
       }
    //}
    fillposarr=-1;
    runposarr=0;
}
int ycoe_csp_fill_posarray (int num_slaves, int posarr_len, DINT *pos_array) {
    int i;
    period_in_cycles = posarr_len;

    if (runposarr == 0) fillposarr = 1;
    else fillposarr = 0;

    for (i=1; i<=num_slaves; i++) {
      DINT *cur_slave_pos;
      if (position_index[runposarr][i]==-1)
        cur_slave_pos = (DINT *)(ec_slave[i].inputs+2);
      else
        cur_slave_pos = &prev_slave_pos[runposarr][i];

      int j;
      for (j=0; j<period_in_cycles; j++)
        position_array[fillposarr][i][j]=*cur_slave_pos + pos_array[(i-1)*period_in_cycles+j];

      prev_slave_pos[fillposarr][i] = *cur_slave_pos;
      // Reset Position Index for Follow_PosArray
      position_index[fillposarr][i] = 0;
    }
/*        if (position_index[runposarr][1]<0 && \
position_index[runposarr][2]<0 && \
position_index[runposarr][3]<0 && \
position_index[runposarr][4]<0)
{	//	runposarr= fillposarr;
	printf("1Fill:%d Run:%d\n",fillposarr,runposarr);
}*/
	continuerun=1;
//	printf("2Fill:%d Run:%d\n",fillposarr,runposarr);
}
int ycoe_csp_follow_posarray (int slaveindex) {
    if (position_index[runposarr][slaveindex] >= 0) {
      if (position_index[runposarr][slaveindex] < period_in_cycles) {
        position_index[runposarr][slaveindex]++;

        int i;
          DINT *target_position_pdo = (DINT *)(ec_slave[slaveindex].outputs+2);
          *target_position_pdo = position_array[runposarr][slaveindex][position_index[runposarr][slaveindex]];
      } else {
        position_index[runposarr][slaveindex] = -1;

//	printf("Runposarr=%d\n",runposarr);
      }
    }
    else
    {
        if (continuerun==1) {
		runposarr= (runposarr==0)?1:0;
		continuerun=0;
	}//	printf("Runposarr=%d\n",runposarr);
    }
return 0;
}
int ycoe_csp_posindicies(int numslaves){
    int i;
    printf("Posindices: %d:\t", runposarr);
    for (i=1; i<numslaves+1; i++)
       printf("%d ", position_index[runposarr][i]);
    printf("\n");
    return numslaves;
}

int ycoe_csp_setup_sinarray(int num_slaves, unsigned int samples_per_second, unsigned int period_in_secs) {
  period_in_cycles = samples_per_second * period_in_secs;
  ycoe_csp_setup_posarray(num_slaves, period_in_cycles);

  DINT *cur_slave1_pos = (DINT *)(ec_slave[1].inputs+2);
  DINT *cur_slave2_pos = (DINT *)(ec_slave[2].inputs+2);

  // Circular Interpolation
  DINT *_pos_arr = malloc(num_slaves*sizeof(DINT)*period_in_cycles);
  sinfill(_pos_arr, 0, 6400000.0, period_in_cycles);//6400000=100000counts/s
  sinfill(_pos_arr+period_in_cycles, 0, 6400000.0, period_in_cycles);// 800000=12500counts/s
  ycoe_csp_fill_posarray (num_slaves, period_in_cycles, _pos_arr);
  free(_pos_arr);
  // Other graphs can be used to fill these position arrays
  // The two arrays have to be equal in size
  // also keep in mind the max velocities and accelerations

  position_index[1] = 0;
  position_index[2] = 0;
  return 0;
}
int ycoe_csp_loop_posarray (int slavenum) {

    position_index[slavenum]++;
    if (position_index[runposarr][slavenum] >= period_in_cycles) position_index[runposarr][slavenum] = 0;

    DINT *target_position_pdo = (DINT *)(ec_slave[slavenum].outputs+2);
    *target_position_pdo = position_array[runposarr][slavenum][position_index[runposarr][slavenum]];

    return 0;
}



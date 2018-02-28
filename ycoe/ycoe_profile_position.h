#ifndef _YCOE_PROFILE_POSITION_H
#define _YCOE_PROFILE_POSITION_H

enum {
  CW_PPM_SNPATR1 = 0x00,
  CW_PPM_SNPATR2 = 0x10, /* Start the next Positioning after Target Reached */
  CW_PPM_SNPI1   = 0x20,
  CW_PPM_SNPI2   = 0x30 /* Start the next positioning immediately */
} cw_ppm_bits459; /* Control Word Profile Position mode bits 4,5 & 9 */

enum {
  CW_PPM_MASK_SNPATR = 0x70,
  CW_PPM_MASK_SNPI   = 0x30
} cw_ppm_mask_bits459; /*Control Word Profile Position mode Masks bits 4,5 & 9 */

enum {
  CW_PPM_RELPOS = 0x40, /* Target position is relative value */
  CW_PPM_HALT   = 0x100 /* Stop axis according to halt option code (605Dh) */
} cw_ppm_bits68; /* Control Word Profile position mode bits 6 & 8 */

enum {
  SW_PPM_TARGET_REACHED = 0x0400,
  SW_PPM_SETPOINT_ACK   = 0x1000,
  SW_PPM_FOLLOW_ERR  = 0x2000
} sw_ppm_bits1013;

int ycoe_ppm_setup(int slavenum);
int ycoe_ppm_checkcontrol (int slavenum, UINT targetcontrol);
int ycoe_ppm_checkstatus(int slavenum, UINT targetstatus);

int ycoe_ppm_get_parameters(int slavenum);

int ycoe_ppm_set_velocity (int slavenum,UDINT profile_velocity);
int ycoe_ppm_set_acceleration (int slavenum,UDINT profile_acceleration);
int ycoe_ppm_set_deceleration (int slavenum,UDINT profile_deceleration);
int ycoe_ppm_set_quick_stop_deceleration (int slavenum,UDINT quick_stop_deceleration);
int ycoe_ppm_set_parameters (int slavenum,UDINT profile_velocity, UDINT profile_acceleration, UDINT profile_deceleration, UDINT quick_stop_deceleration);

int ycoe_ppm_set_position (int slavenum, DINT position);
#endif

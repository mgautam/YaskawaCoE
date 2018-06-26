#ifndef _YCOE_HOMING_H
#define _YCOE_HOMING_H

enum {
  CW_HOME_START = 0x10;
  CW_HOME_HALT = 0x100;
} cw_home_bits48; /* Control Word Homing mode bits 4 & 8 */

enum {
  SW_HOME_PROGRESS = 0;
  SW_HOME_INTERRUPT = 0x400;
  SW_HOME_NOSUCCESS = 0x1000;
  SW_HOME_SUCCESS = 0x1400;
  SW_HOME_FAILURE = 0x2000;
  SW_HOME_FAILV0  = 0x2400;
} sw_home_bits101213; /* Statusword Homing mode bits 10,12 & 13 */

SW_HOME_MASK = 0x3400;

int ycoe_homing_setup(int slavenum, UDINT home_offset, UDINT acceleration, UDINT search_switch_speed, UDINT search_zero_speed, SINT homing_method);
int ycoe_home_at_curpos (int slavenum);
#endif

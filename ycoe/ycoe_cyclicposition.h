#ifndef _YCOE_CYCLICPOSITION_H
#define _YCOE_CYCLICPOSITION_H

enum {
  CW_CSP_HALT   = 0x100 /* Stop axis according to halt option code (605Dh) */
} cw_csp_bit8; /* Control Word cyclic synchronous positon mode bit 8 */

enum {
  SW_CSP_TARGET_REACHED = 0x0400,
  SW_CSP_TARGET_IGNORED = 0x1000,
  SW_CSP_FOLLOWING_EROR = 0x2000,
} sw_csp_bits1012;

int ycoe_csp_setup(int slavenum);
int ycoe_csp_checkstatus(int slavenum, UINT targetstatus);

int ycoe_csp_get_parameters(int slavenum);

int ycoe_csp_set_veloffset (int slavenum, UDINT velocity_offset);
int ycoe_csp_set_trqoffset (int slavenum, UDINT torque_offset);
int ycoe_csp_set_deceleration (int slavenum, UDINT profile_deceleration);
int ycoe_csp_set_quick_stop_deceleration (int slavenum, UDINT quick_stop_deceleration);
int ycoe_csp_set_parameters (int slavenum, UDINT velocity_offset, UDINT torque_offset, UDINT profile_deceleration, UDINT quick_stop_deceleration);

int ycoe_csp_add_position (int slavenum, DINT position);
int ycoe_csp_goto_position (int slavenum, DINT target_position);
int ycoe_csp_set_position (int slavenum, DINT final_position);
int ycoe_csp_goto_possync (int slavenum);

int ycoe_csp_setup_posarray(int num_slaves, unsigned int array_length);
int ycoe_csp_fill_posarray (int num_slaves, int posarr_len, DINT *pos_array);
int ycoe_csp_follow_posarray (int num_slaves);

int ycoe_csp_setup_sinarray(int num_slaves, unsigned int samples_per_second, unsigned int period_in_secs);
int ycoe_csp_loop_posarray (int slavenum);
#endif

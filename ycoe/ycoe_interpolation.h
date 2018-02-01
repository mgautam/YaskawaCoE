#ifndef _YCOE_INTERPOLATION_H
#define _YCOE_INTERPOLATION_H

enum {
  CW_IPM_DISABLE = 0x00, /* Disable interpolation */
  CW_IPM_ENABLE   = 0x10, /* Enable interpolation */
} cw_ipm_bit4; /* Control Word Interpolation mode bit 4 */

enum {
  CW_IPM_MASK_BIT4 = 0x10,
  CW_IPM_MASK_DUMB   = 0x30
} cw_ipm_mask_bit4; /*Control Word Profile Position mode Masks bits 4,5 & 9 */

enum {
  CW_IPM_DUMB = 0x40, /* Dummy var not required */
  CW_IPM_HALT   = 0x100 /* Stop axis according to halt option code (605Dh) */
} cw_ipm_bit8; /* Control Word Interpolation mode bit 8 */

enum {
  SW_IPM_TARGET_REACHED = 0x0400,
  SW_IPM_ACTIVE   = 0x1000,
} sw_ipm_bits1012;

int ycoe_ipm_setup(int slavenum);
int ycoe_ipm_checkcontrol (int slavenum, UINT targetcontrol);
int ycoe_ipm_checkstatus(int slavenum, UINT targetstatus);

int ycoe_ipm_get_parameters(void);

int ycoe_ipm_set_deceleration (UDINT profile_deceleration);
int ycoe_ipm_set_quick_stop_deceleration (UDINT quick_stop_deceleration);
int ycoe_ipm_set_parameters (UDINT profile_deceleration, UDINT quick_stop_deceleration);

int ycoe_ipm_set_position (int slavenum, DINT position);
#endif

#ifndef _YCOE_CONTROLSTATUS
#define _YCOE_CONTROLSTATUS

enum {
  CW_SHUTDOWN = 6,
  CW_SWITCHON = 7,
  CW_ENABLEOP = 0xF,
  CW_DISABLEOP= 7,
  CW_QUICKSTP = 2,
  CW_DISABLEV = 0
} cw_bit03; /* Controlword bit 0 to 3 */

enum {
  SW_NRTSO              = 0, /* Not Ready to Switch on */
  SW_SWITCHON_DISABLED  = 0x40,
  SW_RTSO               = 0x21, /* Ready to Switch on */
  SW_SWITCHED_ON        = 0x23,
  SW_OP_ENABLED         = 0x27, /* Operation Enabled */
  SW_QUICK_STOP         = 0x07, /* Quick Stop Active */
  SW_FAULT_ACTIVE       = 0x0F, /* Fault Reaction Active */
  SW_FAULT              = 0x08,
  SW_MAIN_POWERON       = 0x10,
  SW_WARNING            = 0x80
} sw_bit07; /* Statusword bit 0 to 7 */

enum {
  SW_MASK_NRTSO         = 0x4F,
  SW_MASK_SWON_DISABLED = 0x40,
  SW_MASK_RSOQ          = 0x2F, /* Masks for Ready to Switch on, Switched on, Operation Enabled, Quick Stop Active */
  SW_MASK_FAULT         = 0x0F,
  SW_MASK_MAIN_POWERON  = 0x10,
  SW_MASK_WARNING       = 0x80
} sw_mask_bit07; /* Statusword masks 0 to 7 */

#define SW_INTERNAL_LIMIT 0x800 /* Internal (Position) Limit Active */



int ycoe_setcontrolword(int slavenum, UINT controlvalue);
int ycoe_checkstatus (int slavenum, UINT targetstatus);
int ycoe_printstatus (int slavenum);
#endif

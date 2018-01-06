#ifndef _YCOETYPE_H
#define _YCOETYPE_H

/* Yaskawa CoE datatype and size definitions */
typedef int8 SINT;
typedef uint8 USINT;
#define SINT_SIZE sizeof(uint8)
#define USINT_SIZE SINT_SIZE

typedef int16 INT;
typedef uint16 UINT;
#define INT_SIZE sizeof(uint16)
#define UINT_SIZE INT_SIZE

typedef int32 DINT;
typedef uint32 UDINT;
#define DINT_SIZE sizeof(uint32)
#define UDINT_SIZE DINT_SIZE

enum {
  NO_MODE_ASSIGNED            = 0,
  PROFILE_POSITION_MODE       = 1,
  PROFILE_VELOCITY_MODE       = 3,
  TORQUE_PROFILE_MODE         = 4,
  HOMING_MODE                 = 6,
  INTERPOLATED_POSITION_MODE  = 7,
  CYCLIC_SYNC_POSITION_MODE   = 8,
  CYCLIC_SYNC_VELOCITY_MODE   = 9,
  CYCLIC_SYNC_TORQUE_MODE     = 10
} ycoe_operation_modes;

enum {
  CW_SHUTDOWN = 6,
  CW_SWITCHON = 7,
  CW_ENABLEOP = 0xF,
  CW_DISABLEOP = 7,
  CW_QUICKSTP = 2,
  CW_DISABLEV = 0
} cw_bit03; /* Controlword bit 0 to 3 */

int ycoe_set_mode_of_operation (SINT mode_of_operation);

#endif

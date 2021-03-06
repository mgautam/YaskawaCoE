#ifndef _YCOETYPE_H
#define _YCOETYPE_H

#include "ethercat.h" // Used only for EC_MAXSLAVE
#include "ethercattype.h"

/* Yaskawa CoE datatype and size definitions */
typedef int8 SINT;
typedef uint8 USINT;
#define SINT_SIZE sizeof(uint8)
#define USINT_SIZE SINT_SIZE

#ifdef linux
typedef int16 INT;
typedef uint16 UINT;
#endif
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

int ycoe_set_mode_of_operation (int slavenum, SINT mode_of_operation);

extern UDINT ycoe_vendor_ids[EC_MAXSLAVE];
extern UDINT ycoe_product_codes[EC_MAXSLAVE];

UDINT ycoe_get_vendor_id (int slavenum);
UDINT ycoe_get_product_code (int slavenum);
void ycoe_print_identity (int slavenum);
void discover_slave_identities (void);
#endif

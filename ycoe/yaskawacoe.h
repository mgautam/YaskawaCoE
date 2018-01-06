#ifndef _YASKAWACOE_H
#define _YASKAWACOE_H

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

int ycoe_get_profile_position_parameters(void);

#endif

#ifndef _YCOE_REGISTERS_H
#define _YCOE_REGISTERS_H

#include "ethercattype.h"

DINT ycoe_read_sysdeltatime(int slavenum);

SINT ycoe_readreg_sint(int slavenum, uint16 regaddr);
INT ycoe_readreg_int(int slavenum, uint16 regaddr);
DINT ycoe_readreg_dint(int slavenum, uint16 regaddr);
DINT ycoe_readCOparam(int slavenum, uint16 index, uint8 subindex);

int ycoe_writereg(int slavenum, uint16 regaddr, int size, UINT data);

#endif

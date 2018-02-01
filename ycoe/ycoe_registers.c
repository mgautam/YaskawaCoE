#include <stdio.h>
#include "ethercat.h"
#include "ycoetype.h"
#include "ycoe_registers.h"

SINT ycoex_readreg_sint(ecx_contextt *context, int slavenum, uint16 regaddr) {
  uint16 slaveh = context->slavelist[slavenum].configadr;
  SINT regcontent;
  (void)ecx_FPRD(context->port, slaveh, regaddr, SINT_SIZE, &regcontent, EC_TIMEOUTRET);
  return regcontent;
}
INT ycoex_readreg_int(ecx_contextt *context, int slavenum, uint16 regaddr) {
  uint16 slaveh = context->slavelist[slavenum].configadr;
  INT regcontent;
  (void)ecx_FPRD(context->port, slaveh, regaddr, INT_SIZE, &regcontent, EC_TIMEOUTRET);
  return regcontent;
}
DINT ycoex_readreg_dint(ecx_contextt *context, int slavenum, uint16 regaddr) {
  uint16 slaveh = context->slavelist[slavenum].configadr;
  DINT regcontent;
  (void)ecx_FPRD(context->port, slaveh, regaddr, DINT_SIZE, &regcontent, EC_TIMEOUTRET);
  return regcontent;
}

SINT ycoe_readreg_sint(int slavenum, uint16 regaddr) {
  return ycoex_readreg_sint(&ecx_context, slavenum, regaddr);
}
INT ycoe_readreg_int(int slavenum, uint16 regaddr) {
  return ycoex_readreg_int(&ecx_context, slavenum, regaddr);
}
DINT ycoe_readreg_dint(int slavenum, uint16 regaddr) {
  return ycoex_readreg_dint(&ecx_context, slavenum, regaddr);
}

DINT ycoe_readCOparam(int slavenum, uint16 index, uint16 subindex) {
  UDINT udintbuff;
  int udintsize = UDINT_SIZE;
  ec_SDOread(slavenum, index, subindex, 0, &udintsize, &udintbuff, EC_TIMEOUTRXM);
  return udintbuff;
}

int ycoex_writereg(ecx_contextt *context, int slavenum, uint16 regaddr, int size, UINT *data) {
  uint16 slaveh = context->slavelist[slavenum].configadr;
  (void)ecx_FPWR(context->port, slaveh, regaddr, size, data, EC_TIMEOUTRET);
  return 0;
}
int ycoe_writereg(int slavenum, uint16 regaddr, int size, UINT data) {
  return ycoex_writereg(&ecx_context, slavenum, regaddr, size, &data);
}

DINT ycoe_read_sysdeltatime(int slavenum) {
  return ycoex_readreg_dint(&ecx_context, slavenum, 0x92C);
}

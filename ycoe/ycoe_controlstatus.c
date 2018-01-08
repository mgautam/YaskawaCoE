#include <stdio.h>
#include "ycoetype.h"
#include "ycoe_controlstatus.h"
#include "ethercat.h"

int ycoe_setcontrolword(int slavenum, UINT controlvalue) {
    UINT *controlword = (UINT *) ec_slave[slavenum].outputs;
    *controlword = controlvalue;

    printf("Controlword: %x -> ",controlvalue);
    if ((controlvalue & CW_MASK_03) == CW_ENABLEOP)
        printf("EnOp");
    else if ((controlvalue & CW_MASK_03) == CW_SWITCHON)
        printf("SwOn");
    else if ((controlvalue & CW_MASK_02) == CW_SHUTDOWN)
        printf("ShDn");

    printf("\n");

    return 0;
}

int ycoe_checkstatus (int slavenum, UINT targetstatus) {
  UINT *statusword = (UINT *)ec_slave[slavenum].inputs;
  int retval = 0;

  if ((targetstatus & SW_MASK_NRTSO) == SW_NRTSO) {
    if ((*statusword & SW_MASK_NRTSO) == SW_NRTSO)
      retval++;
  }
  else if ((targetstatus & SW_MASK_SWON_DISABLED) == SW_SWITCHON_DISABLED) {
    if ((*statusword & SW_MASK_SWON_DISABLED) == SW_SWITCHON_DISABLED)
      retval++;
  }
  else if ((targetstatus & SW_MASK_RSOQ) == SW_RTSO) {
    if ((*statusword & SW_MASK_RSOQ) == SW_RTSO)
      retval++;
  }
  else if ((targetstatus & SW_MASK_RSOQ) == SW_SWITCHED_ON) {
    if ((*statusword & SW_MASK_RSOQ) == SW_SWITCHED_ON)
      retval++;
  }
  else if ((targetstatus & SW_MASK_RSOQ) == SW_OP_ENABLED) {
    if ((*statusword & SW_MASK_RSOQ) == SW_OP_ENABLED)
      retval++;
  }
  else if ((targetstatus & SW_MASK_RSOQ) == SW_QUICK_STOP) {
    if ((*statusword & SW_MASK_RSOQ) == SW_QUICK_STOP)
      retval++;
  }
  else if ((targetstatus & SW_MASK_FAULT) == SW_FAULT_ACTIVE) {
    if ((*statusword & SW_MASK_FAULT) == SW_FAULT_ACTIVE)
      retval++;
  }
  else if ((targetstatus & SW_MASK_FAULT) == SW_FAULT) {
    if ((*statusword & SW_MASK_FAULT) == SW_FAULT)
      retval++;
  }

  if ((targetstatus & SW_MASK_MAIN_POWERON) == SW_MAIN_POWERON) {
    if ((*statusword & SW_MASK_MAIN_POWERON) == SW_MAIN_POWERON)
      retval++;
  }

  if ((targetstatus & SW_MASK_WARNING) == SW_WARNING) {
    if ((*statusword & SW_MASK_WARNING) == SW_WARNING)
      retval++;
  }

  if ((targetstatus & SW_INTERNAL_LIMIT) == SW_INTERNAL_LIMIT) {
    if ((*statusword & SW_INTERNAL_LIMIT) == SW_INTERNAL_LIMIT)
      retval++;
  }

  return retval;
}

int ycoe_printstatus (int slavenum) { //(UDINT *statusword) {
  UINT *statusword = (UINT *)ec_slave[slavenum].inputs;

  printf("Statusword: ");
  if ((*statusword & SW_MASK_NRTSO) == SW_NRTSO)
    //printf("Not Ready to Switch On");
    printf("NRtSo");
  else if ((*statusword & SW_MASK_SWON_DISABLED) == SW_SWITCHON_DISABLED)
    //printf("Switch On Disabled");
    printf("SoD");
  else if ((*statusword & SW_MASK_RSOQ) == SW_RTSO)
    //printf("Ready to Switch On");
    printf("RtSo");
  else if ((*statusword & SW_MASK_RSOQ) == SW_SWITCHED_ON)
    //printf("Switched On");
    printf("SwOn");
  else if ((*statusword & SW_MASK_RSOQ) == SW_OP_ENABLED)
    //printf("Operation Enabled");
    printf("OpEn");
  else if ((*statusword & SW_MASK_RSOQ) == SW_QUICK_STOP)
    //printf("Quick Stop Active");
    printf("QSAct");
  else if ((*statusword & SW_MASK_FAULT) == SW_FAULT_ACTIVE)
    //printf("Fault Reaction Active");
    printf("FaAct");
  else if ((*statusword & SW_MASK_FAULT) == SW_FAULT)
    printf("Fault");

  if ((*statusword & SW_MASK_MAIN_POWERON) == SW_MAIN_POWERON)
    //printf(" | Main PowerOn");
    printf(" | PowerOn");

  if ((*statusword & SW_MASK_WARNING) == SW_WARNING)
    printf(" | Warn");

  if ((*statusword & SW_INTERNAL_LIMIT) == SW_INTERNAL_LIMIT)
    printf(" | ILAct");

  printf("\t");
  return 0;
}

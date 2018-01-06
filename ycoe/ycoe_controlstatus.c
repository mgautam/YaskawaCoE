#include <stdio.h>
#include "ycoetype.h"
#include "ycoe_controlstatus.h"
#include "ethercat.h"

int ycoe_setcontrolword(UINT controlvalue) {
    UINT *controlword = (UINT *) ec_slave[0].outputs;

    *controlword = controlvalue;

    return 0;
}

int ycoe_checkstatus (void) { //(UDINT *statusword) {
  UINT *statusword = (UINT *)ec_slave[0].inputs;

  printf("Statusword: ");
  if (*statusword & SW_MASK_NRTSO == SW_NRTSO)
    //printf("Not Ready to Switch On");
    printf("NRtSo");
  else if (*statusword & SW_MASK_SWON_DISABLED == SW_SWITCHON_DISABLED)
    //printf("Switch On Disabled");
    printf("SoD");
  else if (*statusword & SW_MASK_RSOQ == SW_RTSO)
    //printf("Ready to Switch On");
    printf("RtSo");
  else if (*statusword & SW_MASK_RSOQ == SW_SWITCHED_ON)
    //printf("Switched On");
    printf("SwOn");
  else if (*statusword & SW_MASK_RSOQ == SW_OP_ENABLED)
    //printf("Operation Enabled");
    printf("OpEn");
  else if (*statusword & SW_MASK_RSOQ == SW_QUICK_STOP)
    //printf("Quick Stop Active");
    printf("QSAct");
  else if (*statusword & SW_MASK_FAULT == SW_FAULT_ACTIVE)
    //printf("Fault Reaction Active");
    printf("FaAct");
  else if (*statusword & SW_MASK_FAULT == SW_FAULT)
    printf("Fault");

  if (*statusword & SW_MASK_MAIN_POWERON == SW_MAIN_POWERON)
    //printf(" | Main PowerOn");
    printf(" | PowerOn");

  if (*statusword & SW_MASK_WARNING == SW_WARNING)
    printf(" | Warn");

  if (*statusword & SW_INTERNAL_LIMIT == SW_INTERNAL_LIMIT)
    printf(" | ILAct");

  return 0;
}

#ifndef _YCOE_ENGINE_H_
#define _YCOE_ENGINE_H_

enum {
  YCOE_STATE_PREOP = 0x01,
  YCOE_STATE_SAFEOP = 0x02,
  YCOE_STATE_OPERATIONAL = 0x03
} YCOE_STATE;

void ycoe_engine(char *ifname);
void switch_to_next_ycoestate(void);
int ycoe_get_datamap(char **datamap_ptr);
OSAL_THREAD_FUNC ecatcheck( void *ptr );

#endif

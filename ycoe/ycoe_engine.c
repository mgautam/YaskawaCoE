#include <stdio.h>
#include <string.h>
#include "ethercat.h"
#include "ycoetype.h"
#include "ycoe_engine.h"

#ifndef _WIN32
#include <pthread.h>
#endif

#ifdef _WIN32
HANDLE ecat_mutex;
#else
pthread_mutex_t ecat_mutex;
#endif

#define EC_TIMEOUTMON 500

int ycoestate = 0;
static int switch_ycoestate_sem = 0;

char ycoe_network_pdomap[4096];
int ycoe_pdomap_size = 0;
char IOmap[4096];

int expectedWKC;
volatile int wkc;
uint8 currentgroup = 0;

void switch_to_next_ycoestate(void) {
    switch_ycoestate_sem++;
}

void update_network_pdomap(void) {
    int islaveindex, usedbytes = 0;
    memcpy(ycoe_network_pdomap, &ec_slavecount, 4);
    usedbytes = 4;
    for (islaveindex = 1; islaveindex <= ec_slavecount; islaveindex++) {
        memcpy(ycoe_network_pdomap+usedbytes,&(ec_slave[islaveindex].Ibytes), 4);
        usedbytes += 4;
        memcpy(ycoe_network_pdomap+usedbytes,&(ec_slave[islaveindex].Obytes), 4);
        usedbytes += 4;
        memcpy(ycoe_network_pdomap+usedbytes,ec_slave[islaveindex].inputs, ec_slave[islaveindex].Ibytes);
        usedbytes += ec_slave[islaveindex].Ibytes;
        memcpy(ycoe_network_pdomap+usedbytes,ec_slave[islaveindex].outputs, ec_slave[islaveindex].Obytes);
        usedbytes += ec_slave[islaveindex].Obytes;
    }
    ycoe_pdomap_size = usedbytes;
}

int ycoe_get_datamap(char **datamap_ptr)
{
  *datamap_ptr = ycoe_network_pdomap;
  return ycoe_pdomap_size;
}

OSAL_THREAD_FUNC ecatcheck( void *ptr )
{
  int slave;
  (void)ptr;                  /* Not used */

  while(1)
  {
    if((wkc < expectedWKC) || ec_group[currentgroup].docheckstate)
    {
      /* one ore more slaves are not responding */
      ec_group[currentgroup].docheckstate = FALSE;
      ec_readstate();
      for (slave = 1; slave <= ec_slavecount; slave++)
      {
        if ((ec_slave[slave].group == currentgroup) && (ec_slave[slave].state != EC_STATE_OPERATIONAL))
        {
          ec_group[currentgroup].docheckstate = TRUE;
          if (ec_slave[slave].state == (EC_STATE_SAFE_OP + EC_STATE_ERROR))
          {
            printf("ERROR : slave %d is in SAFE_OP + ERROR, attempting ack.\n", slave);
            ec_slave[slave].state = (EC_STATE_SAFE_OP + EC_STATE_ACK);
            ec_writestate(slave);
          }
          else if(ec_slave[slave].state == EC_STATE_SAFE_OP)
          {
            printf("WARNING : slave %d is in SAFE_OP, change to OPERATIONAL.\n", slave);
            ec_slave[slave].state = EC_STATE_OPERATIONAL;
            ec_writestate(slave);
          }
          else if(ec_slave[slave].state > EC_STATE_NONE)
          {
            if (ec_reconfig_slave(slave, EC_TIMEOUTMON))
            {
              ec_slave[slave].islost = FALSE;
              printf("MESSAGE : slave %d reconfigured\n",slave);
            }
          }
          else if(!ec_slave[slave].islost)
          {
            /* re-check state */
            ec_statecheck(slave, EC_STATE_OPERATIONAL, EC_TIMEOUTRET);
            if (ec_slave[slave].state == EC_STATE_NONE)
            {
              ec_slave[slave].islost = TRUE;
              printf("ERROR : slave %d lost\n",slave);
            }
          }
        }
        if (ec_slave[slave].islost)
        {
          if(ec_slave[slave].state == EC_STATE_NONE)
          {
            if (ec_recover_slave(slave, EC_TIMEOUTMON))
            {
              ec_slave[slave].islost = FALSE;
              printf("MESSAGE : slave %d recovered\n",slave);
            }
          }
          else
          {
            ec_slave[slave].islost = FALSE;
            printf("MESSAGE : slave %d found\n",slave);
          }
        }
      }
      if(!ec_group[currentgroup].docheckstate)
        printf("OK : all slaves resumed OPERATIONAL.\n");
    }
    osal_usleep(10000);
  }
}

void ycoe_engine(char *ifname)
{
    OSAL_THREAD_HANDLE ecatcheck_thread;
    int i, chk;
    int islaveindex;

    printf("Starting YaskawaCoE master\n");

    /* create thread to handle slave error handling in OP */
    osal_thread_create(&ecatcheck_thread, 128000, &ecatcheck, (void*) &ctime);

    /* initialise SOEM, bind socket to ifname */
    if (ec_init(ifname))
    {
        printf("ec_init on %s succeeded.\n",ifname);

        /*
           ec_config_init find and auto-config slaves.
           It requests all slaves to state PRE-OP.
           All data read and configured are stored in a global array ec_slave
        */
        if ( ec_config_init(FALSE) > 0 )
        {
            printf("%d slaves found and configured. PRE_OP requested on all slaves.\n",ec_slavecount);
            discover_slave_identities();

            /* wait for all slaves to reach PREOP state */
            chk = 40;
            do
            {
                ec_statecheck(0, EC_STATE_PRE_OP, 50000);
            } while (chk-- && (ec_slave[0].state != EC_STATE_PRE_OP));
            ycoestate = YCOE_STATE_PREOP;
            // Wait for signal from semaphore to request SAFE_OP state
            while (switch_ycoestate_sem <= 0) osal_usleep(1000);
            --switch_ycoestate_sem;

            /* Configure Distributed Clock mechanism */
            ec_configdc();

            /*
               ec_config_map reads PDO mapping and set local buffer for PDO exchange.
               It requests all slaves to state SAFE-OP.
               Outputs are placed together in the beginning of IOmap, inputs follow
            */
            ec_config_map(&IOmap);
            printf("Slaves mapped, state to SAFE_OP requested.\n");
            /* wait for all slaves to reach SAFE_OP state */
            ec_statecheck(0, EC_STATE_SAFE_OP,  EC_TIMEOUTSTATE * 4);
            /* slave0 is the master. All slaves pdos are mapped to slave0 */
            ycoestate = YCOE_STATE_SAFEOP;
            // Wait for signal from semaphore to request OPERATIONAL state
            while (switch_ycoestate_sem <= 0) osal_usleep(1000);
            --switch_ycoestate_sem;

            printf("segments : %d : %d %d %d %d\n",ec_group[0].nsegments ,ec_group[0].IOsegment[0],ec_group[0].IOsegment[1],ec_group[0].IOsegment[2],ec_group[0].IOsegment[3]);



            printf("Request operational state for all slaves\n");
            expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
            printf("Calculated workcounter %d\n", expectedWKC);
            ec_slave[0].state = EC_STATE_OPERATIONAL;
            /* send one valid process data to make outputs in slaves happy*/
            ec_send_processdata();
            ec_receive_processdata(EC_TIMEOUTRET);
            /* request OP state for all slaves */
            ec_writestate(0);
            chk = 40;
            /* wait for all slaves to reach OP state */
            do
            {
                ec_send_processdata();
                ec_receive_processdata(EC_TIMEOUTRET);
                ec_statecheck(0, EC_STATE_OPERATIONAL, 50000);
            } while (chk-- && (ec_slave[0].state != EC_STATE_OPERATIONAL));
            ycoestate = YCOE_STATE_OPERATIONAL;


            if (ec_slave[0].state == EC_STATE_OPERATIONAL )
            {
                printf("Operational state reached for all slaves.\n");

                /* cyclic loop */
				        i = 0;
        				while(1)
                {
#ifdef _WIN32
           						WaitForSingleObject(ecat_mutex, INFINITE);
#else
                      pthread_mutex_lock(&ecat_mutex);
#endif
 				          	i++;
                    ec_send_processdata();
                    wkc = ec_receive_processdata(EC_TIMEOUTRET);

                    if(wkc >= expectedWKC)
                    {
                      update_network_pdomap();
                    }
#ifdef _WIN32
					          	ReleaseMutex(ecat_mutex);
#else
                      pthread_mutex_unlock(&ecat_mutex);
#endif
                   osal_usleep(1000);

                }
            }
            else
            {
              printf("Not all slaves reached operational state.\n");
              ec_readstate();
              for(i = 1; i<=ec_slavecount ; i++)
              {
                if(ec_slave[i].state != EC_STATE_OPERATIONAL)
                {
                  printf("Slave %d State=0x%2.2x StatusCode=0x%4.4x : %s\n",
                      i, ec_slave[i].state, ec_slave[i].ALstatuscode, ec_ALstatuscode2string(ec_slave[i].ALstatuscode));
                }
              }
            }
            printf("\nRequest init state for all slaves\n");
            ec_slave[0].state = EC_STATE_INIT;
            /* request INIT state for all slaves */
            ec_writestate(0);
        }
        else
        {
          printf("No slaves found!\n");
        }
        printf("End simple test, close socket\n");
        /* stop SOEM, close socket */
        ec_close();
    }
    else
    {
      printf("No socket connection on %s\nExcecute as root\n",ifname);
    }
}



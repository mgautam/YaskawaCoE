/** \file
 * \brief Yaskawa CoE master application
 *
 * Usage : yaskawaCoE [ifname1]
 * ifname is NIC interface, f.e. eth0
 *
 * (c)Gautam Manoharan 2017 - 2018
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "ethercat.h"
#include "yaskawacoe.h"

#ifndef _WIN32
#include <pthread.h>
#endif

#define EC_TIMEOUTMON 500

#ifdef _WIN32
HANDLE IOmutex;
#else
pthread_mutex_t IOmutex;
#endif

char guiIOmap[4096];
int guiIObytes = 0;
int graphIndex = 44; //4+(4+4+6+6)*2
int pos_cmd_sem[3] = {0,0,0}; // Position Command Semaphore
DINT final_position = 0;
char IOmap[4096];
int expectedWKC;
volatile int wkc;
boolean inOP;
uint8 currentgroup = 0;

void coeController(char *ifname)
{
    int i, chk;
    inOP = FALSE;
    int islaveindex;
    DINT curr_position, prev_position = 0;
    DINT slave_velocity = 0;

    ec_timet current_time, previous_time, diff_time;
    unsigned int act_cycle_time;
    //unsigned int min_cycle_time = 999999, max_cycle_time = 0, avg_cycle_time = 0, sum_cycle_time = 0;

    FILE *posfile = fopen("slave_positions.csv","w");
    fprintf(posfile, "Cycle, CycleTime, Slave1 Target Position, Actual Position, Slave2 Target Position,Actual Position,\n");

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

            /* Configure Distributed Clock mechanism */
            ec_configdc();
            for (islaveindex = 1; islaveindex <= ec_slavecount; islaveindex++) {
                /* Check & Set Interpolation Mode Parameters */
                //ycoe_csp_get_parameters(islaveindex);
                ycoe_csp_setup(islaveindex);
                ycoe_set_mode_of_operation(islaveindex,CYCLIC_SYNC_POSITION_MODE);
                ycoe_csp_set_parameters(islaveindex,0,0,1048576,1048576);
                //ycoe_csp_get_parameters(islaveindex);
            }
ycoe_csp_setup_posarray(2,500,5);

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

            if (ec_slave[0].state == EC_STATE_OPERATIONAL )
            {
                printf("Operational state reached for all slaves.\n");
                inOP = TRUE;

                previous_time = osal_current_time();

                /* cyclic loop */
				        i = 0;
        				while(1)
                {
#ifdef _WIN32
           						WaitForSingleObject(IOmutex, INFINITE);
#else
                      pthread_mutex_lock(&IOmutex);
#endif
 				          	i++;

                    for (islaveindex = 1; islaveindex <= ec_slavecount; islaveindex++) {
                        //ycoe_printstatus(1);

                        if(ycoe_checkstatus(islaveindex,SW_SWITCHON_DISABLED))
                          ycoe_setcontrolword(islaveindex,CW_SHUTDOWN);
                        else if(ycoe_checkstatus(islaveindex,SW_RTSO))
                          ycoe_setcontrolword(islaveindex,CW_SWITCHON);
                        else if(ycoe_checkstatus(islaveindex,SW_SWITCHED_ON))
                        {
                            ycoe_setcontrolword(islaveindex,CW_ENABLEOP);
                            final_position = 1500000000;//81920;
                            //ycoe_csp_set_position(islaveindex, 1500000000);
                            pos_cmd_sem[islaveindex] = 1;
                        }
                        /*else {
//                          if (ycoe_checkstatus(islaveindex,SW_OP_ENABLED))
                          if (ycoe_checkstatus(1,SW_OP_ENABLED) \
                              && ycoe_checkstatus(2,SW_OP_ENABLED))
                          {
                             if (pos_cmd_sem[islaveindex] > 0) {
                              // Add interpolation calculations
                              //if (ycoe_csp_goto_position(islaveindex,final_position)) {
                              if (ycoe_csp_goto_possync(islaveindex)) {
                                pos_cmd_sem[islaveindex]=0;
                              }
                              //ycoe_csp_follow_posarray(islaveindex);
                            }

                          }
                        }*/
                    }

                        if (ycoe_checkstatus(1,SW_OP_ENABLED) \
                              && ycoe_checkstatus(2,SW_OP_ENABLED))
                          {
                              // Add interpolation calculations
                              ycoe_csp_follow_posarray(islaveindex);
                          }


                    ec_send_processdata();
                    wkc = ec_receive_processdata(EC_TIMEOUTRET);

                    if(wkc >= expectedWKC)
                    {
                      int usedbytes = 0;
                      memcpy(guiIOmap, &ec_slavecount, 4);
                      usedbytes = 4;
                      for (islaveindex = 1; islaveindex <= ec_slavecount; islaveindex++) {
                          memcpy(guiIOmap+usedbytes,&(ec_slave[islaveindex].Ibytes), 4);
                          usedbytes += 4;
                          memcpy(guiIOmap+usedbytes,&(ec_slave[islaveindex].Obytes), 4);
                          usedbytes += 4;
                          memcpy(guiIOmap+usedbytes,ec_slave[islaveindex].inputs, ec_slave[islaveindex].Ibytes);
                          usedbytes += ec_slave[islaveindex].Ibytes;
                          memcpy(guiIOmap+usedbytes,ec_slave[islaveindex].outputs, ec_slave[islaveindex].Obytes);
                          usedbytes += ec_slave[islaveindex].Obytes;
                      }
                      guiIObytes = usedbytes;
                   }
#ifdef _WIN32
					          	ReleaseMutex(IOmutex);
#else
                      pthread_mutex_unlock(&IOmutex);
#endif
                   osal_usleep(500);


                   current_time = osal_current_time();
                   osal_time_diff(&previous_time,&current_time,&diff_time);
                   act_cycle_time = diff_time.usec;
                   /*if (act_cycle_time < min_cycle_time) min_cycle_time = act_cycle_time;
                   if (act_cycle_time > max_cycle_time) max_cycle_time = act_cycle_time;
                   sum_cycle_time += act_cycle_time;
                   avg_cycle_time = (int) (((float)sum_cycle_time)/((float)i));
                   printf("cycle:%d act:%6d min:%6d avg:%6d max:%6d\r",i,act_cycle_time,min_cycle_time,avg_cycle_time,max_cycle_time);*/
                   previous_time = current_time;

                   //fprintf(posfile,"%d,%d,%d,%d,%d,%d,\n",i,act_cycle_time,*(UDINT *)(ec_slave[1].outputs+2),*(UDINT *)(ec_slave[1].inputs+2),*(UDINT *)(ec_slave[2].outputs+2),*(UDINT *)(ec_slave[2].inputs+2));
                }
                inOP = FALSE;
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

OSAL_THREAD_FUNC ecatcheck( void *ptr )
{
  int slave;
  (void)ptr;                  /* Not used */

  while(1)
  {
    if( inOP && ((wkc < expectedWKC) || ec_group[currentgroup].docheckstate))
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

/* Server for talking to GUI Application */
OSAL_THREAD_FUNC controlserver(void *ptr) {

  /* Automatic Motion Sequence */
  int position_request = 300000000;
  while (1) {
     osal_usleep(900000);
#ifdef _WIN32
    WaitForSingleObject(IOmutex, INFINITE);
#else
    pthread_mutex_lock(&IOmutex);
#endif
     if (ycoe_csp_checkstatus(1, SW_CSP_TARGET_REACHED) &&
          ycoe_csp_checkstatus(2, SW_CSP_TARGET_REACHED)) {
        if (position_request > 0) position_request =0;
        else position_request =300000000;
        ycoe_csp_set_position(1, position_request);
        ycoe_csp_set_position(2, position_request);
        pos_cmd_sem[1]++;
        pos_cmd_sem[2]++;
      }
#ifdef _WIN32
    ReleaseMutex(IOmutex);
#else
    pthread_mutex_unlock(&IOmutex);
#endif
  }

/* user buttons - semi-automatic motion sequence */
  char buffer[15];
	FILE * filepointer = fopen("/dev/ycoe", "r");
	while (1) {
    fread(buffer, sizeof(char) , 4, filepointer);

#ifdef _WIN32
    WaitForSingleObject(IOmutex, INFINITE);
#else
    pthread_mutex_lock(&IOmutex);
#endif
      if (buffer[2] == 4) {
        //printf("Read data=%x\n\r",buffer[0]);
          // Multi axis Command
        USINT slaveaddr;
        final_position = 1500000000;
        for (slaveaddr = 1; slaveaddr <= ec_slavecount; slaveaddr++) {
          pos_cmd_sem[slaveaddr]++;
          //printf("Slave %x Requested position:%d and pos_cmd_sem=%d\n\r",slaveaddr,*targetposition,pos_cmd_sem[slaveaddr]);
        }
      }
      else if (buffer[2] == 8) {
        //printf("Read data=%x\n\r",buffer[0]);
        // Multi axis Command
        USINT slaveaddr;
        final_position = 0;
        for (slaveaddr = 1; slaveaddr <= ec_slavecount; slaveaddr++) {
          pos_cmd_sem[slaveaddr]++;
          //printf("Slave %x Requested position:%d and pos_cmd_sem=%d\n\r",slaveaddr,*targetposition,pos_cmd_sem[slaveaddr]);
        }
      }

#ifdef _WIN32
    ReleaseMutex(IOmutex);
#else
    pthread_mutex_unlock(&IOmutex);
#endif

    osal_usleep(500000);
  }
  fclose(filepointer);
}

int main(int argc, char *argv[])
{
  OSAL_THREAD_HANDLE thread1, thread2;
#ifdef _WIN32
  IOmutex = CreateMutex(
      NULL,              // default security attributes
      FALSE,             // initially not owned
      NULL);             // unnamed mutex
  if (IOmutex == NULL)
  {
    printf("CreateMutex error: %d\n", GetLastError());
    return 1;
  }
#else
  //IOmutex = PTHREAD_MUTEX_INITALIZER;
  pthread_mutex_init(&IOmutex, NULL);
#endif
  printf("YaskawaCoE (Yaskawa Canopen over Ethercat Master)\nControl Application\n");

  if (argc > 1)
  {
    /* create thread to handle slave error handling in OP */
    //      pthread_create( &thread1, NULL, (void *) &ecatcheck, (void*) &ctime);
    //osal_thread_create(&thread1, 128000, &ecatcheck, (void*) &ctime);
    // thread to handle gui application requests
    //osal_thread_create(&thread2, 128000, &controlserver, (void*)&ctime);
    /* start cyclic part */
    osal_thread_create_rt(&thread2, 128000, &coeController, argv[1]);
    //coeController(argv[1]);
    //controlserver(argv[1]);
    ecatcheck((void *) &ctime);
  }
  else
  {
    printf("Usage: yaskawaCoE ifname1\nifname = eth0 for example\n");
  }

#ifdef _WIN32
  CloseHandle(IOmutex);
#else
  pthread_mutex_destroy(&IOmutex);
#endif
  printf("End program\n");
  return (0);
}

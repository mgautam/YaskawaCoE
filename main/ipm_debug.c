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

#include "zmq.h"

#include "ethercat.h"
#include "ecatcheck.h"
#include "yaskawacoe.h"

#ifndef _WIN32
#include <pthread.h>
#endif

#ifdef _WIN32
HANDLE IOmutex;
#else
pthread_mutex_t IOmutex;
#endif

char guiIOmap[4096];
int guiIObytes = 0;
int pos_cmd_sem[3] = {0,0,0}; // Position Command Semaphore
DINT final_position = 0;
//char  oloop, iloop;
char IOmap[4096];

extern int expectedWKC;
extern volatile int wkc;
extern uint8 currentgroup;

void coeController(char *ifname)
{
    int i,/* j,*/ chk;
    int islaveindex;

    printf("Starting YaskawaCoE master\n");

    /* initialise SOEM, bind socket to ifname */
    if (ec_init(ifname))
    {
        printf("ec_init on %s succeeded.\n",ifname);
printf("a(ec_init) Slave:%d CoE State: %x\n\r",1,ycoe_readreg_int(1, 0x130));

        /*
           ec_config_init find and auto-config slaves.
           It requests all slaves to state PRE-OP.
           All data read and configured are stored in a global array ec_slave
        */
        if ( ec_config_init(FALSE) > 0 )
        {
printf("a(ec_config_init) Slave:%d CoE State: %x\n\r",1,ycoe_readreg_int(1, 0x130));
            printf("%d slaves found and configured. PRE_OP requested on all slaves.\n",ec_slavecount);

            /* Configure Distributed Clock mechanism */
            ec_configdc();
            for (islaveindex = 1; islaveindex <= ec_slavecount; islaveindex++) {
                /* Check & Set Interpolation Mode Parameters */
                //ycoe_ipm_get_parameters(islaveindex);
                printf("Slave %x Index:Subindex %x:%x Content = %x\n\r",islaveindex,0x1602,2,ycoe_readCOparam(islaveindex, 0x1602, 2));
                ycoe_ipm_setup(islaveindex);
                printf("Slave %x Index:Subindex %x:%x Content = %x\n\r",islaveindex,0x1602,2,ycoe_readCOparam(islaveindex, 0x1602, 2));
                ycoe_set_mode_of_operation(islaveindex,INTERPOLATED_POSITION_MODE);
                printf("Slave %x Index:Subindex %x:%x Content = %x\n\r",islaveindex,0x1602,2,ycoe_readCOparam(islaveindex, 0x1602, 2));
                ycoe_ipm_set_parameters(islaveindex,1048576,1048576);
                //ycoe_ipm_get_parameters(islaveindex);
            }

printf("a(ycoe_setup) Slave:%d CoE State: %x\n\r",1,ycoe_readreg_int(1, 0x130));


            /*
               ec_config_map reads PDO mapping and set local buffer for PDO exchange.
               It requests all slaves to state SAFE-OP.
               Outputs are placed together in the beginning of IOmap, inputs follow
            */
            ec_config_map(&IOmap);
printf("a(ec_config_map) Slave:%d CoE State: %x\n\r",1,ycoe_readreg_int(1, 0x130));
            printf("Slaves mapped, state to SAFE_OP requested.\n");
            /* wait for all slaves to reach SAFE_OP state */
            ec_statecheck(0, EC_STATE_SAFE_OP,  EC_TIMEOUTSTATE * 4);
            /* slave0 is the master. All slaves pdos are mapped to slave0 */
            //oloop = ec_slave[0].Obytes; /* Obytes are the total number of output bytes consolidated from all slaves */
            //iloop = ec_slave[0].Ibytes; /* Ibytes are the total number of input bytes consolidated from all slaves */

            printf("segments : %d : %d %d %d %d\n",ec_group[0].nsegments ,ec_group[0].IOsegment[0],ec_group[0].IOsegment[1],ec_group[0].IOsegment[2],ec_group[0].IOsegment[3]);



            printf("Request operational state for all slaves\n");
            expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
            printf("Calculated workcounter %d\n", expectedWKC);
            ec_slave[0].state = EC_STATE_OPERATIONAL;
            /* send one valid process data to make outputs in slaves happy*/
            ec_send_processdata();
            ec_receive_processdata(EC_TIMEOUTRET);
printf("a(1ec_send_pdo) Slave:%d CoE State: %x\n\r",1,ycoe_readreg_int(1, 0x130));
            /* request OP state for all slaves */
            ec_writestate(0);
printf("a(ec_writestate) Slave:%d CoE State: %x\n\r",1,ycoe_readreg_int(1, 0x130));
            chk = 40;
            /* wait for all slaves to reach OP state */
            do
            {
                ec_send_processdata();
                ec_receive_processdata(EC_TIMEOUTRET);
                ec_statecheck(0, EC_STATE_OPERATIONAL, 50000);
            } while (chk-- && (ec_slave[0].state != EC_STATE_OPERATIONAL));
printf("a(wait_op) Slave:%d CoE State: %x\n\r",1,ycoe_readreg_int(1, 0x130));


            if (ec_slave[0].state == EC_STATE_OPERATIONAL )
            {
                printf("Operational state reached for all slaves.\n");

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
                            final_position = 181920;
                            pos_cmd_sem[islaveindex] = 1;
                            //ycoe_ipm_set_position (1,8192);
                        }
                        else {
                          if (ycoe_checkstatus(islaveindex,SW_OP_ENABLED))
                          {
/*                            if (pos_cmd_sem[islaveindex] > 0) {
                              ycoe_ipm_set_position(islaveindex, final_position);
                              pos_cmd_sem[islaveindex]--;
                              ycoe_setcontrolword(islaveindex,CW_ENABLEOP | CW_IPM_ENABLE);
                            }
*/                          if (ycoe_ipm_checkcontrol(islaveindex, CW_IPM_DISABLE) || \
                              (ycoe_ipm_checkstatus(islaveindex,SW_IPM_ACTIVE)==0)) {
                              ycoe_setcontrolword(islaveindex,CW_ENABLEOP | CW_IPM_ENABLE);
                            }
                            if (pos_cmd_sem[islaveindex] > 0) {
                              /* Add interpolation calculations */
                              //printf("cycle %d: pos_cmd_sem[islaveindex]>0\n\r",i);
//          					          printf("PDO cycle %4d, T:%"PRId64"\n\r", i, ec_DCtime);
                              if (ycoe_ipm_goto_position(islaveindex,final_position)) {
                                pos_cmd_sem[islaveindex]--;
                              }
          					         // printf("PDO cycle %4d, T:%"PRId64"\n\r", i, ec_DCtime);
                            }
                          }
                        }
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
/*
          					  printf("PDO cycle %4d, WKC %d , T:%"PRId64"\n", i, wkc, ec_DCtime);

                      printf(" O:");
                      for(j = 0 ; j < oloop; j++)
                        printf(" %2.2x", *(ec_slave[0].outputs + j));

                      printf("\tI:");
                      for(j = 0 ; j < iloop; j++)
                        printf(" %2.2x", *(ec_slave[0].inputs + j));
                      printf("\n");
*/                   }
#ifdef _WIN32
					          	ReleaseMutex(IOmutex);
#else
                      pthread_mutex_unlock(&IOmutex);
#endif
                   osal_usleep(500);

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

/* Server for talking to GUI Application */
OSAL_THREAD_FUNC controlserver(void *ptr) {
  //  Socket to talk to clients
	void *context = zmq_ctx_new();
	void *responder = zmq_socket(context, ZMQ_REP);
	int rc = zmq_bind(responder, "tcp://*:5555");
	char buffer[15];

	while (1) {
    zmq_recv(responder, buffer, 15, 0);

#ifdef _WIN32
    WaitForSingleObject(IOmutex, INFINITE);
#else
    pthread_mutex_lock(&IOmutex);
#endif
    if (buffer[0] == 3) {
      USINT *slaveaddr = (USINT *)(buffer + 1);
      DINT *targetposition = (DINT *)(buffer + 1+1);
      if (*slaveaddr <= ec_slavecount) {
        //ycoe_ipm_set_position(*slaveaddr, *targetposition);//Vulnerable to racing conditions
        final_position = *targetposition;
        pos_cmd_sem[*slaveaddr]++;
        printf("Slave %x Requested position:%d and pos_cmd_sem=%d\n\r",*slaveaddr,*targetposition,pos_cmd_sem[*slaveaddr]);
      } else {
        printf("Invalid slave address:%x Requested position:%d and pos_cmd_sem=%d\n\r",*slaveaddr,*targetposition,pos_cmd_sem[*slaveaddr]);
      }
    }
    else if (buffer[0] == 6) {
      USINT *slaveaddr = (USINT *)(buffer + 1);
      INT *regaddr = (INT *)(buffer + 1+1);
      printf("Slave %x Register %x Content = %x\n\r",*slaveaddr,*regaddr,ycoe_readreg_dint(*slaveaddr, *regaddr));
    }
    else if (buffer[0] == 9) {
      USINT *slaveaddr = (USINT *)(buffer + 1);
      INT *index = (INT *)(buffer + 1+1);
      INT *subindex = (INT *)(buffer + 1+1+4);
      printf("Slave %x Index:Subindex %x:%x Content = %x\n\r",*slaveaddr,*index,*subindex,ycoe_readCOparam(*slaveaddr, *index, *subindex));
    }
    else if (buffer[0] == 33) {
      USINT slaveaddr;
      DINT *targetposition = (DINT *)(buffer + 1);
      final_position = *targetposition;
      for (slaveaddr = 1; slaveaddr <= ec_slavecount; slaveaddr++) {
        //ycoe_ipm_set_position(*slaveaddr, *targetposition);//Vulnerable to racing conditions
        pos_cmd_sem[slaveaddr]++;
        printf("Slave %x Requested position:%d and pos_cmd_sem=%d\n\r",slaveaddr,*targetposition,pos_cmd_sem[slaveaddr]);
      }
    }

    zmq_send(responder, guiIOmap, guiIObytes, 0);
#ifdef _WIN32
    ReleaseMutex(IOmutex);
#else
    pthread_mutex_unlock(&IOmutex);
#endif
  }
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
    osal_thread_create(&thread1, 128000, &ecatcheck, (void*) &ctime);
    // thread to handle gui application requests
    //osal_thread_create(&thread2, 128000, &controlserver, (void*)&ctime);
    /* start cyclic part */
    osal_thread_create(&thread2, 128000, &coeController, argv[1]);
    //coeController(argv[1]);
    controlserver(argv[1]);
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

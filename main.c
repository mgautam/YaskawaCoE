/** \file
 * \brief Yaskawa CoE master application
 *
 * Usage : yaskawaCoE [ifname1]
 * ifname is NIC interface, f.e. eth0
 *
 * (c)Gautam Manoharan 2017 - 2018
 */

#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>
#include <alchemy/task.h>
#include <alchemy/timer.h>
#include <alchemy/sem.h>
//#include <rtdm/testing.h>
#include <boilerplate/trace.h>
#include <xenomai/init.h>


#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "ethercat.h"
#include "yaskawacoe.h"

#ifndef _WIN32
#include <pthread.h>
#endif

#define EC_TIMEOUTMON 500

RT_TASK engine_task;
int run=1;

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

void coeController(void *arg)
{
    char ifname[32]="wiznet";

    int i, chk;
    inOP = FALSE;
    int islaveindex;
    //DINT curr_position, prev_position = 0;
    //DINT slave_velocity = 0;

    //ec_timet current_time, previous_time, diff_time;
    //unsigned int act_cycle_time;
    //unsigned int min_cycle_time = 999999, max_cycle_time = 0, avg_cycle_time = 0, sum_cycle_time = 0;


    /* initialise SOEM, bind socket to ifname */
    if (ec_init(ifname))
    {
        rt_printf("ec_init on %s succeeded.\n",ifname);

        /*
           ec_config_init find and auto-config slaves.
           It requests all slaves to state PRE-OP.
           All data read and configured are stored in a global array ec_slave
        */
        if ( ec_config_init(FALSE) > 0 )
        {
            rt_printf("%d slaves found and configured. PRE_OP requested on all slaves.\n",ec_slavecount);

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
            rt_printf("Slaves mapped, state to SAFE_OP requested.\n");
            /* wait for all slaves to reach SAFE_OP state */
            ec_statecheck(0, EC_STATE_SAFE_OP,  EC_TIMEOUTSTATE * 4);
            /* slave0 is the master. All slaves pdos are mapped to slave0 */

            rt_printf("Request operational state for all slaves\n");
            expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
            rt_printf("Calculated workcounter %d\n", expectedWKC);
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
                rt_printf("Operational state reached for all slaves.\n");
                inOP = TRUE;

                //previous_time = osal_current_time();

                /* cyclic loop */
				        i = 0;
                rt_task_sleep(1e6);
                rt_task_set_periodic(NULL, TM_NOW, 1000000);//1ms

        				while(run)
                {
                     rt_task_wait_period(NULL);   //wait for next cycle

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
                            //final_position = 1500000000;//81920;
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

                   //current_time = osal_current_time();
                   //osal_time_diff(&previous_time,&current_time,&diff_time);
                   //act_cycle_time = diff_time.usec;
                   /*if (act_cycle_time < min_cycle_time) min_cycle_time = act_cycle_time;
                   if (act_cycle_time > max_cycle_time) max_cycle_time = act_cycle_time;
                   sum_cycle_time += act_cycle_time;
                   avg_cycle_time = (int) (((float)sum_cycle_time)/((float)i));
                   rt_printf("cycle:%d act:%6d min:%6d avg:%6d max:%6d\r",i,act_cycle_time,min_cycle_time,avg_cycle_time,max_cycle_time);*/
                   //previous_time = current_time;

                   //frt_printf(posfile,"%d,%d,%d,%d,%d,%d,\n",i,act_cycle_time,*(UDINT *)(ec_slave[1].outputs+2),*(UDINT *)(ec_slave[1].inputs+2),*(UDINT *)(ec_slave[2].outputs+2),*(UDINT *)(ec_slave[2].inputs+2));
                }
                inOP = FALSE;
            }
            else
            {
              rt_printf("Not all slaves reached operational state.\n");
              ec_readstate();
              for(i = 1; i<=ec_slavecount ; i++)
              {
                if(ec_slave[i].state != EC_STATE_OPERATIONAL)
                {
                  rt_printf("Slave %d State=0x%2.2x StatusCode=0x%4.4x : %s\n",
                      i, ec_slave[i].state, ec_slave[i].ALstatuscode, ec_ALstatuscode2string(ec_slave[i].ALstatuscode));
                }
              }
            }
            rt_printf("\nRequest init state for all slaves\n");
            ec_slave[0].state = EC_STATE_INIT;
            /* request INIT state for all slaves */
            ec_writestate(0);
        }
        else
        {
          rt_printf("No slaves found!\n");
        }
        rt_printf("End simple test, close socket\n");
        /* stop SOEM, close socket */
        ec_close();
    }
    else
    {
      rt_printf("No socket connection on %s\nExcecute as root\n",ifname);
    }
}

void catch_signal(int sig)
{
    run=0;
    osal_usleep(1e5);
    rt_task_delete(&engine_task);
    exit(1);
}

int main(int argc, char *argv[])
{
  signal(SIGTERM, catch_signal);
  signal(SIGINT, catch_signal);

  mlockall(MCL_CURRENT | MCL_FUTURE);

  printf("YaskawaCoE (Yaskawa Canopen over Ethercat Master)\nControl Application\n");

  if (argc > 1)
  {
    /* create thread to handle slave error handling in OP */
    //      pthread_create( &thread1, NULL, (void *) &ecatcheck, (void*) &ctime);
    //osal_thread_create(&thread1, 128000, &ecatcheck, (void*) &ctime);
    // thread to handle gui application requests
    //osal_thread_create(&thread2, 128000, &controlserver, (void*)&ctime);
    /* start cyclic part */
    rt_task_create(&engine_task, "ycoe_engine", 0, 90, 0 );
    rt_task_start(&engine_task, &coeController, NULL);
    //osal_thread_create_rt(&thread2, 128000, &coeController, argv[1]);
    //coeController(argv[1]);
    while (run)
      osal_usleep(300000);
  }
  else
  {
    printf("Usage: yaskawaCoE ifname1\nifname = eth0 for example\n");
  }

  printf("End program\n");
  return (0);
}

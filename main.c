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
#include "ycoe_math.h"

// Unistd.h only for sleep
#include <unistd.h>
#include <pthread.h>
#include <mqueue.h>
#define IN_QUEUE  "/ycoe_inbound"
#define MAX_POSARR_LEN    2500


#define EC_TIMEOUTMON 500
#define ECAT_CYCLE_PERIOD 1000000

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

float current_time, previous_time, diff_time,prev_print_time=0;
float act_cycle_time,min_cycle_time = 999999, max_cycle_time = 0, sum_cycle_time = 0, avg_cycle_time = 0;
float act_latency,min_latency = 999999, max_latency = 0, sum_latency = 0, avg_latency = 0;
int cycle_count = 0;

void coeController(void *arg)
{
    char ifname[32]="wiznet";

    int i, chk;
    inOP = FALSE;
    int islaveindex;

    mqd_t mq;
    ssize_t mq_bytes_read;
    struct mq_attr mqattr;
    char pos_buffer[MAX_POSARR_LEN*sizeof(DINT)*2];

    /* initialize the queue attributes */
    mqattr.mq_flags = 0;
    mqattr.mq_maxmsg = 10;
    mqattr.mq_msgsize = MAX_POSARR_LEN*sizeof(DINT)*2;
    mqattr.mq_curmsgs = 0;

    /* create the message queue */
    mq = mq_open(IN_QUEUE, O_CREAT | O_RDONLY | O_NONBLOCK, 0644, &mqattr);


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
//ycoe_csp_setup_posarray(2,500,5);

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
            ec_receive_processdata(100);//(EC_TIMEOUTRET);
            /* request OP state for all slaves */
            ec_writestate(0);
            chk = 40;
            /* wait for all slaves to reach OP state */
            do
            {
                ec_send_processdata();
                ec_receive_processdata(100);//(EC_TIMEOUTRET);
                ec_statecheck(0, EC_STATE_OPERATIONAL, 50000);
            } while (chk-- && (ec_slave[0].state != EC_STATE_OPERATIONAL));

            if (ec_slave[0].state == EC_STATE_OPERATIONAL )
            {
                rt_printf("Operational state reached for all slaves.\n");
                inOP = TRUE;

ycoe_csp_setup_posarray(2,MAX_POSARR_LEN);
                /* cyclic loop */
				        cycle_count = 0;
                rt_task_sleep(1e6);
                rt_task_set_periodic(NULL, TM_NOW, ECAT_CYCLE_PERIOD);//1ms
                previous_time = rt_timer_read()/1000000.0;

                while(run)
                {
                    rt_task_wait_period(NULL);   //wait for next cycle

 				          	cycle_count++;

                    memset(pos_buffer, 0x00, sizeof(pos_buffer));
                    mq_bytes_read = mq_receive(mq, pos_buffer, 2*MAX_POSARR_LEN*sizeof(DINT), NULL);
                    if(mq_bytes_read >= 0) {
                        printf("SERVER: Received bytes = %d\n", (int)mq_bytes_read);
                        ycoe_csp_fill_posarray (2, MAX_POSARR_LEN, (DINT *)pos_buffer);
                    }

                    for (islaveindex = 1; islaveindex <= ec_slavecount; islaveindex++) {

                        if(ycoe_checkstatus(islaveindex,SW_SWITCHON_DISABLED))
                          ycoe_setcontrolword(islaveindex,CW_SHUTDOWN);
                        else if(ycoe_checkstatus(islaveindex,SW_RTSO))
                          ycoe_setcontrolword(islaveindex,CW_SWITCHON);
                        else if(ycoe_checkstatus(islaveindex,SW_SWITCHED_ON))
                        {
                            ycoe_setcontrolword(islaveindex,CW_ENABLEOP);
                            pos_cmd_sem[islaveindex] = 1;
                        }
                    }

                        if (ycoe_checkstatus(1,SW_OP_ENABLED) \
                              && ycoe_checkstatus(2,SW_OP_ENABLED))
                          {
                              // Add interpolation calculations
                              //ycoe_csp_loop_posarray(islaveindex);
                              ycoe_csp_follow_posarray(2);
                          }


                    ec_send_processdata();
                    wkc = ec_receive_processdata(100);//(EC_TIMEOUTRET);

                   current_time = rt_timer_read()/1000000.0;
                   diff_time = current_time - previous_time;
                   act_cycle_time = diff_time;
                   if (act_cycle_time < min_cycle_time) min_cycle_time = act_cycle_time;
                   if (act_cycle_time > max_cycle_time) max_cycle_time = act_cycle_time;
                   sum_cycle_time += act_cycle_time;

                   //act_latency = diff_time - prev_diff_time;
                   act_latency = diff_time - (ECAT_CYCLE_PERIOD/1000000.0);
                   //if (act_latency < 0) act_latency = -act_latency;
                   if (act_latency < min_latency) min_latency = act_latency;
                   if (act_latency > max_latency) max_latency = act_latency;
                   if (act_latency < 0) act_latency = -act_latency;
                   sum_latency += act_latency;

                   //if (cycle_count % 100 == 0)
                   if (current_time > prev_print_time + 100) {
                      avg_cycle_time = sum_cycle_time/((float)cycle_count);
                      avg_latency = sum_latency/((float)cycle_count);
                      rt_printf("cycle:%d time:%.5f act:%.5f min:%.5f avg:%.5f max:%.5f\n\r",cycle_count,current_time,act_cycle_time,min_cycle_time,avg_cycle_time,max_cycle_time);
                      rt_printf("lcycle:%d time:%.5f act:%.5f min:%.5f avg:%.5f max:%.5f\r\n",cycle_count,current_time,act_latency,min_latency,avg_latency,max_latency);
                      prev_print_time = current_time;
                      min_cycle_time=999999, max_cycle_time = 0;
                      min_latency=999999, max_latency = 0;
                      sum_cycle_time = 0, sum_latency = 0;
                      cycle_count = 0;
                   }
                   //prev_diff_time = diff_time;
                   previous_time = current_time;
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
        /* mq cleanup */
        mq_close(mq);
        mq_unlink(IN_QUEUE);
    }
    else
    {
      rt_printf("No socket connection on %s\nExcecute as root\n",ifname);
    }
}

void *mediator(void *args) {
  mqd_t mq;
  /* open the mail queue */
  mq = mq_open(IN_QUEUE, O_WRONLY);

  DINT *_sin_pos_arr = malloc(2*MAX_POSARR_LEN*sizeof(DINT));
  sinfill(_sin_pos_arr, 0, 6400000.0, MAX_POSARR_LEN);//6400000=100000counts/s
  sinfill(_sin_pos_arr+MAX_POSARR_LEN, 0, 6400000.0, MAX_POSARR_LEN);// 800000=12500counts/s

  DINT *_tri_pos_arr = malloc(2*MAX_POSARR_LEN*sizeof(DINT));
  trifill(_tri_pos_arr, 0, 6400000.0, MAX_POSARR_LEN);//6400000=100000counts/s
  trifill(_tri_pos_arr+MAX_POSARR_LEN, 0, 6400000.0, MAX_POSARR_LEN);// 800000=12500counts/s

  while (1) {
    //ycoe_csp_fill_posarray (num_slaves, _pos_arr);
    mq_send(mq, (char *)_sin_pos_arr, 2*MAX_POSARR_LEN*sizeof(DINT), 0);
    sleep(3);// Sleep 3 seconds
    mq_send(mq, (char *)_tri_pos_arr, 2*MAX_POSARR_LEN*sizeof(DINT), 0);
    sleep(3);// Sleep 3 seconds
  }
  free(_sin_pos_arr);
  free(_tri_pos_arr);
  mq_close(mq);

  return NULL;
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
    /* start cyclic part */
    rt_task_create(&engine_task, "ycoe_engine", 0, 99, 0 );
    rt_task_start(&engine_task, &coeController, NULL);

    pthread_t thread1;
    pthread_create(&thread1, NULL, mediator, argv[1]);
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

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

#define EC_TIMEOUTMON 500

char IOmap[4096];
OSAL_THREAD_HANDLE thread1;
int expectedWKC;
boolean needlf;
volatile int wkc;
boolean inOP;
uint8 currentgroup = 0;

void coeController(char *ifname)
{
    int i, j, oloop, iloop, chk;
    needlf = FALSE;
    inOP = FALSE;

    printf("Starting YaskawaCoE master\n");

    /* initialise SOEM, bind socket to ifname */
    if (ec_init(ifname))
    {
        printf("ec_init on %s succeeded.\n",ifname);

        /* find and auto-config slaves */
        if ( ec_config_init(FALSE) > 0 )
        {
            printf("%d slaves found and configured. PRE_OP requested on all slaves.\n",ec_slavecount);

            /* read PDO mapping and set local buffer for PDO exchange */
            ec_config_map(&IOmap);
            printf("Slaves mapped, state to SAFE_OP requested.\n");

            /* Configure Distributed Clock mechanism */
            ec_configdc();

            /* wait for all slaves to reach SAFE_OP state */
            ec_statecheck(0, EC_STATE_SAFE_OP,  EC_TIMEOUTSTATE * 4);


            /* slave0 is the master. All slaves pdos are mapped to slave0 */
            oloop = ec_slave[0].Obytes; /* Obytes are the total number of output bytes consolidated from all slaves */
            if ((oloop == 0) && (ec_slave[0].Obits > 0)) oloop = 1;
            if (oloop > 8) oloop = 8;
            iloop = ec_slave[0].Ibytes; /* Ibytes are the total number of input bytes consolidated from all slaves */
            if ((iloop == 0) && (ec_slave[0].Ibits > 0)) iloop = 1;
            if (iloop > 8) iloop = 8;

            printf("segments : %d : %d %d %d %d\n",ec_group[0].nsegments ,ec_group[0].IOsegment[0],ec_group[0].IOsegment[1],ec_group[0].IOsegment[2],ec_group[0].IOsegment[3]);

	          /* Check & Set Profile Position Mode Parameters */
            ycoe_ppm_get_parameters();
            ycoe_set_mode_of_operation(PROFILE_POSITION_MODE);
            ycoe_ppm_set_velocity(502400);

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

                int pos_cmd_sem = 1; // Position Command Semaphore
                DINT pos_cmds[1] = {0xFFFFFF};
                /* cyclic loop */
                for(i = 1; i <= 10000; i++)
                {
                    ycoe_printstatus(1);

                    //if (i<10) //ec_slave[0].outputs[0] = CW_SHUTDOWN;
                    if(ycoe_checkstatus(1,SW_SWITCHON_DISABLED))
                      ycoe_setcontrolword(1,CW_SHUTDOWN);
                    //else if (i<20) //ec_slave[0].outputs[0] = CW_SWITCHON;
                    else if(ycoe_checkstatus(1,SW_RTSO))
                      ycoe_setcontrolword(1,CW_SWITCHON);
		                //else if (i<30)
                    else if(ycoe_checkstatus(1,SW_SWITCHED_ON))
                    {
			                  //ec_slave[0].outputs[0] = CW_ENABLEOP;
                        ycoe_setcontrolword(1,CW_ENABLEOP);
			                  /*ec_slave[0].outputs[2] = 0xFF; //targetposition
			                  ec_slave[0].outputs[3] = 0xFF;
			                  ec_slave[0].outputs[4] = 0xFF;*/
                        ycoe_set_slave_position (1,0xFFFFFF);
                    }
		                //else if (i<40) //ec_slave[0].outputs[0] = CW_ENABLEOP | CW_PPM_SNPI1; //startnextposition
                    else {
                      if (ycoe_checkstatus(1,SW_OP_ENABLED))
                      {

                        if (ycoe_ppm_checkcontrol(1, CW_PPM_SNPI2) && \
                            ycoe_ppm_checkstatus(1,SW_SETPOINT_ACK)) {
                          pos_cmd_sem--;
                          ycoe_setcontrolword(1,CW_ENABLEOP | CW_PPM_SNPI1);
                        }
                        //else if (i<50) //ec_slave[0].outputs[0] = CW_ENABLEOP | CW_PPM_SNPI2; //startnextpositionimmediately
                        else if ((pos_cmd_sem>0) && ycoe_ppm_checkcontrol(1, CW_PPM_SNPI1))
                          ycoe_setcontrolword(1,CW_ENABLEOP | CW_PPM_SNPI2);
                        else if (pos_cmd_sem>0)// Only in PP mode, this means CW = 0x0F
                          ycoe_setcontrolword(1,CW_ENABLEOP | CW_PPM_SNPI1);
                      }
                      //else if (i<60) //ec_slave[0].outputs[0] = CW_ENABLEOP | CW_PPM_SNPI1; //startnextposition
                      //else if (ycoe_ppm_checkstatus(1,SW_SETPOINT_ACK))
                      //ycoe_setcontrolword(1,CW_ENABLEOP | CW_PPM_SNPI1);
                      //else ec_slave[0].outputs[0] = 0x0;
                    }

                    ec_send_processdata();
                    wkc = ec_receive_processdata(EC_TIMEOUTRET);

                    if(wkc >= expectedWKC)
                    {
                      printf("PDO cycle %4d, WKC %d , T:%"PRId64"\r\n", i, wkc, ec_DCtime);
                      needlf = TRUE;

                      printf(" O:");
                      for(j = 0 ; j < oloop; j++)
                        printf(" %2.2x", *(ec_slave[0].outputs + j));

                      printf("\tI:");
                      for(j = 0 ; j < iloop; j++)
                        printf(" %2.2x", *(ec_slave[0].inputs + j));
                      printf("\n");
                   }
                    osal_usleep(5000);

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
      if (needlf)
      {
        needlf = FALSE;
        printf("\n");
      }
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

int main(int argc, char *argv[])
{
  printf("YaskawaCoE (Yaskawa Canopen over Ethercat Master)\nControl Application\n");

  if (argc > 1)
  {
    /* create thread to handle slave error handling in OP */
    //      pthread_create( &thread1, NULL, (void *) &ecatcheck, (void*) &ctime);
    osal_thread_create(&thread1, 128000, &ecatcheck, (void*) &ctime);
    /* start cyclic part */
    coeController(argv[1]);
  }
  else
  {
    printf("Usage: yaskawaCoE ifname1\nifname = eth0 for example\n");
  }

  printf("End program\n");
  return (0);
}
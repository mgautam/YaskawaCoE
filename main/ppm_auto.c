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

#include "zmq.h"

#include "yaskawacoe.h"
#include "ycoe_engine.h"

#ifndef _WIN32
#include <pthread.h>
#endif

#ifdef _WIN32
extern HANDLE ecat_mutex;
#else
extern pthread_mutex_t ecat_mutex;
#endif

extern int ycoestate;


char *network_datamap_ptr;
int network_datamap_size = 0;

int pos_cmd_sem[3] = {0,0,0}; // Position Command Semaphore
/* Server for talking to GUI Application */
OSAL_THREAD_FUNC controlserver(void *ptr) {
  switch_to_next_ycoestate();
  switch_to_next_ycoestate();


  int position_request = 1500000000;
  int islaveindex, numslaves;
  while (1) {
      osal_usleep(900000);
#ifdef _WIN32
      WaitForSingleObject(ecat_mutex, INFINITE);
#else
      pthread_mutex_lock(&ecat_mutex);
#endif
    network_datamap_size = ycoe_get_datamap(&network_datamap_ptr);
    memcpy(&numslaves,network_datamap_ptr,4);

    // Auto Sequencing
    if (ycoe_ppm_checkstatus(1, SW_PPM_TARGET_REACHED) &&
        ycoe_ppm_checkstatus(2, SW_PPM_TARGET_REACHED)) {
        if (position_request > 0) position_request = 0;
        else position_request = 300000000;
        ycoe_ppm_set_position(1, position_request);
        ycoe_ppm_set_position(2, position_request);
        pos_cmd_sem[1]++;
        pos_cmd_sem[2]++;
    }
    //Auto Sequencing ends

    // Control Logic
    for (islaveindex = 1; islaveindex <= numslaves; islaveindex++) {
      //ycoe_printstatus(1);
      if(ycoe_checkstatus(islaveindex,SW_SWITCHON_DISABLED)) {
         /* Check & Set Profile Position Mode Parameters */
         //ycoe_ppm_get_parameters(islaveindex);
         ycoe_ppm_setup(islaveindex);
         ycoe_set_mode_of_operation(islaveindex,PROFILE_POSITION_MODE);
         ycoe_ppm_set_parameters(islaveindex,54700000,1024,1048576,1048576);//10485760
         ycoe_ppm_get_parameters(islaveindex);
         ycoe_setcontrolword(islaveindex,CW_SHUTDOWN);
      }
      else if(ycoe_checkstatus(islaveindex,SW_RTSO))
        ycoe_setcontrolword(islaveindex,CW_SWITCHON);
      else if(ycoe_checkstatus(islaveindex,SW_SWITCHED_ON))
      {
          ycoe_setcontrolword(islaveindex,CW_ENABLEOP);
          //ycoe_ppm_set_position (islaveindex,8192);
          //pos_cmd_sem[islaveindex] = 1;
      }
      else {
        if (ycoe_checkstatus(islaveindex,SW_OP_ENABLED))
        {
          if (ycoe_ppm_checkcontrol(islaveindex, CW_PPM_SNPI2) && \
              ycoe_ppm_checkstatus(islaveindex,SW_PPM_SETPOINT_ACK)) {
            pos_cmd_sem[islaveindex]--;
            printf("Slave %d acknowledged pos_cmd_sem=%d\n\r",islaveindex,pos_cmd_sem[islaveindex]);
            ycoe_setcontrolword(islaveindex,CW_ENABLEOP | CW_PPM_SNPI1);
          }
          else if ((pos_cmd_sem[islaveindex]>0) && ycoe_ppm_checkcontrol(islaveindex, CW_PPM_SNPI1))
            ycoe_setcontrolword(islaveindex,CW_ENABLEOP | CW_PPM_SNPI2);
          else if (pos_cmd_sem[islaveindex]>0)// Only in PP mode, this means CW = 0x0F
            ycoe_setcontrolword(islaveindex,CW_ENABLEOP | CW_PPM_SNPI1);
        }
      }
    }
    // Control Logic Ends


#ifdef _WIN32
      ReleaseMutex(ecat_mutex);
#else
      pthread_mutex_unlock(&ecat_mutex);
#endif
  }
}

int main(int argc, char *argv[])
{
  OSAL_THREAD_HANDLE thread1;
#ifdef _WIN32
  ecat_mutex = CreateMutex(
      NULL,              // default security attributes
      FALSE,             // initially not owned
      NULL);             // unnamed mutex
  if (ecat_mutex == NULL)
  {
    printf("CreateMutex error: %d\n", GetLastError());
    return 1;
  }
#else
  //ecat_mutex = PTHREAD_MUTEX_INITALIZER;
  pthread_mutex_init(&ecat_mutex, NULL);
#endif
  printf("YaskawaCoE (Yaskawa Canopen over Ethercat Master)\nControl Application\n");

  if (argc > 1)
  {
    /* start cyclic part */
    osal_thread_create(&thread1, 128000, &ycoe_engine, argv[1]);
    // thread to handle gui application requests
    controlserver(argv[1]);
  }
  else
  {
    printf("Usage: yaskawaCoE ifname1\nifname = eth0 for example\n");
  }

#ifdef _WIN32
  CloseHandle(ecat_mutex);
#else
  pthread_mutex_destroy(&ecat_mutex);
#endif
  printf("End program\n");
  return (0);
}

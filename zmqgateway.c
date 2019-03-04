#include <math.h>
#include "ycoe_math.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <zmq.h>

#define NUM_SLAVES 4
#define DRV_POSARR_LEN 3000
#define RCV_BUF_MULT 100
int MAX_POSRCV_LEN = (DRV_POSARR_LEN * RCV_BUF_MULT);

DINT DRV_POSBUF[NUM_SLAVES*DRV_POSARR_LEN] = {0};
int main(int argc, char *argv[]) {
  int i = 0, j = 0;
  printf("Send YCOE Position Data program\n");
  DINT *_pos_arr = malloc(NUM_SLAVES*MAX_POSRCV_LEN*sizeof(DINT));

  /*  // Single frequency sine profile
      for (i=0; i < NUM_SLAVES; i++)
    sinfill1(_pos_arr+i*MAX_POSRCV_LEN, 0, 6400000.0, 1000, MAX_POSRCV_LEN);//6400000=100000counts/s
*/

  // Multi frequency sine profile
    for (i=0; i < NUM_SLAVES; i++)
    {
      for (j=0; j < 100; j++) {
        int k=j%3;
        sinfill1(_pos_arr+i*MAX_POSRCV_LEN+j*DRV_POSARR_LEN, 0, 6400000.0, 3000/(k+1), DRV_POSARR_LEN);//6400000=100000counts/s
      }
    }
  printf("Sinefill dest:%ld\n",_pos_arr);

/* // Triangular Profile
  for (i=0; i < NUM_SLAVES; i++)
    trifill(_pos_arr+i*MAX_POSRCV_LEN, 0, 2000000000.0, MAX_POSRCV_LEN);// 800000=12500counts/s
*/
    char buffer[15] = {0};
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);
    zmq_connect (requester, "tcp://localhost:5555");


    i=0,j=0;
    int usleep_time=3000*1000;
    int usleep_buffer=50*1000;
    int n;
    void *memdest, *memsrc;
    while (1)
    {
      if (j>=RCV_BUF_MULT) j=0;

      for (n=0; n< NUM_SLAVES; n++) {
/*        memdest = ((void *)DRV_POSBUF)+n*DRV_POSARR_LEN*sizeof(DINT);
        memsrc = ((void *)_sin_pos_arr)+((n*MAX_POSRCV_LEN+j*DRV_POSARR_LEN)*sizeof(DINT));
*/        memdest = DRV_POSBUF+n*DRV_POSARR_LEN;
        memsrc = _pos_arr+n*MAX_POSRCV_LEN+j*DRV_POSARR_LEN;
        memcpy ( memdest, memsrc, DRV_POSARR_LEN*sizeof(DINT) );
      }
printf("%d,%d:Relay Gateway - dst:%ld, src:%ld\n",i,j,memdest,memsrc);
      zmq_send(requester, (char *)DRV_POSBUF, NUM_SLAVES*DRV_POSARR_LEN*sizeof(DINT), 0);
      zmq_recv(requester,buffer,12,0);
      usleep(usleep_time-usleep_buffer);// Sleep 6 seconds

/*printf("%d:Send trI\n",i);
      zmq_send(requester, (char *)_tri_pos_arr, NUM_SLAVES*MAX_POSARR_LEN*sizeof(DINT), 0);
      zmq_recv(requester,buffer,12,0);
      usleep(usleep_time-usleep_buffer);// Sleep 6 seconds
*/
      i++,j++;
      if (i%10==0) {
        usleep(usleep_buffer*9);
        if (i%100==0) {
          usleep(usleep_buffer*9);
          if (i%1000==0) {
            usleep(usleep_buffer*9);
          }
        }
      }
    }
    free(_pos_arr);
    //free(_tri_pos_arr);

    zmq_close(requester);
    zmq_ctx_destroy(context);
   printf("End program\n");
  return (0);
}

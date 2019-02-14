
#include "ycoe_math.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <zmq.h>

#define NUM_SLAVES 4
#define MAX_POSARR_LEN 3000

int main(int argc, char *argv[]) {
  int i = 0;
   printf("Send YCOE Position Data program\n");
   DINT *_sin_pos_arr = malloc(NUM_SLAVES*MAX_POSARR_LEN*sizeof(DINT));
   for (i=0; i < NUM_SLAVES; i++)
    sinfill1(_sin_pos_arr+i*MAX_POSARR_LEN, 0, 6400000.0, 1000, MAX_POSARR_LEN);//6400000=100000counts/s

    DINT *_tri_pos_arr = malloc(NUM_SLAVES*MAX_POSARR_LEN*sizeof(DINT));
   for (i=0; i < NUM_SLAVES; i++)
    trifill(_tri_pos_arr+i*MAX_POSARR_LEN, 0, 6400000.0, MAX_POSARR_LEN);// 800000=12500counts/s

    char buffer[15] = {0};
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);
    zmq_connect (requester, "tcp://localhost:5555");


    i=0;
    int usleep_time=3000*1000;
    int usleep_buffer=50*1000;
    while (1)
    {
      i++;
printf("%d:Send sine\n",i);
      zmq_send(requester, (char *)_sin_pos_arr, NUM_SLAVES*MAX_POSARR_LEN*sizeof(DINT), 0);
      zmq_recv(requester,buffer,12,0);
      usleep(usleep_time-usleep_buffer);// Sleep 6 seconds

printf("%d:Send trI\n",i);
      zmq_send(requester, (char *)_tri_pos_arr, NUM_SLAVES*MAX_POSARR_LEN*sizeof(DINT), 0);
      zmq_recv(requester,buffer,12,0);
      usleep(usleep_time-usleep_buffer);// Sleep 6 seconds

      if (i%10==0)
        usleep(usleep_buffer*19);
    }
    free(_sin_pos_arr);
    free(_tri_pos_arr);

    zmq_close(requester);
    zmq_ctx_destroy(context);
   printf("End program\n");
  return (0);
}

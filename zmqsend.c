
#include "ycoe_math.h"
#include <stdio.h>
#include <unistd.h>
#include <zmq.h>

#define MAX_POSARR_LEN    2500

int main(int argc, char *argv[]) {
   printf("Send YCOE Position Data program\n");
   DINT *_sin_pos_arr = malloc(2*MAX_POSARR_LEN*sizeof(DINT));
    sinfill(_sin_pos_arr, 0, 6400000.0, MAX_POSARR_LEN);//6400000=100000counts/s
    sinfill(_sin_pos_arr+MAX_POSARR_LEN, 0, 6400000.0, MAX_POSARR_LEN);// 800000=12500counts/s

    DINT *_tri_pos_arr = malloc(2*MAX_POSARR_LEN*sizeof(DINT));
    trifill(_tri_pos_arr, 0, 6400000.0, MAX_POSARR_LEN);//6400000=100000counts/s
    trifill(_tri_pos_arr+MAX_POSARR_LEN, 0, 6400000.0, MAX_POSARR_LEN);// 800000=12500counts/s

    char buffer[15] = {0};
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);
    zmq_connect (requester, "tcp://localhost:5555");
    while (1)
    {
      zmq_send(requester, (char *)_sin_pos_arr, 2*MAX_POSARR_LEN*sizeof(DINT), 0);
      zmq_recv(requester,buffer,12,0);
      sleep(3);// Sleep 3 seconds
      zmq_send(requester, (char *)_tri_pos_arr, 2*MAX_POSARR_LEN*sizeof(DINT), 0);
      zmq_recv(requester,buffer,12,0);
      sleep(3);// Sleep 3 seconds*/
      //osal_usleep(300000);
    }
    free(_sin_pos_arr);
    free(_tri_pos_arr);

    zmq_close(requester);
    zmq_ctx_destroy(context);
   printf("End program\n");
  return (0);
}

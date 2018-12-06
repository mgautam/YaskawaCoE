
#include "ycoe_math.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
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

    DINT *_pos_arr = malloc(2*MAX_POSARR_LEN*sizeof(DINT));

    char buffer[15] = {0};
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);
    zmq_connect (requester, "tcp://localhost:5555");
    while (1)
    {
      memset(_pos_arr,0, MAX_POSARR_LEN*sizeof(DINT));
      memcpy((_pos_arr+MAX_POSARR_LEN),_tri_pos_arr, MAX_POSARR_LEN*sizeof(DINT));
      zmq_send(requester, (char *)_pos_arr, 2*MAX_POSARR_LEN*sizeof(DINT), 0);
      zmq_recv(requester,buffer,12,0);
      sleep(3);// Sleep 3 seconds

      memcpy(_pos_arr,_tri_pos_arr, MAX_POSARR_LEN*sizeof(DINT));
      memset((_pos_arr+MAX_POSARR_LEN),0, MAX_POSARR_LEN*sizeof(DINT));
      zmq_send(requester, (char *)_pos_arr, 2*MAX_POSARR_LEN*sizeof(DINT), 0);
      zmq_recv(requester,buffer,12,0);
      sleep(3);// Sleep 3 seconds*/

      memcpy(_pos_arr,_tri_pos_arr, MAX_POSARR_LEN*sizeof(DINT));
      memcpy((_pos_arr+MAX_POSARR_LEN),_tri_pos_arr, MAX_POSARR_LEN*sizeof(DINT));
      zmq_send(requester, (char *)_pos_arr, 2*MAX_POSARR_LEN*sizeof(DINT), 0);
      zmq_recv(requester,buffer,12,0);
      sleep(3);// Sleep 3 seconds

      memcpy(_pos_arr,_sin_pos_arr, MAX_POSARR_LEN*sizeof(DINT));
      memcpy((_pos_arr+MAX_POSARR_LEN),_tri_pos_arr, MAX_POSARR_LEN*sizeof(DINT));
      zmq_send(requester, (char *)_pos_arr, 2*MAX_POSARR_LEN*sizeof(DINT), 0);
      zmq_recv(requester,buffer,12,0);
      sleep(3);// Sleep 3 seconds

      memcpy(_pos_arr,_tri_pos_arr, MAX_POSARR_LEN*sizeof(DINT));
      memcpy((_pos_arr+MAX_POSARR_LEN),_sin_pos_arr, MAX_POSARR_LEN*sizeof(DINT));
      zmq_send(requester, (char *)_pos_arr, 2*MAX_POSARR_LEN*sizeof(DINT), 0);
      zmq_recv(requester,buffer,12,0);
      sleep(3);// Sleep 3 seconds*/
      
      memcpy(_pos_arr,_sin_pos_arr, MAX_POSARR_LEN*sizeof(DINT));
      memcpy((_pos_arr+MAX_POSARR_LEN),_sin_pos_arr, MAX_POSARR_LEN*sizeof(DINT));
      zmq_send(requester, (char *)_pos_arr, 2*MAX_POSARR_LEN*sizeof(DINT), 0);
      zmq_recv(requester,buffer,12,0);
      sleep(3);// Sleep 3 seconds
    }
    free(_sin_pos_arr);
    free(_tri_pos_arr);

    zmq_close(requester);
    zmq_ctx_destroy(context);
   printf("End program\n");
  return (0);
}

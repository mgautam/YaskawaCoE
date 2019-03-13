#include <math.h>
#include "ycoe_math.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <zmq.h>

#define NUM_SLAVES 4
#define DRV_POSARR_LEN 3000
#define RCV_BUF_MULT 1000
int MAX_POSRCV_LEN = (DRV_POSARR_LEN * RCV_BUF_MULT);

DINT DRV_POSBUF[NUM_SLAVES*DRV_POSARR_LEN] = {0};
int main(int argc, char *argv[]) {
  int i = 0, j = 0, k;
  printf("YCOE Position Gateway\n");
  DINT *_in_pos_arr = malloc((NUM_SLAVES*MAX_POSRCV_LEN+1)*sizeof(DINT));
  DINT *_out_pos_arr = _in_pos_arr + 1;
  DINT *maxfillcount=_in_pos_arr;

    char buffer[15] = {0};
    void *context = zmq_ctx_new ();
    void *in_gate = zmq_socket (context, ZMQ_REP);
    void *out_gate = zmq_socket (context, ZMQ_REQ);
    zmq_bind (in_gate, "tcp://*:6666");
    zmq_connect (out_gate, "tcp://localhost:5555");


    i=0,j=0;
    int usleep_time=3000*1000;
    int usleep_buffer=50*1000;
    int n;
    void *memdest, *memsrc;
    while (1)
    {
      printf("Posdata Rcvd wait...\n");
      zmq_recv(in_gate, (char *)_in_pos_arr, (NUM_SLAVES*MAX_POSRCV_LEN+1)*sizeof(DINT), 0);

      printf("Posdata Rcvd!workbytes=%ld\n", *maxfillcount);
          /*for (n=0;n<NUM_SLAVES;n++) {
          printf("Poss %d:",n);
          for (k=0;k<5;k++)
            printf("%ld\t", _out_pos_arr[k+n*MAX_POSRCV_LEN]);
          printf("\n");
          }*/

      zmq_send(in_gate, _in_pos_arr, 12, 0);
      printf("Posdata Response Sent!\n");
      i++,j=0;
      unsigned int numchunks = (*maxfillcount/DRV_POSARR_LEN) + 1;
      if (numchunks > RCV_BUF_MULT) numchunks = RCV_BUF_MULT;
      while (j < numchunks) {
          for (n=0; n< NUM_SLAVES; n++) {
            memdest = DRV_POSBUF+n*DRV_POSARR_LEN;
            memsrc = _out_pos_arr+n*MAX_POSRCV_LEN+j*DRV_POSARR_LEN;
            memcpy ( memdest, memsrc, DRV_POSARR_LEN*sizeof(DINT) );
          }
          j++;//This has to be below the memcpy otherwise it skips first chuck
    printf("%d,%d:Relay Gateway - dstpos:%ld, src:%ld\n",i,j,*(DINT *)memdest,memsrc);

          for (n=0;n<NUM_SLAVES;n++) {
          printf("Poss %d:",n);
          for (k=0;k<5;k++)
            printf("%ld\t", DRV_POSBUF[k+n*DRV_POSARR_LEN]);
          printf("\n");
          }

          zmq_send(out_gate, (char *)DRV_POSBUF, NUM_SLAVES*DRV_POSARR_LEN*sizeof(DINT), 0);
          zmq_recv(out_gate,buffer,12,0);

          usleep(usleep_time-usleep_buffer);// Sleep 6 seconds

        if (j%10==0) {
          usleep(usleep_buffer*9);
          if (j%100==0) {
            usleep(usleep_buffer*3);
            if (j%1000==0) {
              usleep(usleep_buffer*1);
            }
          }
        }
      }
    }
    free(_in_pos_arr);

    zmq_close(in_gate);
    zmq_close(out_gate);
    zmq_ctx_destroy(context);
   printf("End program\n");
  return (0);
}

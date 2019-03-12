#include <math.h>
#include "ycoe_math.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <zmq.h>

#define NUM_SLAVES 4
#define DRV_POSARR_LEN 3000
#define RCV_BUF_MULT 500
int MAX_POSRCV_LEN = (DRV_POSARR_LEN * RCV_BUF_MULT);

int main(int argc, char *argv[]) {
  int i = 0, j =0;
  printf("Send YCOE Position Data program\n");
  DINT *_pos_arr = malloc(NUM_SLAVES*MAX_POSRCV_LEN*sizeof(DINT));

  /*  // Single frequency sine profile
      for (i=0; i < NUM_SLAVES; i++)
    sinfill1(_pos_arr+i*MAX_POSRCV_LEN, 0, 6400000.0, 1000, MAX_POSRCV_LEN);//6400000=100000counts/s
*/
/*
  // Multi frequency sine profile
    for (i=0; i < NUM_SLAVES; i++)
    {
      for (j=0; j < 100; j++) {
        int k=j%3;
        sinfill1(_pos_arr+i*MAX_POSRCV_LEN+j*DRV_POSARR_LEN, 0, 6400000.0, 3000/(k+1), DRV_POSARR_LEN);//6400000=100000counts/s
      }
    }
  printf("Sinefill dest:%ld\n",_pos_arr);
*/
/* // Triangular Profile
for (i=0; i < NUM_SLAVES; i++)
  trifill(_pos_arr+i*MAX_POSRCV_LEN, 0, 2000000000.0, MAX_POSRCV_LEN);// 800000=12500counts/s
*/
/* // staircase velocity profile
  for (i=0; i < num_slaves; i++) {
    stairfill(_pos_arr+i*max_posrcv_len, 0, 2140000000.0, 0.5, 100, max_posrcv_len);// 800000=12500counts/s
  }*/
 // multi-staircase velocity profile
/*  for (i=0; i < NUM_SLAVES; i++) {
    for (j=0; j < 10; j++) {
      stairfill(_pos_arr+i*MAX_POSRCV_LEN+j*MAX_POSRCV_LEN/10, 0, 2140000000.0, 0.5, 100, MAX_POSRCV_LEN/10);// 800000=12500counts/s
    }
  }
*/
  unsigned int fillcount, k;
  DINT maxfillcount=0;
  for (i=0; i < NUM_SLAVES; i++) {
    fillcount = vapfill(_pos_arr+1+i*MAX_POSRCV_LEN,3,104857600,10485760,104857600);
    if (maxfillcount < fillcount) maxfillcount = fillcount;
    for (k=i*MAX_POSRCV_LEN+fillcount;k<(i+1)*MAX_POSRCV_LEN;k++)
      _pos_arr[1+k]=_pos_arr[1+fillcount-1];

    printf("LastPos: %ld\n",_pos_arr[1+fillcount-1]);
    printf("Poss %d:",i);
          for (k=0;k<5;k++)
            printf("%ld\t", _pos_arr[1+k+i*MAX_POSRCV_LEN+19999]);
          printf("\n");
  }
  _pos_arr[0] = maxfillcount;
  unsigned int numchunks = (maxfillcount/DRV_POSARR_LEN) + 1;
  if(numchunks > RCV_BUF_MULT) numchunks = RCV_BUF_MULT;

    char buffer[15] = {0};
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);
    zmq_connect (requester, "tcp://10.1.1.5:6666");


    i=0;
    int usleep_time=3000*1000;
    int usleep_buffer=50*1000;
    if (1)
    {
      printf("Posdata Prepared!\n");
      zmq_send(requester, (char *)_pos_arr, NUM_SLAVES*MAX_POSRCV_LEN*sizeof(DINT), 0);
      printf("Posdata Sent!\n");
      zmq_recv(requester,buffer,12,0);
      printf("Posdata Response recvd!\n");

      for (i=0;i<numchunks;i++) {
        usleep(usleep_time);// Sleep 6 seconds
        printf("Sleep count=%d\n", i);
      }
    }
    free(_pos_arr);

    zmq_close(requester);
    zmq_ctx_destroy(context);
   printf("End program\n");
  return (0);
}

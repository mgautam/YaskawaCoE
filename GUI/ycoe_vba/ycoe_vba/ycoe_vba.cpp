// ycoe_vba.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "zmq.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
typedef __int32 DINT;

#define MAX_MSGMAP_LEN 4096

#define COMMAND_FAILED -1
#define COMMAND_SUCCESS 1
#define COMMAND_ERROR 2

static int connection_active = 0;
#define NUM_SLAVES 4
#define DRV_POSARR_LEN 3000
#define RCV_BUF_MULT 1000
int MAX_POSRCV_LEN = (DRV_POSARR_LEN * RCV_BUF_MULT);


float _stdcall ycoe_vba_version(void)
{
	float version = 3.00;
	return version;
}

unsigned int vapfill(DINT *array, double gr, double acceleration, double velocity, DINT distance) {
	double accperms = gr * acceleration / 1000000.0;
	double velperms = gr * velocity / 1000.0;
	DINT _distance = gr * distance;

	//printf("Apms=%lf, Vpms=%lf\n", accperms, velperms);

	double dist1, dist2;
	double vba = pow(gr*velocity, 2.0) / (gr*acceleration);
	if (_distance <= vba) {
		dist1 = _distance / 2.0;
		dist2 = _distance / 2.0;
	}
	else {
		dist1 = 0.5 * vba;
		dist2 = _distance - dist1;
	}
	//printf("Dist1=%lf, Dist2=%lf\n",dist1,dist2);

	double tempvel = 0;
	double currposptr = 0;
	unsigned int i = 0;
	while (currposptr < dist1) {
		tempvel += accperms;
		currposptr += tempvel;
		array[i++] = (DINT)currposptr;
		//printf("Fill1 %d= %lf\n",i,currposptr);
	}
	while (currposptr < dist2) {
		currposptr += velperms;
		array[i++] = (DINT)currposptr;
		//printf("Fill2 %d= %lf\n",i,currposptr);
	}
	//printf("Tempvel=%lf\n",tempvel);
	//tempvel = velperms;
	while (/*(currposptr < _distance) &&*/ (tempvel > 0)) {
		tempvel -= accperms;
		currposptr += tempvel;
		array[i++] = (DINT)currposptr;
		//printf("Fill3 %d= %lf\n",i,currposptr);
	}

	printf("Vapfill Count = %d\n", i);

	return i;
}

unsigned int rpsfill(DINT *array, double gr, double acceleration, double velocity, double distance) {
	return vapfill(array, gr, acceleration * 1048576, velocity * 1048576, distance * 1048576);
}


void *context;
void *requester;
int _stdcall ycoe_connect(char *data)
{
	connection_active = COMMAND_FAILED;
	context = zmq_ctx_new();
	requester = zmq_socket(context, ZMQ_REQ);
	zmq_connect(requester, "tcp://10.1.1.5:6666");
	connection_active = 1;
	return connection_active;
}

double gr[NUM_SLAVES] = { 1.0,1.0,1.0,1.0 };
double accel[NUM_SLAVES] = { 100.0,100.0,100.0,100.0 };
double speed[NUM_SLAVES] = { 10.0,10.0,10.0,10.0 };
double distance[NUM_SLAVES] = { 50.0,50.0,50.0,50.0 };
int _stdcall ycoe_setprofiler(int slaveindex, double _gr, double _acceleration, double _velocity, double _distance) {
	if ((slaveindex >=0) && (slaveindex < NUM_SLAVES)) {
		gr[slaveindex] = _gr;
		accel[slaveindex] = _acceleration;
		speed[slaveindex] = _velocity;
		distance[slaveindex] = _distance;
		return COMMAND_SUCCESS;
	}
	else
		return COMMAND_FAILED;
}

int _stdcall ycoe_getprofiler(int slaveindex, double *_gr, double *_acceleration, double *_velocity, double *_distance) {
	if ((slaveindex >= 0) && (slaveindex < NUM_SLAVES)) {
		*_gr = gr[slaveindex];
		*_acceleration = accel[slaveindex];
		*_velocity = speed[slaveindex];
		*_distance = distance[slaveindex];
		return COMMAND_SUCCESS;
	}
	else
		return COMMAND_FAILED;
}


int _stdcall ycoe_possend(void) {
	if (connection_active) {
		int i = 0, j = 0;
		DINT *_posmsg_arr = (DINT *)malloc(NUM_SLAVES * MAX_POSRCV_LEN * sizeof(DINT));
		DINT *_pos_arr = _posmsg_arr + 1;

		unsigned int fillcount, k;
		DINT maxfillcount = 0;
		for (i = 0; i < NUM_SLAVES; i++) {
			if ((gr[i] <= 0) || (accel[i] <= 0) || (speed[i] <= 0) || (distance[i] <= 0)) {
				_pos_arr[0] = 0;
				fillcount = 1;
			}
			else {
				fillcount = rpsfill(_pos_arr + i * MAX_POSRCV_LEN, gr[i], accel[i], speed[i], distance[i]);				
			}
			if (maxfillcount < fillcount) maxfillcount = fillcount;
			for (k = i * MAX_POSRCV_LEN + fillcount; k < (i + 1)*MAX_POSRCV_LEN; k++)
				_pos_arr[k] = _pos_arr[i*MAX_POSRCV_LEN + fillcount - 1];
			/*printf("LastPos: %ld\n", _pos_arr[fillcount - 1]);
			printf("Poss %d:", i);
			for (k = 0; k < 5; k++)
				printf("%ld\t", _pos_arr[k + i * MAX_POSRCV_LEN + 9999]);
			printf("\n");*/
		}
		_posmsg_arr[0] = maxfillcount;
		unsigned int numchunks = (maxfillcount / DRV_POSARR_LEN) + 1;
		if (numchunks > RCV_BUF_MULT) numchunks = RCV_BUF_MULT;

		char buffer[15] = { 0 };
		//void *context = zmq_ctx_new();
		//void *requester = zmq_socket(context, ZMQ_REQ);
		//zmq_connect(requester, "tcp://10.1.1.5:6666");


		i = 0;
		int sleep_time = 3000;
		
		if (1)
		{
			//printf("Posdata Prepared!\n");
			zmq_send(requester, (char *)_posmsg_arr, NUM_SLAVES*MAX_POSRCV_LEN * sizeof(DINT), 0);
			//printf("Posdata Sent!\n");
			zmq_recv(requester, buffer, 12, 0);
			//printf("Posdata Response recvd!\n");

			for (i = 0; i < numchunks; i++) {
				Sleep(sleep_time);// Sleep 6 seconds
				//printf("Sleep count=%d\n", i);
			}
		}
		//free(_posmsg_arr);


		return COMMAND_SUCCESS;// (int)maxfillcount;
	}
	else {
		return COMMAND_FAILED;
	}


}



int _stdcall ycoe_disconnect(char *data)
{
	connection_active = COMMAND_FAILED;
	zmq_close(requester);
	zmq_ctx_destroy(context);
	connection_active = 0;
	return connection_active;
}

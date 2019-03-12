#include <stdio.h>
#include <math.h>
#include <ycoe_math.h>
#define PI 3.14159265

int sinfill1(DINT *array, DINT origin, double radius, unsigned int period, unsigned int num_samples) {
    /* Max num_samples is 65535 */
    //array = (DINT *) malloc(sizeof(UDINT) * num_samples);

    double omega = 2.0*PI /(double)(period);

    int i;
    for (i=0; i<num_samples; i++) {
      array[i] = origin + (DINT) (radius * sin(omega * (double)i));
      //if ((i < 15)||(i > num_samples-15)) printf("%d->%d ",i,array[i]);
    }
    printf("Sinfill: %d\n",num_samples);
    return 0;
}
int sinfill(DINT *array, DINT origin, double radius, unsigned int num_samples) {
  sinfill1(array, origin, radius, num_samples, num_samples);
  return 0;
}

int cosfill(DINT *array, DINT origin, double radius, unsigned int num_samples) {
    //array = (DINT *) malloc(sizeof(UDINT) * num_samples);

    double omega = 2.0*PI /(double)(num_samples);

    int i;
    for (i=0; i<num_samples; i++) {
      array[i] = origin + (DINT) (radius * cos(omega * (double)i));
      //printf("%d->%d ",i,array[i]);
    }
    return 0;
}

int trifill1(DINT *array, DINT origin, double relative_height, unsigned int period, unsigned int num_samples) {
    /* Max num_samples is 65535 */
    //array = (DINT *) malloc(sizeof(UDINT) * num_samples);

    double slope = ((double) relative_height) / (double)period;

    int i;
    for (i=0; i<period; i++) {
      array[i] = origin + (DINT) (slope * (double)i);
      //if (i < 15) printf("%d->%d ",i,array[i]);
    }

    //printf("Trifill: %d\n",num_samples);
    return 0;
}
int trifill(DINT *array, DINT origin, double height, unsigned int num_samples) {
    trifill1(array, origin, height, num_samples/2, num_samples/2);
    trifill1(array+(num_samples/2), origin+height, -height, num_samples/2, num_samples/2);
    return 0;
}

int stairfill1(DINT *array, DINT origin, double height, double vel_divider, unsigned int num_stairs, unsigned int num_samples) {
    int period = num_samples / num_stairs;
    //printf("period=%d\n",period);
    double max_vel = height / ((double) period);
    //printf("height=%lf,max_vel=%lf\n",height,max_vel);
    max_vel = max_vel * (1 - vel_divider) / (1 - pow(vel_divider,num_stairs));
    //printf("max_vel=%lf\n",max_vel);

    int i;
    double new_height;
    DINT new_origin=origin;
    for (i=0; i < num_stairs; i++) {
      new_height = ((double)period) * max_vel * pow(vel_divider,i);
      //printf("%d new_origin=%ld new_height=%lf \n",i,new_origin,new_height);
      trifill1(array+i*period, new_origin, new_height, period, period);
      new_origin += (DINT) new_height;
    }
    return 0;
}

int stairfill2(DINT *array, DINT origin, double height, double vel_divider, unsigned int num_stairs, unsigned int num_samples) {
    int period = num_samples / num_stairs;
    //printf("period=%d\n",period);
    double max_vel = height / ((double) period);
    //printf("height=%lf,max_vel=%lf\n",height,max_vel);
    double vel_incr = max_vel* 2.0 / (num_stairs *(num_stairs+1));
    //printf("vel_incr=%lf\n",vel_incr);

    int i;
    double new_height;
    DINT new_origin=origin;
    for (i=0; i < num_stairs; i++) {
      new_height = ((double)(period * (i+1)))* vel_incr;
      //printf("%d new_origin=%ld new_height=%lf \n",i,new_origin,new_height);
      trifill1(array+i*period, new_origin, new_height, period, period);
      new_origin += (DINT) new_height;
    }
    return 0;
}

int stairfill(DINT *array, DINT origin, double height, double vel_divider, unsigned int num_stairs, unsigned int num_samples){
  stairfill2(array, origin, height, vel_divider, num_stairs, num_samples/2);
  stairfill2(array+(num_samples/2), origin+height, -height, vel_divider, num_stairs, num_samples/2);
}

unsigned int vapfill (DINT *array, double gr, double acceleration, double velocity, DINT distance) {
    double accperms = gr*acceleration/1000000.0;
    double velperms = gr*velocity/1000.0;
    DINT _distance = gr*distance;

    //printf("Apms=%lf, Vpms=%lf\n", accperms, velperms);

    double dist1,dist2;
    double vba = pow(gr*velocity,2.0)/(gr*acceleration);
    if (_distance <= vba) {
      dist1 = _distance/2.0;
      dist2 = _distance/2.0;
    } else {
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
      array[i++] = (DINT) currposptr;
      //printf("Fill1 %d= %lf\n",i,currposptr);
    }
    while (currposptr < dist2) {
      currposptr += velperms;
      array[i++] = (DINT) currposptr;
      //printf("Fill2 %d= %lf\n",i,currposptr);
    }
    //printf("Tempvel=%lf\n",tempvel);
    //tempvel = velperms;
    while (/*(currposptr < _distance) &&*/ (tempvel > 0)){
      tempvel -= accperms;
      currposptr += tempvel;
      array[i++] = (DINT) currposptr;
      //printf("Fill3 %d= %lf\n",i,currposptr);
    }

    printf("Vapfill Count = %d\n",i);

    return i;
}

unsigned int rpsfill (DINT *array, double gr, double acceleration, double velocity, double distance) {
  return vapfill(array, gr, acceleration*1048576, velocity*1048576, distance*1048576);
}

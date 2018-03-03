//#include <stdio.h>
#include <math.h>
#include <ycoe_math.h>
#define PI 3.14159265

int sinfill(DINT *array, double radius, unsigned int num_samples) {
    /* Max num_samples is 65535 */
    //array = (DINT *) malloc(sizeof(UDINT) * num_samples);

    double omega = 2.0*PI /(double)(num_samples);

    int i;
    for (i=0; i<num_samples; i++) {
      array[i] = (DINT) (radius * sin(omega * (double)i));
      //printf("%d->%d ",i,array[i]);
    }
    return 0;
}

int cosfill(DINT *array, double radius, unsigned int num_samples) {
    //array = (DINT *) malloc(sizeof(UDINT) * num_samples);

    double omega = 2.0*PI /(double)(num_samples);

    int i;
    for (i=0; i<num_samples; i++) {
      array[i] = (DINT) (radius * cos(omega * (double)i));
      //printf("%d->%d ",i,array[i]);
    }
    return 0;
}

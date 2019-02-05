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
      if ((i < 15)||(i > num_samples-15)) printf("%d->%d ",i,array[i]);
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

int trifill1(DINT *array, DINT origin, double height, unsigned int period, unsigned int num_samples) {
    /* Max num_samples is 65535 */
    //array = (DINT *) malloc(sizeof(UDINT) * num_samples);

    double slope = ((double) (height*2)) / (double)period;

    int i;
    for (i=0; i<period/2; i++) {
      array[i] = origin + (DINT) (slope * (double)i);
      if ((i < 15)||(i > num_samples-15)) printf("%d->%d ",i,array[i]);
    }
    for (i=period/2; i<period; i++) {
      array[i] = origin + (DINT) (slope * (double)(period-i));
      //if (i < 15) printf("%d->%d ",i,array[i]);
    }


    printf("Trifill: %d\n",num_samples);
    return 0;
}
int trifill(DINT *array, DINT origin, double height, unsigned int num_samples) {
    trifill1(array, origin, height, num_samples, num_samples);
    return 0;
}

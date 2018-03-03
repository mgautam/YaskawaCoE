#include <math.h>
#include <ycoe_math.h>

#define PI 3.14159265

int sinfill(UDINT *array, double radius, unsigned int num_samples) {
    /* Max num_samples is 65535 */
    array = (UDINT *) malloc(sizeof(UDINT) * num_samples);

    double omega = 2.0*PI /(double)(num_samples);

    for (int i=0; i<num_samples; i++)
      array[i] = (UDINT) (radius * sin(omega * (double)i));

    return 0;
}

int cosfill(UDINT *array, double radius, unsigned int num_samples) {
    array = (UDINT *) malloc(sizeof(UDINT) * num_samples);

    double omega = 2.0*PI /(double)(num_samples);

    for (int i=0; i<num_samples; i++)
      array[i] = (UDINT) (radius * cos(omega * (double)i));

    return 0;
}

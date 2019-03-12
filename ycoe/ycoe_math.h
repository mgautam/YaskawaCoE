#ifndef _YCOE_MATH_H
#define _YCOE_MATH_H

#include <stdlib.h>
#include "ycoetype.h"


int sinfill(DINT *array, DINT origin, double radius, unsigned int num_samples);
int sinfill1(DINT *array, DINT origin, double radius, unsigned int period, unsigned int num_samples);
int cosfill(DINT *array, DINT origin, double radius, unsigned int num_samples);

int trifill1(DINT *array, DINT origin, double height, unsigned int period, unsigned int num_samples);
int trifill(DINT *array, DINT origin, double height, unsigned int num_samples);


int stairfill1(DINT *array, DINT origin, double height, double vel_divider, unsigned int num_stairs, unsigned int num_samples);
int stairfill(DINT *array, DINT origin, double height, double vel_divider, unsigned int num_stairs, unsigned int num_samples);

unsigned int vapfill (DINT *array, double gr, double acceleration, double velocity, DINT distance);
unsigned int rpsfill (DINT *array, double gr, double acceleration, double velocity, double distance);
#endif

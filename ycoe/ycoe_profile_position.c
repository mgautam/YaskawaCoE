#include <stdio.h>
#include "ethercat.h"
#include "ycoetype.h"
#include "ycoe_profile_position.h"

int ycoe_get_profile_position_parameters (void) {
    UDINT udintbuff;
    int udintsize = UDINT_SIZE;

    ec_SDOread(1,0x6081,0,0,&udintsize,&udintbuff,EC_TIMEOUTRXM);
    printf("Profile Velocity: %x [vel. units](incs/sec)\r\n",udintbuff);
    ec_SDOread(1,0x607F,0,0,&udintsize,&udintbuff,EC_TIMEOUTRXM);
    printf("Max Profile Velocity: %d [vel. units](incs/sec)\r\n",udintbuff);

    ec_SDOread(1,0x6083,0,0,&udintsize,&udintbuff,EC_TIMEOUTRXM);
    printf("Profile Acceleration: %d [acc. units](incs/sec2)\r\n",udintbuff);
    ec_SDOread(1,0x6084,0,0,&udintsize,&udintbuff,EC_TIMEOUTRXM);
    printf("Profile Deceleration: %d [acc. units](incs/sec2)\r\n",udintbuff);
    ec_SDOread(1,0x6085,0,0,&udintsize,&udintbuff,EC_TIMEOUTRXM);
    printf("Quick Stop Deceleration: %d [acc. units](incs/sec2)\r\n",udintbuff);

    return 0;
}

int ycoe_set_profile_velocity (UDINT profile_velocity) {
    ec_SDOwrite(1,0x6081,0,0,UDINT_SIZE,&profile_velocity,EC_TIMEOUTRXM);
    return 0;
}
int ycoe_set_profile_acceleration (UDINT profile_acceleration) {
    ec_SDOwrite(1,0x6083,0,0,UDINT_SIZE,&profile_acceleration,EC_TIMEOUTRXM);
    return 0;
}
int ycoe_set_profile_deceleration (UDINT profile_deceleration) {
    ec_SDOwrite(1,0x6084,0,0,UDINT_SIZE,&profile_deceleration,EC_TIMEOUTRXM);
    return 0;
}
int ycoe_set_quick_stop_deceleration (UDINT quick_stop_deceleration) {
    ec_SDOwrite(1,0x6085,0,0,UDINT_SIZE,&quick_stop_deceleration,EC_TIMEOUTRXM);
    return 0;
}

int ycoe_set_profile_position_parameters (UDINT profile_velocity, UDINT profile_acceleration, UDINT profile_deceleration, UDINT quick_stop_deceleration) {
    ycoe_set_profile_velocity (profile_velocity);
    ycoe_set_profile_acceleration (profile_acceleration);
    ycoe_set_profile_deceleration (profile_deceleration);
    ycoe_set_quick_stop_deceleration (quick_stop_deceleration);

    return 0;
}

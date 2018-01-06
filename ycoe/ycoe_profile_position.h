#ifndef _YCOE_PROFILE_POSITION_H
#define _YCOE_PROFILE_POSITION_H

int ycoe_get_profile_position_parameters(void);

int ycoe_set_profile_velocity (UDINT profile_velocity);
int ycoe_set_profile_acceleration (UDINT profile_acceleration);
int ycoe_set_profile_deceleration (UDINT profile_deceleration);
int ycoe_set_quick_stop_deceleration (UDINT quick_stop_deceleration);
int ycoe_set_profile_position_parameters (UDINT profile_velocity, UDINT profile_acceleration, UDINT profile_deceleration, UDINT quick_stop_deceleration);

#endif

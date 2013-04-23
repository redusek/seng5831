/*
 * timers.h
 *
 * Created: 4/17/2013 8:06:10 PM
 *  Author: bobd
 */ 


#ifndef TIMERS_H_
#define TIMERS_H_

#include <inttypes.h> //gives us uintX_t

#define INTERPOLATOR_TIMER_RESOLUTION 10
#define CONTROLLER_TIMER_RESOLUTION 1
#define NORMALIZER 1000
#define DEGREES_PER_COUNT 5625

void init_timers();
int abs(int);

#endif /* TIMERS_H_ */
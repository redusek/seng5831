/*
 * motors.c
 *
 * Created: 4/17/2013 9:09:29 PM
 *  Author: bobd
 */ 

#include <pololu/orangutan.h>
#include "motors.h"

void init_motor() 
{
	// Set motor power pin to output
	DDRD |= ( 1 << DDD6 );
	// set the motor direction control to output
	DDRC |= ( 1 << DDC6 );

	// set motor direction
	PORTC |= ( 1 << PORTC6 );

	// shut the motor off to begin with
	OCR2B = MIN_OCR2B - 1;	
}

/*
 * timers.c
 *
 * Created: 4/17/2013 8:52:39 PM
 *  Author: bobd
 */ 

#include <pololu/orangutan.h>
#include <inttypes.h>

#include "timers.h"
#include "motors.h"
#include "menu.h"

extern volatile g_reference_count;
extern volatile g_reference_count_full;
extern volatile g_reference_direction;
extern volatile g_reference_degrees_full;
extern volatile g_controller_ticks;
extern volatile g_int32_terpolator_ticks;
extern volatile g_Kp;
extern volatile g_Kd;
extern volatile g_count_step;
	
void init_timers () 
{
	init_pwm_timer();
	init_controller_timer();
	init_int32_terpolator_timer();
}

void init_pwm_timer()
{	
	// Using Fast PWM mode with 0xFF for TOP. This is mode 3, thus WGM2/210 =  011.
	TCCR2B &= ~( 1 << WGM22 );  // turn on WGM12
	TCCR2A |= ( 1 << WGM21 );  // turn on WGM11
	TCCR2A |= ( 1 << WGM20 );  // turn on WGM10

	// Normal FastPWM mode (non-inverted)
	TCCR2A &= ~( 1 << COM2A1 );
	TCCR2A &= ~( 1 << COM2A0 );
	TCCR2A |= ( 1 << COM2B1 );
	TCCR2A &= ~( 1 << COM2B0 );

	// Using pre-scaler 8. This is CS1/2/1/0 = 010
	TCCR2B &= ~( 1 << CS22 );
	TCCR2B |= ( 1 << CS21 );
	TCCR2B &= ~( 1 << CS20 );

	//Enable output compare match timer2 for OCR2B
	TCNT2=0; // we want to start off with a zero counter
	// TIMSK2 = ( 1 << OCIE2B ); // enable int32_terrupt on TCNT1=OCR1B	
}

// timer 1 has higher priority than timers 2 and 3, so it goes to the controller
void  init_controller_timer()
{
	// Using CTC mode with OCR3A for TOP. This is mode 4, thus WGM3/3210 = 0100.
	TCCR1B &= ~( 1 << WGM13 );  // turn off WGM13
	TCCR1B |= ( 1 << WGM12 );   // turn on WGM12
	TCCR1A &= ~( 1 << WGM11 );  // turn off WGM11
	TCCR1A &= ~( 1 << WGM10 );  // turn off WGM10
	
	// Using pre-scaler 8. This is CS3/2/1/0 = 010
	TCCR1B &= ~( 1 << CS12 );
	TCCR1B |=  ( 1 << CS11 );
	TCCR1B &= ~( 1 << CS10 );

	// We want this thing firing 1000 times per second
	// int32_terrupt Frequency: 1000 = 20000000 / (prescaler*OCR1A) -or- (prescaler*OCR1A) = 20000
	// prescaler = 8; OCR1A=2500
	OCR1A = 2500;
	
	TCNT1=0; // we want to start off with a zero counter
	TIMSK1 = ( 1<<OCIE1A );  // enable the int32_terrupt 
}


void init_int32_terpolator_timer()
{
	// Using CTC mode with OCR3A for TOP. This is mode 4, thus WGM3/3210 = 0100.
	TCCR3B &= ~( 1 << WGM33 );  // turn off WGM33
	TCCR3B |= ( 1 << WGM32 );  // turn on WGM32
	TCCR3A &= ~( 1 << WGM31 );  // turn off WGM31
	TCCR3A &= ~( 1 << WGM30 );  // turn off WGM30
	
	
	// Using pre-scaler 64. This is CS3/2/1/0 = 011
	TCCR3B &= ~( 1 << CS32 );
	TCCR3B |=  ( 1 << CS31 );
	TCCR3B |=  ( 1 << CS30 );

	// We want this thing firing 100 times per second
	// int32_terrupt Frequency: 100 = 20000000 / (prescaler*OCR3A) -or- (prescaler*OCR3A) = 200000
	// prescaler = 64; OCR3A=3125
	OCR3A = 3125;
	
	TCNT3=0; // we want to start off with a zero counter
	TIMSK3 = ( 1<<OCIE3A );  // enable the int32_terrupt
}

// int32_tERRUPT HANDLER for PD controller
ISR( TIMER1_COMPA_vect )
{
	++g_controller_ticks;
		
	// int32_t ( motor_speed / 16 ) = approximate encoder counts per second
	static int velocity_table[16] = { 0, 7, 17, 27, 37, 47, 56, 65, 74, 83, 89, 97, 102, 110, 117, 121 };
	static int32_t prev_torque = 0;
	static int32_t printlen;
	static char printbuffer[128];	

	int32_t torque;
	int32_t measured_count = encoders_get_counts_m2();
	
	// if the motor is in position, set velocity to zero
	if( measured_count == g_reference_count_full )
	{
		if( prev_torque != 0 )
		{
			printlen = sprintf( printbuffer, "DESTINATIONDESTINATIONDESTINATIONDESTINATIONDESTINATION\r\n" );
			print_usb( printbuffer, printlen );
		}		
		
		torque = 0;
		OCR2B = 0;
		prev_torque = 0;
	}
	else 
	{		
		// determine velocity based on error and velocity
		int32_t error = abs( g_reference_count - measured_count );
		int velocity = velocity_table[ prev_torque / 16 ];	// velocity is strictly a function of torque here
		torque = ( g_Kp * error ) - ( g_Kd * velocity );
		
 		if( g_controller_ticks % 250 ) 
		{
			printlen = sprintf( printbuffer, "CONTROL:e%ld:v%d:t%ld\r\n", error, velocity, torque );
			print_usb( printbuffer, printlen );	
			printlen = sprintf( printbuffer, "CONTROL:m%ld:r%ld:rf%ld\r\n", measured_count, g_reference_count, g_reference_count_full );
			print_usb( printbuffer, printlen );
			
		}
		
		if( torque > 255 ) 
		{
			torque = 255; 
		}
					
		// make sure the motor is spinning in the right direction (may depend on motor connection)
		if( g_reference_count > measured_count )
		{
			// counter clockwise (positive degrees)
			PORTC |= ( 1 << PORTC6 );			
		}
		else
		{
			// clockwise (negative degrees)
			PORTC &= ~( 1 << PORTC6 );
		}
		
		OCR2B = torque; // set speed
		prev_torque = torque;
	}	
	
	if( g_controller_ticks % 1000 ) 
	{
		g_controller_ticks = 0;  // reset it to prevent overflow
	}
}

// int32_tERRUPT HANDLER for int32_terpolator
ISR( TIMER3_COMPA_vect )
{
	++g_int32_terpolator_ticks;

	int32_t printlen;
	char printbuffer[128];
	
	int32_t measured = encoders_get_counts_m2();
	int32_t difference = abs( g_reference_count_full - measured );
	
	if( g_int32_terpolator_ticks % 25 == 0 )
	{
		printlen = sprintf( printbuffer, "int32_t1:m%ld:d%ld:s%ld\r\n", measured, difference, g_count_step );
		print_usb( printbuffer, printlen );
 	}
	
	if( difference <= g_count_step )
	{
		g_reference_count = g_reference_count_full; // hold position
		
		// move on to next step if difference == 0?
	}
	else if( g_reference_count_full < measured )
	{
		g_reference_count = measured - g_count_step;
	} 
	else
	{
		g_reference_count = measured + g_count_step;
	}	
	
 	if( g_int32_terpolator_ticks % 25 == 0 )
 	{
		printlen = sprintf( printbuffer, "INT2:grc=%ld\r\n", g_reference_count );
		print_usb( printbuffer, printlen );
	 }
	 
	 if( g_int32_terpolator_ticks % 100 == 0 )
	 {		
 		g_int32_terpolator_ticks = 0;
 	}
}

// http://stackoverflow.com/questions/9772348/get-absolute-value-without-using-abs-function-nor-if-statement
int32_t abs (int32_t n)
{
	if( n < 0 ) 
		return 0 - n;
	return n;
}

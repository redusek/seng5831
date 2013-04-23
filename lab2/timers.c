/*
 * timers.c
 *
 * Created: 4/17/2013 8:52:39 PM
 *  Author: bobd
 */ 

#include <pololu/orangutan.h>
#include "timers.h"
#include "motors.h"
#include "menu.h"

extern volatile g_reference_count;
extern volatile g_reference_count_full;
extern volatile g_reference_direction;
extern volatile g_reference_degrees_full;
extern volatile g_controller_ticks;
extern volatile g_interpolator_ticks;
extern volatile g_Kp;
extern volatile g_Kd;
extern volatile g_count_step;
	
void init_timers () 
{
	init_pwm_timer();
	init_controller_timer();
	init_interpolator_timer();
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
	// TIMSK2 = ( 1 << OCIE2B ); // enable interrupt on TCNT1=OCR1B	
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
	// Interrupt Frequency: 1000 = 20000000 / (prescaler*OCR1A) -or- (prescaler*OCR1A) = 20000
	// prescaler = 8; OCR1A=2500
	OCR1A = 2500;
	
	TCNT1=0; // we want to start off with a zero counter
	TIMSK1 = ( 1<<OCIE1A );  // enable the interrupt 
}


void init_interpolator_timer()
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
	// Interrupt Frequency: 100 = 20000000 / (prescaler*OCR3A) -or- (prescaler*OCR3A) = 200000
	// prescaler = 64; OCR3A=3125
	OCR3A = 3125;
	
	TCNT3=0; // we want to start off with a zero counter
	TIMSK3 = ( 1<<OCIE3A );  // enable the interrupt
}

// INTERRUPT HANDLER for PD controller
ISR( TIMER1_COMPA_vect )
{
	++g_controller_ticks;
	if( g_controller_ticks % 100 != 0 )
		return;
		
	g_controller_ticks = 0;
	// int ( motor_speed / 16 ) = approximate encoder counts per second
	static int velocity_table[16] = { 0, 7, 17, 27, 37, 47, 56, 65, 74, 83, 89, 97, 102, 110, 117, 121 };
	static int prev_torque = 0;
	static int printlen;
	static char printbuffer[128];	


	int torque;
	int measured_count = encoders_get_counts_m2();
	
	// if the motor is in position, set velocity to zero
	if( measured_count == g_reference_count )
	{
		if( prev_torque != 0 )
		{
			printlen = sprintf( printbuffer, "CONTROL:Z:%d/%d/%d\r\n", measured_count, g_reference_count, g_reference_count_full );
			print_usb( printbuffer, printlen );
		}		
		
		torque = 0;
		OCR2B = 0;
		prev_torque = 0;
	}
	else 
	{		
		// determine velocity based on error and velocity
		int error = abs( g_reference_count - measured_count );
		int velocity = velocity_table[ prev_torque / 16 ];	// velocity is strictly a function of torque here
		torque = ( g_Kp * error ) - ( g_Kd * velocity );
		
		if( g_controller_ticks % 1 ) 
		{
			printlen = sprintf( printbuffer, "CONTROL:e%d:v%d:t%d\r\n", error, velocity, torque );
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
}

// INTERRUPT HANDLER for interpolator
ISR( TIMER3_COMPA_vect )
{
	++g_interpolator_ticks;

	int printlen;
	char printbuffer[128];
	
	int measured = encoders_get_counts_m2();
	int difference = abs( g_reference_count_full - measured );
	
	if( g_interpolator_ticks % 10 == 0 )
	{
		printlen = sprintf( printbuffer, "INT1:m%d:d%d:s%d\r\n", measured, difference, g_count_step );
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
	
	if( g_interpolator_ticks % 10 == 0 )
	{
		printlen = sprintf( printbuffer, "INT2:grc=%d\r\n", g_reference_count );
		print_usb( printbuffer, printlen );
		g_interpolator_ticks = 0;
	}
}

// http://stackoverflow.com/questions/9772348/get-absolute-value-without-using-abs-function-nor-if-statement
int abs (int n)
{
	if( n < 0 ) 
		return 0 - n;
	return n;
}

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

extern volatile uint32_t g_reference_count;
extern volatile uint32_t g_reference_count_full;
extern volatile uint32_t g_reference_direction;
extern volatile uint32_t g_reference_degrees_full;
extern volatile uint32_t g_controller_ticks;
extern volatile uint32_t g_interpolator_ticks;
extern volatile uint32_t g_Kp;
extern volatile uint32_t g_Kd;
extern volatile uint32_t g_count_step;
	
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
	// int32_terrupt Frequency: 100 = 20000000 / (prescaler*OCR3A) -or- (prescaler*OCR3A) = 200000
	// prescaler = 64; OCR3A=3125
	OCR3A = 3125;
	
	TCNT3=0; // we want to start off with a zero counter
	TIMSK3 = ( 1<<OCIE3A );  // enable the int32_terrupt
}

// INTERRUPT HANDLER for PD controller
ISR( TIMER1_COMPA_vect )
{
	++g_controller_ticks;
	
	static int32_t printlen;
	static char printbuffer[128];
	
	// giving up on the velocity table experiment	
	//  motor_speed / 16 = approximate encoder counts per second
	// static int velocity_table[16] = { 0, 7, 17, 27, 37, 47, 56, 65, 74, 83, 89, 97, 102, 110, 117, 121 };
	
	static int32_t prev_torque = 0;
	static int velocity = 0;
	int32_t torque;
	int32_t error;
	int32_t measured_count = encoders_get_counts_m2();
	static int32_t prev_measured_count = 0;
	static int32_t encoder_counts;
	
	if( prev_measured_count != measured_count )
	{
		encoder_counts += abs( measured_count - prev_measured_count );
// 		printlen = sprintf( printbuffer, "M%ldPM%ldT%ldPT%ld\r\n", measured_count, prev_measured_count, torque, prev_torque );
// 		print_usb( printbuffer, printlen );
	}
	
	if( g_controller_ticks >= 100 )
	{
		velocity = encoder_counts;
		encoder_counts = 0;
		g_controller_ticks = 0;
	}
	
	// if the motor is in position, set velocity to zero
	if( measured_count == g_reference_count_full )
	{		
		error = 0;
		velocity = 0;
		// encoder_counts = 0;
		// g_controller_ticks = 0;
		OCR2B = 0;
		
		if( prev_torque != 0 )
		{
			printlen = sprintf( printbuffer, "DESTINATION\r\n" );
			print_usb( printbuffer, printlen );
		}
		
		torque = 0;
	}
	else 
	{		
		// determine velocity based on error and velocity
		error = abs( g_reference_count - measured_count );
		// velocity = velocity_table[ prev_torque / 16 ];	// velocity is strictly a function of torque here
		torque = ( g_Kp * error ) - ( g_Kd * velocity );
		
		if( torque > 255 ) 
		{
			torque = 255; 
		}
		else if( torque < 0 )
		{
			torque = 0;
		}
		
// 		if( prev_measured_count	!= measured_count )
// 		{
// 			printlen = sprintf( printbuffer, "CTRL e%ld:v%d:t%ld:pt%ld:m%ld:pm%ld\r\n", error, velocity, torque, prev_torque, measured_count, prev_measured_count );
// 			print_usb( printbuffer, printlen );
// 		}			
					
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
	}	
	
	prev_torque = torque;

	prev_measured_count = measured_count;
}

// INTERRUPT HANDLER for interpolator
ISR( TIMER3_COMPA_vect )
{
	int32_t printlen;
	char printbuffer[128];
	static int32_t prev_measured = 0;
	
	int32_t measured = encoders_get_counts_m2();
	int32_t difference = abs( g_reference_count_full - measured );
	
// 	if( prev_measured != measured )
// 	{
// 		printlen = sprintf( printbuffer, "REF m%ld:gf%ld\r\n", measured, g_reference_count_full );
// 		print_usb( printbuffer, printlen );
//  	}
	
	if( difference <= g_count_step )
	{
		g_reference_count = g_reference_count_full; // hold position
		
		// move on to next step if difference == 0?
	}
	else if( g_reference_count_full < measured )
	{
		g_reference_count = measured - g_count_step;
		
		if( prev_measured != measured )
		{
			printlen = sprintf( printbuffer, "OVER\r\n" );
			print_usb( printbuffer, printlen );
		}			
	} 
	else
	{
		g_reference_count = measured + g_count_step;
		if( prev_measured != measured )
		{
			printlen = sprintf( printbuffer, "UNDER\r\n" );
			print_usb( printbuffer, printlen );
		}
	}	
	
//  	if( prev_measured != measured )
//  	{
// 		printlen = sprintf( printbuffer, "GRC %ld\r\n", g_reference_count );
// 		print_usb( printbuffer, printlen );
// 	}
	 	 
	prev_measured = measured; 
}

// http://stackoverflow.com/questions/9772348/get-absolute-value-without-using-abs-function-nor-if-statement
int32_t abs (int32_t n)
{
	if( n < 0 ) 
		return 0 - n;
	return n;
}

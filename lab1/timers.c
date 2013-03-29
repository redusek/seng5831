#include "timer.h"
#include "LEDs.h"

#include <avr/interrupt.h>

// GLOBALS
extern uint32_t G_green_ticks;
extern uint32_t G_yellow_ticks;
extern uint32_t G_yellow_toggles;
extern uint32_t G_green_toggles;
extern uint32_t G_ms_ticks;

extern uint16_t G_red_period;
extern uint16_t G_green_period;
extern uint16_t G_yellow_period;

extern uint16_t G_release_red;

void init_timers() 
{
	// Used to print to serial comm window
	char tempBuffer[128];
	int length = 0;

	// -------------------------  RED --------------------------------------//
	// Software Clock Using Timer/Counter 0.
	// THE ISR for this is below.

	// SET appropriate bits in TCCR....

	// Using CTC mode with OCR0 for TOP. This is mode 4, thus WGM0/1/0 = .
	TCCR0A |= ( 1 << WGM01 );  // turn on WGM01
	TCCR0A &= ~( 1 << WGM00 );  // turn off WGM00
	TCCR0B &= ~( 1 << WGM02 );  // turn off WGM02
	
	// Using pre-scaler 256. This is CS0/2/1/0 = 100
	TCCR0B |= ( 1 << CS02 );
	TCCR0B &= ~( 1 << CS01 );
	TCCR0B &= ~( 1 << CS00 );
	
	// Software Clock Interrupt Frequency: 1000 = f_IO / (prescaler*OCR0)
	// Set OCR0 appropriately for TOP to generate desired frequency of 1KHz
	print_usb( "Initializing software clock to freq 1000Hz (period 1 ms)\r\n", 58);
	OCR0A = 78;  // we want 1000 ticks per second

	//Enable output compare match interrupt on timer 0A
	TCNT0=0; // we want to start off with a zero counter
	TIMSK0 = ( 1<<OCIE0A ); // enable interrupt on TCNT0=OCR0A

	// Initialize counter
	G_ms_ticks = 0;
	

	//--------------------------- YELLOW ----------------------------------//
	// Set-up of interrupt for toggling yellow LEDs. 
	// This task is "self-scheduled" in that it runs inside the ISR that is 
	// generated from a COMPARE MATCH of 
	//      Timer/Counter 3 to OCR3A.
	// Obviously, we could use a single timer to schedule everything, but we are experimenting here!
	// THE ISR for this is below

	// SET appropriate bits in TCCR ...

	// Using CTC mode with OCR3A for TOP. This is mode 4, thus WGM3/3210 = 0100.
	TCCR3B &= ~( 1 << WGM33 );  // turn off WGM33
	TCCR3B |= ( 1 << WGM32 );  // turn on WGM32
	TCCR3A &= ~( 1 << WGM31 );  // turn off WGM31	
	TCCR3A &= ~( 1 << WGM30 );  // turn off WGM30
	
	// Using pre-scaler 64. This is CS3/2/1/0 = 011
	TCCR3B &= ~( 1 << CS32 );
	TCCR3B |= ( 1 << CS31 );
	TCCR3B |= ( 1 << CS30 );

	// Interrupt Frequency: 10 = f_IO / (prescaler*OCR3A)
	// Set OCR3A appropriately for TOP to generate desired frequency using Y_TIMER_RESOLUTION (100 ms).
	// NOTE: This is not the toggle frequency, rather a tick frequency used to time toggles.
	OCR3A = 31250;
	length = sprintf( tempBuffer, "Initializing yellow clock to freq %dHz (period %d ms)\r\n", (int)(10), Y_TIMER_RESOLUTION );	
	print_usb( tempBuffer, length );
	
	//Enable output compare match interrupt on timer 3A
	TCNT3=0; // we want to start off with a zero counter
	TIMSK3 = ( 1<<OCIE3A ); // enable interrupt on TCNT0=OCR0A

	G_yellow_ticks = 0;
	
	//--------------------------- GREEN ----------------------------------//
	// Set-up of interrupt for toggling green LED. 
	// This "task" is implemented in hardware, because the OC1A pin will be toggled when 
	// a COMPARE MATCH is made of 
	//      Timer/Counter 1 to OCR1A.
	// We will keep track of the number of matches (thus toggles) inside the ISR (in the LEDs.c file)
	// Limits are being placed on the frequency because the frequency of the clock
	// used to toggle the LED is limited.

	// Using Fast PWM mode with OCR1A for TOP. This is mode 15, thus WGM3/3210 =  1111.
	TCCR1B |= ( 1 << WGM13 );  // turn on WGM13.
	TCCR1B |= ( 1 << WGM12 );  // turn on WGM12
	TCCR1A |= ( 1 << WGM11 );  // turn on WGM11
	TCCR1A |= ( 1 << WGM10 );  // turn on WGM10

	// Toggle OC1A on a compare match. Thus COM1A_10 = 01
	TCCR1A &= ~( 1 << COM1A1 );
	TCCR1A |= ( 1 << COM1A0 );
	
	// Using pre-scaler 1024. This is CS1/2/1/0 = 101
	TCCR1B |= ( 1 << CS12 );
	TCCR1B &= ~( 1 << CS11 );
	TCCR1B |= ( 1 << CS10 );
	
	OCR1A = 1953;
	
	// using 1024 for the pre-scaler, we have a scaled frequency of 19531.25 Hz
	// to make that a nice usable 10Hz (100 ms), we would set top to 1953.125 (or 1953)
	// if someone wants it to toggle less frequently, say 1000 ms, we would multiply 1953 * (1000/100) = 19530
	// Interrupt Frequency: OCR1A = ( 1953 * (G_green_period / G_TIMER_RESOLUTION ) )
	// Set OCR1A appropriately for TOP to generate desired frequency.
	// NOTE: This IS the toggle frequency.
	length = sprintf( tempBuffer, "green period %d\r\n", G_green_period );
	print_usb( tempBuffer, length );
	OCR1A = (uint16_t) ( 1953 * ( G_green_period / G_TIMER_RESOLUTION ) );
	length = sprintf( tempBuffer, "Set OCR1A to %d\r\n", OCR1A );
	print_usb( tempBuffer, length );
	length = sprintf( tempBuffer, "Initializing green clock to freq %dHz (period %d ms)\r\n", (int)(1), G_green_period );	
	print_usb( tempBuffer, length );
	
	// A match to this will toggle the green LED.
	// Regardless of its value (provided it is less than OCR1A), it will match at the frequency of timer 1.
	OCR1B = 1;

	//Enable output compare match interrupt on timer 1B
	TCNT1=0; // we want to start off with a zero counter
	TIMSK1 = ( 1 << OCIE1B ); // enable interrupt on TCNT1=OCR1B
}

//INTERRUPT HANDLERS
ISR( TIMER0_COMPA_vect )
{
	// This is the Interrupt Service Routine for Timer0
	// Each time the TCNT count is equal to the OCR0 register, this interrupt is "fired".

	// if time to toggle the RED LED, set flag to release
	if ( ( ++G_ms_ticks % G_red_period ) == 0 )
		G_release_red = 1;
}

// INTERRUPT Names are defined in iom1284p.h

// INTERRUPT HANDLER for yellow LED
ISR( TIMER3_COMPA_vect ) 
{
	// This the Interrupt Service Routine for Toggling the yellow LED.
	// Each time the TCNT count is equal to the OCRxx register, this interrupt is enabled.
	// At creation of this file, it was initialized to interrupt every 100ms (10Hz).
	//
	
	// Increment ticks. If it is time, toggle YELLOW and increment toggle counter.
	// period is in ms, the ISR is fired once every 100 ms... 
	if( ( ++G_yellow_ticks % ( G_yellow_period/Y_TIMER_RESOLUTION ) ) == 0 )
	{
		LED_TOGGLE(YELLOW);
		++G_yellow_toggles;
		
		lcd_goto_xy( 0, 0 );
		printf( "y%lu: %lu (%lu)", G_yellow_toggles, G_yellow_ticks, G_green_toggles );
		
		// char tempBuffer[32];
		// int length = 0;
		// length = sprintf( tempBuffer, "y%lu: %lu\r\n", G_yellow_toggles, G_yellow_ticks );
		// print_usb( tempBuffer, length );
	}

}

// INTERRUPT HANDLER for green LED
ISR( TIMER1_COMPB_vect ) 
{
	// This the Interrupt Service Routine for tracking green toggles. The toggling is done in hardware.
	// Each time the TCNT count is equal to the OCRxx register, this interrupt is enabled.
	// This interrupts at the user-specified frequency for the green LED.
	
	++G_green_toggles;
}


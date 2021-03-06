/* lab2 - an application for the Pololu Orangutan SVP
 *
 * This application uses the Pololu AVR C/C++ Library.  For help, see:
 * -User's guide: http://www.pololu.com/docs/0J20
 * -Command reference: http://www.pololu.com/docs/0J18
 *
 * Created: 4/6/2013 4:07:29 PM
 *  Author: bobd
 */

#include <pololu/orangutan.h>
#include <inttypes.h>

volatile int32_t g_reference_degrees_full = 0;
volatile int32_t g_reference_count_full = 0;
volatile int32_t g_reference_count = 0;
volatile int32_t g_controller_ticks = 0;
volatile int32_t g_interpolator_ticks = 0;
// this konstant gave a nice slow down for the velocity table
// however, it's too large for smaller velocity numbers
// volatile int32_t g_Kp = 24;
volatile int32_t g_Kp = 16;
volatile int32_t g_Kd = 1;
volatile int32_t g_count_step = 16;  // max error in Pd controller
// g_count_step * DEGREES_PER_COUNT / NORMALIZER == max degrees

#include "timers.h"
#include "motors.h"
#include "menu.h"

int32_t main()
{
	char printbuffer[128];
	int32_t printlen = 0;
	
	print_usb( "BOOTBOOTBOOTBOOTBOOTBOOTBOOTBOOTBOOT\r\n", 38 );
	print_usb( "BOOTBOOTBOOTBOOTBOOTBOOTBOOTBOOTBOOT\r\n", 38 );
	print_usb( "BOOTBOOTBOOTBOOTBOOTBOOTBOOTBOOTBOOT\r\n", 38 );
	print_usb( "BOOTBOOTBOOTBOOTBOOTBOOTBOOTBOOTBOOT\r\n", 38 );
	print_usb( "BOOTBOOTBOOTBOOTBOOTBOOTBOOTBOOTBOOT\r\n", 38 );	
	
	lcd_init_printf();	
	
	g_reference_degrees_full = 720;
	g_reference_count_full = ( g_reference_degrees_full * NORMALIZER ) / DEGREES_PER_COUNT;
	
	int32_t count = 0; 
	int32_t errors = 0;
	int32_t modifier = -1;
	
	int32_t measured_count = 0;
	int32_t measured_degrees = 0;
	int32_t relative_degrees = 0;
	
	encoders_init( IO_A4, IO_A5, IO_A6, IO_A7 );
	encoders_get_counts_and_reset_m2();
	
	printlen = sprintf( printbuffer, "1:%ld/%ld/%ld\r\n", measured_count, g_reference_count, g_reference_count_full );
	print_usb( printbuffer, printlen );
	
	init_timers();
	init_motor();
	
	printlen = sprintf( printbuffer, "2:%ld/%ld/%ld\r\n", measured_count, g_reference_count, g_reference_count_full );
	print_usb( printbuffer, printlen );
	
	while(1)
	{
		measured_count = encoders_get_counts_m2();
		measured_degrees = ( measured_count * DEGREES_PER_COUNT ) / NORMALIZER;	
		
		lcd_goto_xy(0, 0);
		printf( printbuffer, "C%ld/%ld/%ld", measured_count, g_reference_count, g_reference_count_full );
		printlen = sprintf( printbuffer, "C%ld/%ld/%ld\r\n", measured_count, g_reference_count, g_reference_count_full );
		// print( printbuffer );
		print_usb( printbuffer, printlen );
		
		lcd_goto_xy(0, 1);
		printf( "D%ld/%ld/%ld", measured_degrees, ( ( g_reference_count * DEGREES_PER_COUNT ) / NORMALIZER ), g_reference_degrees_full );	
		printlen = sprintf( printbuffer, "D%ld/%ld/%ld\r\n", measured_degrees, ( ( g_reference_count * DEGREES_PER_COUNT ) / NORMALIZER ), g_reference_degrees_full );
		// print( printbuffer );
		print_usb( printbuffer, printlen );
		
		// print_long( OCR2B );
		
		delay_ms( 1000 );	
	}
}



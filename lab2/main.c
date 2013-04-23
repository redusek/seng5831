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

volatile uint32_t g_reference_degrees_full = 0;
volatile uint32_t g_reference_count_full = 0;
volatile uint32_t g_reference_count = 0;
volatile uint32_t g_controller_ticks = 0;
volatile uint32_t g_interpolator_ticks = 0;
volatile int g_Kp = 24;
volatile int g_Kd = 1;
volatile int g_count_step = 16;  // max error in Pd controller
// g_count_step * DEGREES_PER_COUNT / NORMALIZER == max degrees

#include "timers.h"
#include "motors.h"
#include "menu.h"

int main()
{
	char printbuffer[64];
	int printlen = 0;
	
	print_usb( "BOOTBOOTBOOTBOOTBOOTBOOTBOOTBOOTBOOT\r\n", 38 );
	print_usb( "BOOTBOOTBOOTBOOTBOOTBOOTBOOTBOOTBOOT\r\n", 38 );
	print_usb( "BOOTBOOTBOOTBOOTBOOTBOOTBOOTBOOTBOOT\r\n", 38 );
	print_usb( "BOOTBOOTBOOTBOOTBOOTBOOTBOOTBOOTBOOT\r\n", 38 );
	print_usb( "BOOTBOOTBOOTBOOTBOOTBOOTBOOTBOOTBOOT\r\n", 38 );	
	
	lcd_init_printf();	
	
	g_reference_degrees_full = 720;
	g_reference_count_full = ( g_reference_degrees_full * NORMALIZER ) / DEGREES_PER_COUNT;
	
	int count = 0; 
	int errors = 0;
	int modifier = -1;
	
	uint32_t measured_count = 0;
	uint32_t measured_degrees = 0;
	uint32_t relative_degrees = 0;
	
	encoders_init( IO_A4, IO_A5, IO_A6, IO_A7 );
	encoders_get_counts_and_reset_m2();
	
	printlen = sprintf( printbuffer, "1:%d/%"PRIu32"/%"PRIu32"\r\n", measured_count, g_reference_count, g_reference_count_full );
	print_usb( printbuffer, printlen );
	
	init_timers();
	init_motor();
	
	printlen = sprintf( printbuffer, "2:%d/%"PRIu32"/%"PRIu32"\r\n", measured_count, g_reference_count, g_reference_count_full );
	print_usb( printbuffer, printlen );
	
	while(1)
	{
		lcd_goto_xy(0, 0);
		measured_count = encoders_get_counts_m2();
		measured_degrees = ( measured_count * DEGREES_PER_COUNT ) / NORMALIZER;	
		printlen = sprintf( printbuffer, "C%d/%"PRIu32"/%"PRIu32"\r\n", measured_count, g_reference_count, g_reference_count_full );
		// print( printbuffer );
		print_usb( printbuffer, printlen );
		
		lcd_goto_xy(0, 1);
		printlen = sprintf( printbuffer, "D%"PRIu32"/%"PRIu32"/%"PRIu32"\r\n", measured_degrees, ( ( g_reference_count * DEGREES_PER_COUNT ) / NORMALIZER ), g_reference_degrees_full );
		// print( printbuffer );
		print_usb( printbuffer, printlen );
		
		// print_long( OCR2B );
		
		delay_ms( 250 );	
	}
}



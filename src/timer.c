/**
 * source@: https://github.com/s-matyukevich/raspberry-pi-os/blob/master/src/lesson03/src/timer.c#L8
 */
// Modified by Sean Phillips
#include "utils.h"
// #include "printf.h"
#include "mini_uart.h"
#include "peripherals/timer.h"

const unsigned int interval = 200000;
unsigned int curVal = 0;

void timer_init ( void )
{
	// Get the current free running timer value
	curVal = get32(TIMER_CLO);
	// Add our interval.
	curVal += interval;
	// Place into one of compare registers.
	// This register will be used to help
	// generate the exception when TIMER_CLO
	// equals TIMER_C1.
	put32(TIMER_C1, curVal);
}

void handle_timer_irq( void ) 
{
	curVal += interval;
	// update compare register
	put32(TIMER_C1, curVal);
	// acknowledge interrupt has been received.
	put32(TIMER_CS, TIMER_CS_M1);
	//printf("Timer interrupt received\n\r");
	uart_send_string("Timer interrupt received\n\r");
}
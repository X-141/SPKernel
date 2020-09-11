#include "mini_uart.h"
#include "utils.h"
#include "delay.h"

#include "Apps/Terminal.h"

void kernel_main(void)
{
	uart_init();
	uart_send_string("Kernel Date: 09/09/20.\r\n");

	uart_send_string("Delaying for 5 second\r\n");
	wait_msec(5000000);
	uart_send_string("After 5 second delay\r\n");
	
	init_terminal();
}

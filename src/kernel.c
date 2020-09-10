#include "mini_uart.h"
#include "utils.h"

//#include "Apps/Terminal.h"

void kernel_main(void)
{
	uart_init();
	uart_send_string("Kernel Date: 09/09/20.\r\n");
	
	// while (1) {
	// 	init_terminal();
	// }
}

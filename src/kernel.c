#include "mini_uart.h"
#include "utils.h"

#include "Apps/Terminal.h"

void kernel_main(void)
{
	uart_init();
	uart_send_string("Hey! This is a testing for value!\r\n");
	
	while (1) {
		//uart_send(uart_recv());
		init_terminal();
	}
}

// void proc_main_1(unsigned int aProcValue) {
// 	delay(Pow(10, 5));
// 	char procString[32];
// 	UintToString(aProcValue, procString);
// 	uart_send_string("I'm Processor 1\r\n");
// 	while(1){
// 		// Loop Forever.
// 	}
// }

// void proc_main_2(unsigned int aProcValue) {
// 	delay(Pow(10, 7));
// 	char procString[32];
// 	UintToString(aProcValue, procString);
// 	uart_send_string("I'm Processor 2\r\n");
// 	while(1){
// 		// Loop Forever.
// 	}
// }

// void proc_main_3(unsigned int aProcValue) {
// 	delay(Pow(10, 8));
// 	char procString[32];
// 	UintToString(aProcValue, procString);
// 	uart_send_string("I'm Processor 3\r\n");
// 	while(1){
// 		// Loop Forever.
// 	}
// }
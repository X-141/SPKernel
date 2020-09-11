#include "mini_uart.h"
#include "utils.h"
#include "delay.h"

#include "sd.h"

//#include "Apps/Terminal.h"
extern unsigned char bss_end;

void kernel_main(void)
{
	uart_init();
	uart_send_string("Kernel Date: 09/11/20.\r\n");

	//uart_send_string("Delaying for 5 second\r\n");
	//wait_msec(5000000);
	//uart_send_string("After 5 second delay\r\n");
	
	    // initialize EMMC and detect SD card type
    if(sd_init()==SD_OK) {
        // read the master boot record after our bss segment
        if(sd_readblock(0,&bss_end,1)) {
            // dump it to serial console
            uart_dump(&bss_end);
        }
    }

	//init_terminal();
}

#include "mini_uart.h"
#include "utils.h"
#include "delay.h"

#include "sd.h"
#include "fat.h"

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
        if(fat_getpartition()) {
            // list root directory entries
            fat_listdirectory();
        } else {
            uart_send_string("FAT partition not found???\r\n");
        }
    }

	//init_terminal();
}

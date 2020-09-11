#include "mini_uart.h"
#include "utils.h"
#include "delay.h"

#include "../Drivers/sd.h"
#include "../Drivers/fat.h"

void kernel_main(void)
{
	uart_init();
	uart_send_string("Kernel Date: 09/11/20.\r\n");


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

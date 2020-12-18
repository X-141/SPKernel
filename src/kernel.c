#include "mini_uart.h"
#include "utils.h"
#include "delay.h"

#include "../Apps/Terminal.h"

#include "../Drivers/sd.h"
#include "../Drivers/fat.h"

void kernel_main(void)
{
    
	uart_init();
    uart_send_string("Initalized UART.");
    sd_init();
	uart_send_string("Kernel Date: 12/18/20.\r\n");

    /**
     * unsigned int cluster;
     * // initialize EMMC and detect SD card type
     * if(sd_init()==SD_OK) {
     *     // read the master boot record and find our partition
     *     if(fat_getpartition()) {
     *         // find out file in root directory entries
     *         cluster = fat_getcluster("LICENC~1BRO");
     * 
     *         // if(cluster == 0)
     *         //     cluster = fat_getcluster("KERNEL8 IMG");
     * 
     *         if(cluster) {
     *             // read into memory
     *             uart_dump(fat_readfile(cluster));
     *         }
     *     } else {
     *         uart_send_string("FAT partition not found???\r\n");
     *     }
     * }
    */

    if(sd_init() == SD_OK) {
        fat_getpartition();
    }
        
	init_terminal();
}

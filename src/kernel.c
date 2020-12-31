#include "mini_uart.h"
#include "utils.h"
#include "delay.h"

#include "timer.h"
#include "irq.h"
#include "fork.h"
#include "sched.h"

#include "../Apps/Terminal.h"

//#include "../Drivers/sd.h"
//#include "../Drivers/fat.h"

void process(char *array)
{
	while (1){
		for (int i = 0; i < 5; i++){
			uart_send(array[i]);
			wait_cycles(100000);
		}
	}
}

void kernel_main(void)
{
	uart_init();
    uart_send_string("Initalized UART.");
    //sd_init();
	uart_send_string("Kernel Date: 12/TEST/20.\r\n");
	irq_vector_init();
	timer_init();
	enable_interrupt_controller();
	enable_irq();

    // fork.h/c
	int res = copy_process((unsigned long)&init_terminal, (unsigned long)"12345");
	if (res != 0) {
		// printf("error while starting process 1");
		return;
	}
	// // fork.h/c
	// res = copy_process((unsigned long)&process, (unsigned long)"abcde");
	// if (res != 0) {
	// 	// printf("error while starting process 2");
	// 	return;
	// }

	while (1){
		schedule();
	}	

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

    //if(sd_init() == SD_OK) {
    //    fat_getpartition();
    //}
        
	// init_terminal();
}

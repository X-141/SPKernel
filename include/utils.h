#ifndef	_BOOT_H
#define	_BOOT_H

const static double BAUDRATE = 115200.0;
const static double SYSTEM_CLOCK_FREQ = (250.0 * 1000000.0); // 250 mhz
const static unsigned int BAUDRATE_REG = (unsigned int)((1.0 / ((BAUDRATE / SYSTEM_CLOCK_FREQ) * 8.0)) - 1.0);

extern void delay ( unsigned long);
extern void put32 ( unsigned long, unsigned int );
extern unsigned int get32 ( unsigned long );

#endif  /*_BOOT_H */

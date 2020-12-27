/**
 * source@: https://github.com/s-matyukevich/raspberry-pi-os/blob/master/src/lesson03/include/peripherals/timer.h
 */
// Modified by Sean Phillips

#ifndef	_P_TIMER_H
#define	_P_TIMER_H

#include "peripherals/base.h"

// Timer information can be found on page 172 of the BCM2837 manual.

// Control status register
#define TIMER_CS        (PBASE+0x00003000)

// System clock counter lower 32bits
// Read only register that returns lower 32-bits
// of free running counter.
#define TIMER_CLO       (PBASE+0x00003004)

// System clock counter higher 32bits
#define TIMER_CHI       (PBASE+0x00003008)

// System timer compare 0-3
// Holds some defined value. Whenever TIMER_CLO
// has a value matching the values stored in these
// four registers, the cooresponding bit in the
// TIMER_CS register is asserted.
#define TIMER_C0        (PBASE+0x0000300C)
#define TIMER_C1        (PBASE+0x00003010)
#define TIMER_C2        (PBASE+0x00003014)
#define TIMER_C3        (PBASE+0x00003018)

// One of the 4 bits of CS register
// these are used to inform that we received the
// interrupt.
#define TIMER_CS_M0	(1 << 0)
#define TIMER_CS_M1	(1 << 1)
#define TIMER_CS_M2	(1 << 2)
#define TIMER_CS_M3	(1 << 3)

#endif  /*_P_TIMER_H */
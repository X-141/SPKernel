#include "mm.h"

.section ".text.boot"

.globl _start

//https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2836/QA7_rev3.4.pdf
// Also sourced from armstub8.S
#define LOCAL_CONTROL		0x40000000
#define LOCAL_PRESCALER		0x40000008
#define OSC_FREQ			19200000

_start:
	mrs	x0, mpidr_el1		
	and	x0, x0,#0xFF		// Check processor id
	cbz	x0, proc_0_start
	WFE						// Disables other cores.
	b proc_hang

// Left off here: For some reason newer raspberry pi os has some firmware that breaks this code.
// We have downloaded an older version of raspbian.

proc_0_start:
	/* armstub8.S
	 * LOCAL_CONTROL:
	 * Bit 9 clear: Increment by 1 (vs. 2).
	 * Bit 8 clear: Timer source is 19.2MHz crystal (vs. APB).
	 */
	ldr x0, =LOCAL_CONTROL
	str wzr, [x0]
	/* LOCAL_PRESCALER; divide-by (0x80000000 / register_val) == 1 */
	mov w1, 0x80000000
	str w1, [x0, #(LOCAL_PRESCALER - LOCAL_CONTROL)]

	/* Set System clock frequency to 19.2MHz */
	//mov  x0, #0xf800
	//movk x0, #0x124, lsl 16
	//msr cntfrq_el0, x0
	ldr x0, = OSC_FREQ
	msr CNTFRQ_EL0, x0
 
	/* Set up CNTVOFF_EL2 */
	msr CNTVOFF_EL2, xzr

	adr	x0, bss_begin
	adr	x1, bss_end
	sub	x1, x1, x0
	bl 	memzero

	mov	sp, #LOW_MEMORY_0 
	bl	kernel_main

proc_hang: 
	b 	proc_hang

#include "mm.h"

.section ".text.boot"

.globl _start
_start:
	mrs	x0, mpidr_el1		
	and	x0, x0,#0xFF		// Check processor id
	cbz	x0, proc_0_start	
	b proc_hang

proc_0_start:
	adr	x0, bss_begin
	adr	x1, bss_end
	sub	x1, x1, x0
	bl 	memzero

	mov	sp, #LOW_MEMORY_0 
	bl	kernel_main
	b 	proc_hang		// should never come here

proc_hang: 
	b 	proc_hang

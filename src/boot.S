#include "mm.h"

.section ".text.boot"

.globl _start
_start:
	mrs	x0, mpidr_el1		
	and	x0, x0,#0xFF		// Check processor id
	cbz	x0, proc_0_start	
	//!! Uncomment the code below to execute 
	//!! multiple processors
	//cmp	x0, #0x1
	//beq proc_1_start
	//cmp x0, #0x2
	//beq proc_2_start
	//cmp x0, #0x3
	//beq proc_3_start
	b proc_hang

proc_0_start:
	adr	x0, bss_begin
	adr	x1, bss_end
	sub	x1, x1, x0
	bl 	memzero

	mov	sp, #LOW_MEMORY_0 
	bl	kernel_main
	b 	proc_hang		// should never come here

// proc_1_start:
// 	mrs x0, mpidr_el1
// 	and x0, x0, #0xFF
// 	mov sp, #LOW_MEMORY_1
// 	bl 	proc_main_1
// 	b 	proc_hang

// proc_2_start:
// 	mrs x0, mpidr_el1
// 	and x0, x0, #0xFF
// 	mov sp, #LOW_MEMORY_2
// 	bl 	proc_main_2
// 	b 	proc_hang

// proc_3_start:
// 	mrs x0, mpidr_el1
// 	and x0, x0, #0xFF
// 	mov sp, #LOW_MEMORY_3
// 	bl 	proc_main_3
// 	b 	proc_hang

proc_hang: 
	b 	proc_hang

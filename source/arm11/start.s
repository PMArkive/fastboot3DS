#include "mem_map.h"

.arm
.cpu mpcore
.fpu vfpv2

.global _start
.global _init
.global disableCaches

.type _start STT_FUNC
.type _init STT_FUNC
.type disableCaches STT_FUNC

.section .init



_start:
	cpsid aif                  @ Disable all interrupts

	ldr sp, =(A11_STUB_ENTRY-8)

	mov r0, #0
	mcr p15, 0, r0, c7, c14, 0 @ Clear and invalidate entire data cache
	mcr p15, 0, r0, c7, c10, 5 @ Data memory barrier
	mcr p15, 0, r0, c7, c5, 0  @ Invalidate entire instruction cache,
                               @ also flushes the branch target cache
	mcr p15, 0, r0, c7, c5, 4  @ Flush prefetch buffer
	mcr p15, 0, r0, c7, c5, 6  @ Flush entire branch target cache
	mcr p15, 0, r0, c7, c10, 4 @ Data synchronization barrier
	mrc p15, 0, r0, c1, c0, 1  @ Read auxiliary control register
	orr r0, r0, #0x5F          @ Enable return stack, dynamic branch prediction,
                               @ static branch prediction, exclusive behavior of L1,
                               @ instruction folding and parity checking
	mcr p15, 0, r0, c1, c0, 1  @ Write auxiliary control register
	mrc p15, 0, r0, c1, c0, 0  @ Read control register
	ldr r1, =0x1804            @ D-Cache, program flow prediction and I-Cache bitmask
	orr r0, r0, r1             @ Enable D-Cache, program flow prediction and I-Cache
	mcr p15, 0, r0, c1, c0, 0  @ Write control register

	ldr r0, =(CORE_SYNC_ID & 0xFFFFFFF0)
	mov r1, #0
	str r1, [r0, #0xC]         @ Clear arm9 communication fields
	str r1, [r0, #0x8]

	bl bss_clear

	blx main

endlessLoop:
	wfi                        @ Wait for interrupt
	b endlessLoop


bss_clear:
	ldr r1, =__bss_start__
	ldr r2, =__bss_end__
	mov r3, #0

	loop_clear:
	cmp r1, r2
	bxeq lr
	strb r3, [r1]
	add r1, r1, #1
	b loop_clear


disableCaches:
	mov r2, lr
	bl flushDCache
	mrc p15, 0, r0, c1, c0, 0  @ Read control register
	ldr r1, =0x1804            @ D-Cache, program flow prediction and I-Cache bitmask
	bic r0, r0, r1             @ Disable D-Cache, program flow prediction and I-Cache
	mcr p15, 0, r0, c1, c0, 0  @ Write control register
	bx r2

_init:
	bx lr
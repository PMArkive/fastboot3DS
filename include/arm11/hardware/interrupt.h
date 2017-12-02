#pragma once

/*
 *   This file is part of fastboot 3DS
 *   Copyright (C) 2017 derrek, profi200
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "types.h"


typedef enum
{
	IRQ_MPCORE_SW0    = 0,
	IRQ_MPCORE_SW1    = 1,
	IRQ_MPCORE_SW2    = 2,
	IRQ_MPCORE_SW3    = 3,
	IRQ_MPCORE_SW4    = 4,
	IRQ_MPCORE_SW5    = 5,
	IRQ_MPCORE_SW6    = 6,
	IRQ_MPCORE_SW7    = 7,
	IRQ_MPCORE_SW8    = 8,
	IRQ_MPCORE_SW9    = 9,
	IRQ_MPCORE_SW10   = 10,
	IRQ_MPCORE_SW11   = 11,
	IRQ_MPCORE_SW12   = 12,
	IRQ_MPCORE_SW13   = 13,
	IRQ_MPCORE_SW14   = 14,
	IRQ_MPCORE_SW15   = 15,
	IRQ_TIMER         = 29,  // MPCore timer
	IRQ_WATCHDOG      = 30,  // MPCore watchdog
	IRQ_PSC0          = 40,
	IRQ_PSC1          = 41,
	IRQ_PDC0          = 42,  // aka VBlank0
	IRQ_PDC1          = 43,  // aka VBlank1
	IRQ_PPF           = 44,
	IRQ_P3D           = 45,  // This is a guess
	IRQ_PXI_SYNC      = 80,
	IRQ_PXI_UNK       = 81,  // Unknown what this is for
	IRQ_PXI_NOT_FULL  = 82,
	IRQ_PXI_NOT_EMPTY = 83,
	IRQ_SHELL_OPENED  = 96,
	IRQ_SHELL_CLOSED  = 98,
	IRQ_TOUCHSCREEN   = 99,  // Triggers on hitting the touchscreen.
	IRQ_HEADPH_JACK   = 100, // Headphone jack. Triggers on both plugging in and out? GPIO reg 9, bit 8.
	IRQ_MCU_HID       = 113, // HOME/POWER pressed/released, shell opened/closed and WiFi switch pressed. GPIO reg 19, bit 9.
	IRQ_GAMECARD      = 117  // Gamecard inserted
} Interrupt;


// IRQ handler type.
// intSource: bit 10-12 CPU source ID (0 except for interrupt ID 0-15),
// bit 0-9 interrupt ID
typedef void (*IrqHandler)(u32 intSource);



/**
 * @brief      Initializes the generic interrupt controller.
 */
void IRQ_init(void);

/**
 * @brief      Registers a interrupt handler and enables the specified interrupt.
 *
 * @param[in]  id             The interrupt ID. Must be <128.
 * @param[in]  prio           The priority. 0 = highest, 14 = lowest, 15 = disabled
 * @param[in]  cpuMask        The CPU mask. Each of the 4 bits stands for 1 core. 0 means current CPU.
 * @param[in]  edgeTriggered  Set to true to make the interrupt edge triggered. false is level triggered.
 * @param[in]  handler        The interrupt handler to call.
 */
void IRQ_registerHandler(Interrupt id, u8 prio, u8 cpuMask, bool edgeTriggered, IrqHandler handler);

/**
 * @brief      Unregisters the interrupt handler and disables the specified interrupt.
 *
 * @param[in]  id    The interrupt ID. Must be <128.
 */
void IRQ_unregisterHandler(Interrupt id);

/**
 * @brief      Sets the priority of an interrupt.
 *
 * @param[in]  id    The interrupt ID. Must be <128.
 * @param[in]  prio  The priority. 0 = highest, 14 = lowest, 15 = disabled
 */
void IRQ_setPriority(Interrupt id, u8 prio);

/**
 * @brief      Triggers a software interrupt for the specified CPUs.
 *
 * @param[in]  id       The interrupt ID. Must be <16.
 * @param[in]  cpuMask  The CPU mask. Each of the 4 bits stands for 1 core.
 */
void IRQ_softwareInterrupt(Interrupt id, u8 cpuMask);


static inline void __wfi(void)
{
	__asm__ __volatile__("wfi" : : : "memory");
}

static inline u32 enterCriticalSection(void)
{
	u32 tmp;
	__asm__ __volatile__("mrs %0, cpsr\n"
	                     "cpsid i" : "=r" (tmp) : : "memory");
	return tmp & 0x80;
}

static inline void leaveCriticalSection(u32 oldState)
{
	u32 tmp;
	__asm__ __volatile__("mrs %0, cpsr" : "=r" (tmp) : );
	__asm__ __volatile__("msr cpsr_c, %0" : : "r" ((tmp & ~0x80u) | oldState) : "memory");
}

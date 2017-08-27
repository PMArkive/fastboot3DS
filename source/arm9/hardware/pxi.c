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
#include "hardware/pxi.h"
#include "arm9/hardware/interrupt.h"
#include "arm9/debug.h"
#include "ipc_handler.h"



static void pxiIrqHandler(UNUSED u32 id);

void PXI_init(void)
{
	REG_PXI_SYNC9 = PXI_IRQ_ENABLE;
	REG_PXI_CNT9 = PXI_FLUSH_SEND_FIFO | PXI_EMPTY_FULL_ERROR | PXI_ENABLE_SEND_RECV_FIFO;

	REG_PXI_SYNC9 |= 9u<<8;
	while(PXI_DATA_RECEIVED(REG_PXI_SYNC9) != 11u);

	IRQ_registerHandler(IRQ_PXI_SYNC, pxiIrqHandler);
}

static void pxiIrqHandler(UNUSED u32 id)
{
	const u32 cmdCode = REG_PXI_RECV9;
	const u8 params = cmdCode>>16 & 0xFFu;

	if(params != PXI_DATA_RECEIVED(REG_PXI_SYNC9))
	{
		panic();
	}

	REG_PXI_SEND9 = IPC_handleCmd(cmdCode>>24, params);
}

u32 PXI_sendCmd(u32 cmd, const u32 *const buf, u8 words)
{
	if(!buf) words = 0;

	while(REG_PXI_CNT9 & PXI_SEND_FIFO_FULL);
	REG_PXI_SEND9 = cmd | words<<16;
	for(u32 i = 0; i < words; i++)
	{
		REG_PXI_SEND9 = buf[i];
	}

	REG_PXI_SYNC9 = PXI_DATA_SENT(REG_PXI_SYNC9, words) | PXI_NOTIFY_11;

	while(REG_PXI_CNT9 & PXI_RECV_FIFO_EMPTY);
	return REG_PXI_RECV9;
}

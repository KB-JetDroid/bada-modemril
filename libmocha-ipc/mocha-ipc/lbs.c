/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2011 KB <kbjetdroid@gmail.com>
 *
 * libmocha-ipc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libmocha-ipc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libmocha-ipc.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <getopt.h>

#include <radio.h>
#include <lbs.h>

#define LOG_TAG "RIL-Mocha-LBS"
#include <utils/Log.h>


void lbs_init(void)
{
	lbs_send_init(1);
}

uint8_t lbsSendBuf[0x100C];

void lbs_send_packet(uint32_t type, uint32_t size, uint32_t subType, void* buf)
{	
	struct modem_io pkt;
	struct lbsPacketHeader* hdr;
	void* sendBuf;
	sendBuf = (void*) lbsSendBuf;
	hdr = (struct lbsPacketHeader*) &lbsSendBuf;
	
	/* BIG WTF at the lengths below, shitload of uninitialized, redundant data 
	 * OHAI retarded Samsung devs */
	if(type <= 6 || (type >= 21 && type <= 30)) //common 1
		pkt.datasize = 0x100C;
	else if((type >= 7 && type <= 12) || 
			(type >= 31 && type <= 35)) //supl 2
		pkt.datasize = 0x81C;
	else if((type >= 13 && type <= 14) || 
			(type >= 16 && type <= 18) || 
			(type >= 36 && type <= 39)) //xtra 3
		pkt.datasize = 0x3C;
	else if(type == 15) //large_xtra 5
	{
		sendBuf = malloc(0xA014);
		pkt.datasize = 0xA014;
	}
	else if(type == 19 || type == 20) //baseband 5
		pkt.datasize = 0xDC;
	else
	{
		DEBUG_E("Unknown LBS packet type, abandoning...");
		return;
	}
	if(size > pkt.datasize - sizeof(struct lbsPacketHeader))
	{
		DEBUG_E("Too big LBS packet, type %d, len %d", type, size);
		return;
	}
	
	hdr->type = type;
	hdr->size = size;
	hdr->subType = subType;
	memcpy((char*)(sendBuf) + sizeof(struct lbsPacketHeader), buf, size);
	
	pkt.magic = 0xCAFECAFE;
	pkt.cmd = FIFO_PKT_LBS;
	pkt.data = sendBuf;
	
	ipc_send(&pkt);
	if(type == 15)
		free(sendBuf);
}

void lbs_send_init(uint32_t var)
{
	lbs_send_packet(0, 4, 1, (void*)&var);
}

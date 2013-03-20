/**
 * This file is part of libsamsung-ipc.
 *
 * Copyright (C) 2012 Dominik Marszk <dmarszk@gmail.com>
 *
 *
 * libsamsung-ipc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libsamsung-ipc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libsamsung-ipc.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdlib.h>
#include <stdio.h>

#include <drv.h>
#include <tapi.h>
#include <fm.h>
#include <sim.h>
#include <radio.h>
#include <syssec.h>

#include "ipc_private.h"

#define LOG_TAG "Mocha-RIL-IPC-MISC"
#include <utils/Log.h>

void ipc_send_debug_level(uint32_t debug_level)
{
	struct modem_io pkt;
	pkt.magic = 0xCAFECAFE;
	pkt.cmd = FIFO_PKT_DVB_H_DebugLevel;
	pkt.data = (uint8_t*)&debug_level;
	pkt.datasize = 4;
	ipc_send(&pkt);
}

void ipc_send_lazy_fw_ver(void)
{
	uint8_t buf[0x18];
	struct modem_io pkt;
	memset(buf, 0, 0x18);
	pkt.magic = 0xCAFECAFE;
	pkt.cmd = FIFO_PKT_BOOT;
	pkt.data = buf;
	*(uint32_t*)(&buf) = 0xC;
	strcpy((char*)buf+4, fake_apps_version);
	pkt.datasize = 0x18;
	ipc_send(&pkt);
}

void ipc_send_lpm_mode(int lpmEnabled)
{
	uint32_t buf[2];
	struct modem_io pkt;
	pkt.magic = 0xCAFECAFE;
	pkt.cmd = FIFO_PKT_BOOT;
	pkt.data = (uint8_t*)&buf;
	buf[0] = 0xB; /* LPM state */
	buf[1] = lpmEnabled;
	pkt.datasize = 8;
	ipc_send(&pkt);
}


void ipc_power_mode(int mode)
{
	uint32_t buf;
	struct modem_io pkt;
	buf = mode;
	pkt.magic = 0xCAFECAFE;
	pkt.cmd = FIFO_PKT_BOOT;
	pkt.data = (uint8_t*)&buf;
	pkt.datasize = 4;
	ipc_send(&pkt);
}


void ipc_parse_boot(struct ipc_client *client, struct modem_io *ipc_frame)
{
	DEBUG_I("Inside ipc_parse_boot\n");
	int retval, count;
	
    DEBUG_I("Inside ipc_parse_boot leaving\n");

}

void ipc_parse_dbg_level(struct ipc_client *client, struct modem_io *ipc_frame)
{
	DEBUG_I("Inside ipc_parse_dbg_level\n");

	ipc_send_debug_level(1);
	/* Initialize AMSS in LPM mode by default, we'll initialize network interfaces on RIL request */
	ipc_send_lpm_mode(1);
	syssec_send_imei();
	ipc_send_lazy_fw_ver();
	DEBUG_I("Inside ipc_parse_dbg_level leaving\n");

}

void ipc_parse_system(struct ipc_client *client, struct modem_io *ipc_frame)
{
	DEBUG_I("received SYSTEM packet with AMSS version, notifying RIL that AMSS has initialized");
	uint32_t desc_size;
	int suffix_size;
	desc_size = strlen((const char*)ipc_frame->data);
	if(desc_size > 32 || desc_size > ipc_frame->datasize)
		DEBUG_E("too big desc_size: %d", desc_size);
	else
		memcpy(cached_sw_version, ipc_frame->data, desc_size);
	cached_sw_version[desc_size] = 0x00;
	suffix_size = ipc_frame->datasize - desc_size - 1;
	if(suffix_size > 0) {
		DEBUG_I("dumping rest of data from IPC_SYSTEM packet");
		ipc_hex_dump(client, ipc_frame->data+desc_size+1, suffix_size);
	}
	ipc_invoke_ril_cb(CP_SYSTEM_START, ipc_frame->data);
}

void ipc_parse_dbg(struct ipc_client *client, struct modem_io *ipc_frame)
{
	ipc_client_log(client, "AMSS debugstring - %s\n", (char *)(ipc_frame->data));
}


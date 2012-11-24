/**
 * This file is part of libsamsung-ipc.
 *
 * Copyright (C) 2012 KB <kbjetdroid@gmail.com>
 *
 * Implemented as per the Mocha AP-CP protocol analysis done by Dominik Marszk
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
 */

#include <stdlib.h>

#include <radio.h>
#include <tapi.h>
#include <tapi_nettext.h>

#define LOG_TAG "RIL-TAPI-SMS"
#include <utils/Log.h>

/*
 * All the TAPI Nettext handling will be done here
 *
 */

void tapi_nettext_parser(uint16_t tapiNettextType, uint32_t tapiNettextLength, uint8_t *tapiNettextData)
{
	struct tapiPacket tx_packet;

	struct modem_io request;
    uint8_t *frame;
    uint8_t *payload;
    uint32_t frame_length;

    switch(tapiNettextType)
    {
    case 0x01:
    	break;
    default:
    	break;
    }
}

void tapi_nettext_set_mem_available(uint32_t bMemAvail)
{
	struct tapiPacket pkt;
	pkt.header.len = 4;
	pkt.header.tapiService = TAPI_TYPE_NETTEXT;	
	pkt.header.tapiServiceFunction = TAPI_NETTEXT_SETMEMAVAIL;
	pkt.buf = malloc(4);
	(*(uint32_t*)pkt.buf) = bMemAvail;
	
	tapi_send_packet(&pkt);
	free(pkt.buf);
}

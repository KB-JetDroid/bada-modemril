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

#define LOG_TAG "Mocha-RIL-TAPI-SMS"
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
	case TAPI_NETTEXT_INCOMING:
		tapi_nettext_incoming(tapiNettextLength, tapiNettextData);
		break;	
    	default:
		DEBUG_I("TapiNettext packet type 0x%X is not yet handled, len = 0x%x", tapiNettextType, tapiNettextLength);
		hex_dump(tapiNettextData, tapiNettextLength);
	    	break;
    }
}

void tapi_nettext_set_mem_available(uint32_t bMemAvail)
{
	struct tapiPacket pkt;
	pkt.header.len = 4;
	pkt.header.tapiService = TAPI_TYPE_NETTEXT;	
	pkt.header.tapiServiceFunction = TAPI_NETTEXT_SET_MEM_AVAIL;
	pkt.buf = &bMemAvail;	
	tapi_send_packet(&pkt);
}

void tapi_nettext_set_preferred_memory(uint8_t preferredMemory)
{	
	struct tapiPacket pkt;
	pkt.header.len = 1;
	pkt.header.tapiService = TAPI_TYPE_NETTEXT;	
	pkt.header.tapiServiceFunction = TAPI_NETTEXT_SET_PREFERRED_MEM;
	pkt.buf = &preferredMemory;	
	tapi_send_packet(&pkt);
}

void tapi_nettext_set_net_burst(uint32_t bNetBurstEnabled)
{	
	struct tapiPacket pkt;
	pkt.header.len = 4;
	pkt.header.tapiService = TAPI_TYPE_NETTEXT;	
	pkt.header.tapiServiceFunction = TAPI_NETTEXT_SET_BURST;
	pkt.buf = &bNetBurstEnabled;	
	tapi_send_packet(&pkt);
}

void tapi_nettext_set_cb_settings(uint8_t* cb_sett_buf)
{	
	struct tapiPacket pkt;
	pkt.header.len = 0x64;
	pkt.header.tapiService = TAPI_TYPE_NETTEXT;	
	pkt.header.tapiServiceFunction = TAPI_NETTEXT_SET_CB_SETTING;
	pkt.buf = cb_sett_buf;	
	tapi_send_packet(&pkt);
}

void tapi_nettext_incoming(uint32_t tapiNettextLength, uint8_t *tapiNettextData)
{
	tapiNettextInfo* nettextInfo = (tapiNettextInfo*)(tapiNettextData);
	DEBUG_I("tapi_nettext_incoming: Incoming SMS received from %s, message: %s", nettextInfo->phoneNumber, nettextInfo->messageBody);
	ipc_invoke_ril_cb(NETTEXT_INCOMING, (void*)nettextInfo);
}

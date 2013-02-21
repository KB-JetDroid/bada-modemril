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
#include <tapi_call.h>

#define LOG_TAG "Mocha-RIL-TAPI-CALL"
#include <utils/Log.h>

/*
 * All the TAPI call handling will be done here
 *
 */

void tapi_call_parser(uint16_t tapiCallType, uint32_t tapiCallLength, uint8_t *tapiCallData)
{
	struct tapiPacket tx_packet;

	struct modem_io request;
    uint8_t *frame;
    uint8_t *payload;
    uint32_t frame_length;

    switch(tapiCallType)
    {
	case TAPI_CALL_APIREQ:
		/* Confirmation of properly executed function, just drop it */
		break;
	case TAPI_CALL_INCOMING_IND:
		tapi_call_incoming_ind(tapiCallLength, tapiCallData);
		break;
	case TAPI_CALL_END_IND:
		tapi_call_end_ind(tapiCallLength, tapiCallData);
		break;
    default:	
		DEBUG_I("TapiCall Packet type 0x%X is not yet handled, len = 0x%x", tapiCallType, tapiCallLength);
		hex_dump(tapiCallData, tapiCallLength);
    	break;
    }
}


void tapi_call_incoming_ind(uint32_t tapiCallLength, uint8_t *tapiCallData)
{
	tapiCallInfo* callInfo = (tapiCallInfo*)(tapiCallData);
	DEBUG_I("tapi_call_incoming_ind: Incoming call received from %s", callInfo->phoneNumber);
	DEBUG_I("tapi_call_incoming_ind: callId = %d", callInfo->callId );
	ipc_invoke_ril_cb(CALL_INCOMING_IND, (void*)callInfo);
}
		
void tapi_call_end_ind(uint32_t tapiCallLength, uint8_t *tapiCallData)
{
	DEBUG_I("tapi_call_end_ind");
	ipc_invoke_ril_cb(CALL_END_IND, (void*)tapiCallData);
}

void tapi_call_hangup(uint32_t callId)
{
	struct tapiPacket tx_packet;
	uint8_t resp_buf[12];
	*(uint32_t*)(resp_buf) = 0;
	*(uint32_t*)(resp_buf+4) = callId; 
	*(uint32_t*)(resp_buf+8) = 0;
	tx_packet.buf = resp_buf;
	tx_packet.header.tapiService = 0;
	tx_packet.header.tapiServiceFunction = 3;
	tx_packet.header.len = 12;
	tapi_send_packet(&tx_packet);
}

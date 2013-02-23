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

void tapi_call_release(uint8_t callType,uint32_t callId, uint8_t releaseCause)
{
	struct tapiPacket tx_packet;
	tapiCallRelease* callRelease;
	callRelease = (tapiCallRelease*)malloc(sizeof(tapiCallRelease));
	memset(callRelease, 0, sizeof(tapiCallRelease));
	callRelease->callType = callType;
	callRelease->align0[0] = 0x0;
	callRelease->align0[1] = 0x0;
	callRelease->align0[2] = 0x0;
	callRelease->callId = callId;
	callRelease->releaseCause = releaseCause; 
	callRelease->align1[0] = 0x0;
	callRelease->align1[1] = 0x0;
	callRelease->align1[2] = 0x0;
	tx_packet.buf = (uint8_t *)callRelease;
	tx_packet.header.tapiService = 0;
	tx_packet.header.tapiServiceFunction = TAPI_CALL_RELEASE;
	tx_packet.header.len = 12;
	tapi_send_packet(&tx_packet);
}

void tapi_call_answer(uint8_t callType, uint32_t callId)
{
	struct tapiPacket tx_packet;
	tapiCallAnswer* callAnswer;
	callAnswer = (tapiCallAnswer*)malloc(sizeof(tapiCallAnswer));
	memset(callAnswer, 0, sizeof(tapiCallAnswer));
	callAnswer->callType = callType;
	callAnswer->align[0] = 0x0;
	callAnswer->align[1] = 0x0;
	callAnswer->align[2] = 0x0;
	callAnswer->callId = callId; 
	tx_packet.buf = (uint8_t *)callAnswer;
	tx_packet.header.tapiService = 0;
	tx_packet.header.tapiServiceFunction = TAPI_CALL_ANSWER;
	tx_packet.header.len = 8;
	tapi_send_packet(&tx_packet);
}

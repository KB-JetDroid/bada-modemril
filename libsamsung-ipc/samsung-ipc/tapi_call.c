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
	case 0x00:
		/* Confirmation of properly executed function, just drop it */
		break;
    default:	
		DEBUG_I("TapiCall Packet type 0x%X is not yet handled, len = 0x%x", tapiCallType, tapiCallLength);
		hex_dump(tapiCallData, tapiCallLength);
    	break;
    }
}

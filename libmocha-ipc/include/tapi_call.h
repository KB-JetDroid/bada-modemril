/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2012 KB <kbjetdroid@gmail.com>
 *
 * Implemented as per the Mocha AP-CP protocol analysis done by Dominik Marszk
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
 *
 */

#ifndef __TAPI_CALL_H__
#define __TAPI_CALL_H__

#if defined(DEVICE_JET)
#include "device/jet/tapi_call.h"
#elif defined(DEVICE_WAVE)
#include "device/wave/tapi_call.h"
#endif

/**
 * All the TAPI call context structures generic to all Mocha devices will be defined here
 */

struct tapiCall {
	uint8_t type; //1 or 2 or 3
	uint32_t handle;
};
typedef struct {
	uint32_t callId;
	uint8_t callType; 
	uint8_t nameMode;
	char phoneNumber[1];
} __attribute__((__packed__)) tapiCallInfo;

typedef struct {
	uint8_t callType;
	uint8_t align[3]; //to align to DWORD, uninitialized
	uint32_t callId;
} __attribute__((__packed__)) tapiCallAnswer;

typedef struct{
	/* Alignments aren't initialized in APPS */
	uint8_t callType;
	uint8_t align0[3];
	uint32_t callId; 
	uint8_t releaseCause;
	uint8_t align1[3];
} __attribute__((__packed__)) tapiCallRelease;

typedef struct{
	uint32_t unknown0; //01
	uint32_t unknown1; //01
	uint32_t unknown2; // 01 or 03
	uint32_t unknown3; //5C 25 C8 41
	uint32_t unknown4; // FF
	uint32_t unknown5; //01
	uint8_t unknown6; //02
	char number1[83];
	uint32_t timestamp0;
	uint32_t timestamp1;
	uint8_t unknown7; // 00
	uint8_t unknown8; //01
	uint8_t unknown9[82]; //00
	uint8_t unknown10; //01
	uint8_t unknown11[759]; //00
	uint32_t unknown12; //3A
	uint8_t unknown13[284]; //00
	uint8_t unknown14; //01
	uint8_t unknown15[268]; //00
	char number2[551];	
} __attribute__((__packed__)) tapiCallSetup;


void tapi_call_parser(uint16_t tapiCallType, uint32_t tapiCallLength, uint8_t *tapiCallData);
void tapi_call_incoming_ind(uint32_t tapiCallLength, uint8_t *tapiCallData);
void tapi_call_end_ind(uint32_t tapiCallLength, uint8_t *tapiCallData);
void tapi_call_setup_ind(uint32_t tapiCallLength, uint8_t *tapiCallData);
void tapi_call_connected_number_ind(uint32_t tapiCallLength, uint8_t *tapiCallData);
void tapi_call_release(uint8_t callType,uint32_t callId, uint8_t releaseCause);
void tapi_call_answer(uint8_t callType, uint32_t callId);
void tapi_call_setup(tapiCallSetup* callSetup);

#endif

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
 *
 */

#ifndef __TAPI_NETTEXT_H__
#define __TAPI_NETTEXT_H__

#if defined(DEVICE_JET)
#include "device/jet/tapi_nettext.h"
#elif defined(DEVICE_WAVE)
#include "device/wave/tapi_nettext.h"
#endif

/**
 * All the TAPI Nettext (sms) context structures generic to all Mocha devices will be defined here
 */

struct tapiNetText {
	uint32_t handle;
};

typedef struct {
	uint32_t ext_cb;
	uint32_t ext_cb_enable;
	uint32_t enable_all_combined_cb_channels;
	uint32_t combined_language_type;
	uint32_t number_of_combined_cbmi;
	uint16_t cb_info[40];
} __attribute__((__packed__)) tapi_nettext_cb_settings;

typedef struct {
	uint8_t msgType; 
	uint8_t bearer; 
	uint8_t unknown1[6]; 
	char phoneNumber [48]; 
	uint8_t unknown2[8]; 
	char serviceCenter [48]; 
	uint8_t unknown3[32]; 
	uint32_t unknown4; 
	uint8_t nUDH; 
} __attribute__((__packed__)) tapiNettextInfo;

typedef struct {
	uint8_t messageLength; 
	uint8_t messageBody[160];
} __attribute__((__packed__)) tapiNettextSingleInfo;

typedef struct {
	uint8_t messageLength; 
	uint8_t unknownUDH1; 
	uint8_t unknownUDH2; 
	uint8_t unknownUDH3; 
	uint8_t quantityParts; 
	uint8_t numberPart; 	
	uint8_t messageBody[160];
} __attribute__((__packed__)) tapiNettextMultiInfo;

typedef struct {
	uint32_t Unknown1[7]; // all is NULL
	uint8_t Unknown2; // 00
	uint8_t Unknown3; // 01
	uint8_t numberType; // 00 -national, 01 - international
	uint8_t numberLength;
	char phoneNumber [24]; 
	uint8_t unknown4[7]; 
	uint8_t serviceCenterLength;
	char serviceCenter [48]; 
	uint8_t unknown5; //01
	uint8_t unknown6; //03
	uint8_t unknown7[6];// all is NULL
	uint8_t unknown8; //FF
	uint8_t unknown9[17];// all is NULL 
	uint8_t messageDCS;
	uint8_t unknown10; //04	
	uint8_t unknown11; //04
	uint32_t unknown12[2];	
	uint8_t messageLength;
	uint8_t messageBody[161]; 
} __attribute__((__packed__)) tapiNettextOutgoingMessage;

typedef struct {
	uint32_t unknown1; 
	uint32_t status;
} __attribute__((__packed__)) tapiNettextCallBack;


void tapi_nettext_parser(uint16_t tapiNettextType, uint32_t tapiNettextLength, uint8_t *tapiNettextData);

void tapi_nettext_set_mem_available(uint32_t bMemAvail);
void tapi_nettext_set_preferred_memory(uint8_t preferredMemory);
void tapi_nettext_set_net_burst(uint32_t bNetBurstEnabled);
void tapi_nettext_set_cb_settings(uint8_t* cb_sett_buf);
void tapi_nettext_incoming(uint32_t tapiNettextLength, uint8_t *tapiNettextData);
void tapi_nettext_send(uint8_t* tapiNettextOutgoingMessage);
void tapi_nettext_send_callback(uint8_t *callBack);

#endif

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
	uint8_t messageLength; 
	uint8_t messageBody[160];
} __attribute__((__packed__)) tapiNettextInfo;

void tapi_nettext_parser(uint16_t tapiNettextType, uint32_t tapiNettextLength, uint8_t *tapiNettextData);

void tapi_nettext_set_mem_available(uint32_t bMemAvail);
void tapi_nettext_set_preferred_memory(uint8_t preferredMemory);
void tapi_nettext_set_net_burst(uint32_t bNetBurstEnabled);
void tapi_nettext_set_cb_settings(uint8_t* cb_sett_buf);
void tapi_nettext_incoming(uint32_t tapiNettextLength, uint8_t *tapiNettextData);

#endif

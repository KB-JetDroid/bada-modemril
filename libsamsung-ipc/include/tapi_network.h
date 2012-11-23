/**
 * This file is part of libsamsung-ipc.
 *
 * Copyright (C) 2012 Dominik Marszk <dmarszk@gmail.com>
 *                    KB <kbjetdroid@gmail.com>
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

#ifndef __TAPI_NETWORK_H__
#define __TAPI_NETWORK_H__

#if defined(DEVICE_JET)
#include "device/jet/tapi_network.h"
#elif defined(DEVICE_WAVE)
#include "device/wave/tapi_network.h"
#endif

/**
 * All the TAPI Network context structures generic to all Mocha devices will be defined here
 */

//TODO: check if following definitions does apply to JET too, confirm it for WAVE
struct tapiNetRegistrationFail {
	uint8_t state;
	uint8_t cause;
} __attribute__((__packed__));

#define NET_MAX_NAME_LEN 40 //it's kinda guess
#define NET_MAX_SPN_LEN 24 //same as above

struct tapiNetworkInfo { //not sure if it's specific to WAVE or same for JET, 
//most likely sizeof should be 0x78 bytes for WAVE
	uint8_t serviceLevel;
	uint8_t serviceType;
	uint8_t psServiceType;
	uint8_t _unknown_; //its possible that here is whole systemId structure
	uint8_t systemType; //9 - unknown, ignored when offline mode, 7 - unknown
	uint8_t _unknown2_[23];	//its possible that here is whole systemId structure
	char name[NET_MAX_NAME_LEN];
	char spn[NET_MAX_SPN_LEN];
	uint32_t bRoaming;
	uint32_t bDisplayPplmn;
	uint32_t bDisplaySpn;
	uint8_t _unknown3_[8]; //this can belong to NetworkInfo or registrationFail
	struct tapiNetRegistrationFail registrationFail;	
	uint8_t _unknown4_[6]; //this can belong to NetworkInfo or registrationFail
	
} __attribute__((__packed__));

struct tapiStartupNetworkInfo {
//most likely sizeof should be 0x14 bytes for WAVE
	uint32_t bAuto;
	uint32_t bAttach;
	uint8_t networkOrder;
	uint8_t serviceDomain;
	uint8_t _unknown_[2];
	uint32_t mode;
	uint8_t subs;
	uint8_t bFlight;
} __attribute__((__packed__));

struct tapiCellInfo {
//most likely sizeof should be 0x1C bytes for WAVE
	uint32_t bCellChanged;
	uint32_t bRACChanged;
	uint32_t bLACChanged;
	uint32_t bPLMNChanged;
	uint8_t cellId[4];
	uint8_t racId;
	uint8_t ladId[2];
	uint8_t plnmId[3];
	uint8_t cbchStatus;
	uint8_t _unknown_;
} __attribute__((__packed__));

struct tapiNetwork {
	struct tapiNetworkInfo networkInfo;
	uint8_t networkOrder;
};

void tapi_network_parser(uint16_t tapiNetworkType, uint32_t tapiNetworkLength, uint8_t *tapiNetworkData);

//void tapi_network_apiRequest(uint32_t tapiNetLength, uint8_t *tapiNetData);
void tapi_network_startup(uint32_t tapiNetLength, uint8_t *tapiNetData);
void tapi_network_setSubscriptionMode(uint32_t tapiNetLength, uint8_t *tapiNetData);
void tapi_network_radioInfo(uint32_t tapiNetLength, uint8_t *tapiNetData);
void tapi_network_networkSelectInd(uint32_t tapiNetLength, uint8_t *tapiNetData);
void tapi_network_commonError(uint32_t tapiNetLength, uint8_t *tapiNetData);
void tapi_network_cellInfo(uint32_t tapiNetLength, uint8_t *tapiNetData);

int tapi_network_init(void);

#endif

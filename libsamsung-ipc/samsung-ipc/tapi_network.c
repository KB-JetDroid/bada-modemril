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
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <radio.h>
#include <tapi_packet.h>
#include <tapi_network.h>

#define LOG_TAG "RIL-TAPI-NET"
#include <utils/Log.h>

/*
 * All the TAPI Network handling will be done here
 *
 */

void tapi_network_handler(uint16_t tapiNetType, uint32_t tapiNetLength, uint8_t *tapiNetData)
{
	struct tapiPacket tx_packet;

	struct modem_io request;
    uint8_t *frame;
    uint8_t *payload;
    uint32_t frame_length;

    switch(tapiNetType)
    {
//	case TAPI_NETWORK_APIREQ:
//		tapi_network_apiRequest(tapiNetLength, tapiNetData);
//		break;
	case TAPI_NETWORK_STARTUP:	
		tapi_network_startup(tapiNetLength, tapiNetData);
		break;
	case TAPI_NETWORK_SETSUBMODE:
		tapi_network_setSubscriptionMode(tapiNetLength, tapiNetData);
		break;
    case TAPI_NETWORK_RADIOINFO:
		tapi_network_radioInfo(tapiNetLength, tapiNetData);
    	break;
	case TAPI_NETWORK_SELECTNET:
		tapi_network_networkSelect(tapiNetLength, tapiNetData);
		break;
	case TAPI_NETWORK_COMMONERROR:
		tapi_network_commonError(tapiNetLength, tapiNetData);
		break;
	case TAPI_NETWORK_CELLINFO:
		tapi_network_cellInfo(tapiNetLength, tapiNetData);
		break;
    default:
    	break;
    }
}

void tapi_network_apiRequest(uint32_t tapiNetLength, uint8_t *tapiNetData)
{	
	DEBUG_I("\n");
	//call handler here
}

void tapi_network_startup(uint32_t tapiNetLength, uint8_t *tapiNetData)
{
	struct tapiStartupNetworkInfo* startInfo = (struct tapiStartupNetworkInfo*)(tapiNetData);
	DEBUG_I("Auto:%d,bAttach:%d,mode=%d,networkOrder:%d,serviceDomain:%d,subs:%d,bFlight=%d\n", startInfo->bAuto, startInfo->bAttach, startInfo->mode, startInfo->networkOrder, startInfo->serviceDomain, startInfo->subs, startInfo->bFlight);
	//maybe memcpy info and call handler here	
}

void tapi_network_setSubscriptionMode(uint32_t tapiNetLength, uint8_t *tapiNetData)
{
	uint8_t subscriptionMode = (uint8_t)tapiNetData[0];	
	DEBUG_I("mode:%d\n", subscriptionMode);
	//TODO: bounce-back packet to CP, with the same type, subtype and mode
}
void tapi_network_networkSelect(uint32_t tapiNetLength, uint8_t *tapiNetData)
{	
	struct tapiNetworkInfo* netInfo = (struct tapiNetworkInfo*)(tapiNetData);	
	
	DEBUG_I("serviceLevel=%d, serviceType=%d, psServiceType=%d, systemId.systemType=%d,bRoaming=%d,name=%s, spn =%s,registrationFail.state = %d, registrationFail.cause = %d, bDisplayPplmn = %d, bDisplaySpn = %d\n", netInfo->serviceLevel, netInfo->serviceType, netInfo->psServiceType, netInfo->systemType, netInfo->bRoaming, netInfo->name, netInfo->spn, netInfo->registrationFail.state, netInfo->registrationFail.cause, netInfo->bDisplayPplmn, netInfo->bDisplaySpn);
	//maybe memcpy info and call handler here	
}

void tapi_network_radioInfo(uint32_t tapiNetLength, uint8_t *tapiNetData)
{

	DEBUG_I("\n");
	uint8_t* radioInfo = malloc(tapiNetLength);
	memcpy(radioInfo, tapiNetData, tapiNetLength);
	//call handler here
	free(radioInfo);
}

void tapi_network_commonError(uint32_t tapiNetLength, uint8_t *tapiNetData)
{	
	uint8_t networkOptError = (uint8_t)tapiNetData[0];
	DEBUG_I("networkOptError: %d\n", networkOptError);
	//call handler here
}

void tapi_network_cellInfo(uint32_t tapiNetLength, uint8_t *tapiNetData)
{	
	struct tapiCellInfo* cellInfo = (struct tapiCellInfo*)(tapiNetData);	
	DEBUG_I("cbchStatus:%d, bCellChanged:%d, bRACChanged:%d, bLACChanged:%d, bPLMNChanged:%d\n", cellInfo->cbchStatus, cellInfo->bCellChanged, cellInfo->bRACChanged, cellInfo->bLACChanged, cellInfo->bPLMNChanged);
	DEBUG_I("cellId:%x %x %x %x, racId:%x, ladId:%x %x, plmnId:%x %x %x)\n", cellInfo->cellId[0], cellInfo->cellId[1], cellInfo->cellId[2], cellInfo->cellId[3], cellInfo->racId, cellInfo->ladId[0], cellInfo->ladId[1], cellInfo->plnmId[0], cellInfo->plnmId[1], cellInfo->plnmId[2]);
	
	//call handler here
}

int tapi_network_init(void)
{
	struct tapiPacket pkt;
	pkt.header.len = 0;
	pkt.header.tapiService = TAPI_TYPE_NETWORK;	
	pkt.header.tapiServiceFunction = TAPI_NETWORK_INIT;
	pkt.buf = NULL;
	
	return tapi_send_packet(&pkt);
}

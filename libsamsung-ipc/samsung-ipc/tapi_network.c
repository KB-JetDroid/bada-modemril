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

#include <radio.h>
#include <tapi_packet.h>
#include <tapi_network.h>

/*
 * All the TAPI Network handling will be done here
 *
 */

void tapi_network_handler(unsigned short tapiNetType, unsigned int tapiNetLength, char *tapiNetData)
{
	struct tapiRequest tx_packet;

	struct modem_io request;
    unsigned char *frame;
    unsigned char *payload;
    int frame_length;

    switch(tapiNetType)
    {
	case TAPI_NETWORK_APIREQ:		
		tapi_network_apiRequest(tapiNetLength, tapiNetData);
		break;
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

void tapi_network_apiRequest(unsigned int tapiNetLength, char *tapiNetData)
{	
	printf("%s\n", __FUNCTION__);
	//call handler here
}

void tapi_network_startup(unsigned int tapiNetLength, char *tapiNetData)
{
	struct tapiStartupInfo* startInfo = (struct tapiStartupInfo*)(tapiNetData);	
	printf("%s: \bAuto:%d,bAttach:%d,mode=%d,networkOrder:%d,serviceDomain:%d,subs:%d,bFlight=%d\n", __FUNCTION__, startInfo->bAuto, starInfo->bAttach, startInfo->mode, startInfo->networkOrder, startInfo->serviceDomain, startInfo->subs, startInfo->bFlight);	
	//maybe memcpy info and call handler here	
}

void tapi_network_setSubscriptionMode(unsigned int tapiNetLength, char *tapiNetData)
{
	uint8_t subscriptionMode = (uint8_t)tapiNetData[0];	
	printf("%s: mode:%d\n", __FUNCTION__, subscriptionMode);
	//TODO: bounce-back packet to CP, with the same type, subtype and mode
}
void tapi_network_networkSelect(unsigned int tapiNetLength, char *tapiNetData)
{	
	struct tapiNetworkInfo* netInfo = (struct tapiNetworkInfo*)(tapiNetData);	
	
	printf("%s: serviceLevel=%d, serviceType=%d, psServiceType=%d, systemId.systemType=%d,bRoaming=%d,name=%s, spn =%s,registrationFail.state = %d, registrationFail.cause = %d, bDisplayPplmn = %d, bDisplaySpn = %d\n", __FUNCTION__, netInfo->serviceLevel, netInfo->serviceType, netInfo->psServiceType, netInfo->systemType, netInfo->bRoaming, &netInfo->name, &netInfo->spn, netInfo->registrationFail.state, netInfo->registrationFail.cause, netInfo->bDisplayPplmn, netInfo->bDisplaySpn);
	//maybe memcpy info and call handler here	
}

void tapi_network_radioInfo(unsigned int tapiNetLength, char *tapiNetData)
{

	printf("%s\n", __FUNCTION__);
	char* radioInfo = malloc(tapiNetLength);
	memcpy(radioInfo, tapiNetData, tapiNetLength);
	//call handler here
	free(radioInfo);
}

void tapi_network_commonError(unsigned int tapiNetLength, char *tapiNetData)
{	
	uint8_t networkOptError = (uint8_t)tapiNetData[0];
	printf("%s networkOptError: %d\n", __FUNCTION__, networkOptError);
	//call handler here
}

void tapi_network_cellInfo(unsigned int tapiNetLength, char *tapiNetData)
{	
	struct tapiCellInfo* cellInfo = (struct tapiCellInfo*)(tapiNetData);	
	printf("%s - cbchStatus:%d, bCellChanged:%d, bRACChanged:%d, bLACChanged:%d, bPLMNChanged:%d\n", __FUNCTION__, cellInfo->cbchStatus, cellInfo->bCellChanged, cellInfo->bRACChanged, cellInfo->bLacChanged, cellInfo->bPLMNChanged);
	printf("%s - cellId:%x %x %x %x, racId:%x, ladId:%x %x, plmnId:%x %x %x)\n", __FUNCTION__, cellInfo->cellId[0], cellInfo->cellId[1], cellInfo->cellId[2], cellInfo->cellId[3], cellInfo->racId, cellInfo->ladId[0], cellInfo->ladId[1], cellInfo->plmnId[0], cellInfo->plmnId[1], cellInfo->plmnId[2]);
	
	//call handler here
}
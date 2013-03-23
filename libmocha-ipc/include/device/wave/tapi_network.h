/**
 * This file is part of libmocha-ipc.
 *
 * Copyright (C) 2012 Dominik Marszk <dmarszk@gmail.com>
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

#ifndef __DEVICE_WAVE_NETWORK_H__
#define __DEVICE_WAVE_NETWORK_H__

/**
 * All the TAPI Network context structures specific to Wave will be defined here
 */

enum TAPI_NETWORK_TYPE
{
	TAPI_NETWORK_INIT 				= 0x45,	
	TAPI_NETWORK_STARTUP			= 0x46,
	TAPI_NETWORK_SHUTDOWN			= 0x47,
	TAPI_NETWORK_SET_OFFLINE_MODE	= 0x48,
	TAPI_NETWORK_SELECT				= 0x49,
	TAPI_NETWORK_RESELECT			= 0x4A,
	//0x4B unused
	TAPI_NETWORK_SEARCH				= 0x4C,
	TAPI_NETWORK_SET_SELECTION_MODE	= 0x4D,
	//0x4E unused
	TAPI_NETWORK_SET_SERVICE_DOMAIN	= 0x4F,
	TAPI_NETWORK_SET_NETWORK_ORDER	= 0x50,
	TAPI_NETWORK_SET_MODE			= 0x51,
	TAPI_NETWORK_SET_SUBSCRIPTION_MODE = 0x52,
	TAPI_NETWORK_SELECT_CNF			= 0x53,
	TAPI_NETWORK_SELECT_IND			= 0x54,
	TAPI_NETWORK_SEARCH_CNF			= 0x55,
	TAPI_NETWORK_SEARCH_IND			= 0x56,
	TAPI_NETWORK_RADIO_INFO 		= 0x57, //signal info, 3x uint8_t {rxLevel, quality, unk}	
	TAPI_NETWORK_COMMON_ERROR 		= 0x58,
	TAPI_NETWORK_CELL_INFO			= 0x59,
	TAPI_NETWORK_HOME_ZONE_IND		= 0x5A,
	TAPI_NETWORK_NITZ_INFO_IND		= 0x5B,
};

enum TAPI_SERVICE_LEVEL
{
	TAPI_SERVICE_LEVEL_NONE 		= 1, 
	TAPI_SERVICE_LEVEL_EMERGENCY 		= 2, 
	TAPI_SERVICE_LEVEL_FULL 		= 3, 
	TAPI_SERVICE_LEVEL_SEARCHING 		= 4,
};
 
#endif

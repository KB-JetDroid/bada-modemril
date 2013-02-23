/**
 * This file is part of libsamsung-ipc.
 *
 * Copyright (C) 2012 Dominik Marszk <dmarszk@gmail.com>
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

#ifndef __DEVICE_WAVE_CALL_H__
#define __DEVICE_WAVE_CALL_H__

/**
 * All the TAPI call context structures specific to Wave will be defined here
 */
enum TAPI_CALL_TYPE
{
	TAPI_CALL_APIREQ 	= 0x0,
	TAPI_CALL_ANSWER	= 0x2,
	TAPI_CALL_RELEASE	= 0x3,
	TAPI_CALL_INCOMING_IND  = 0x14,
	TAPI_CALL_END_IND	= 0x18,
};
 
#endif

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

void tapi_call_handler(uint16_t tapiCallType, uint32_t tapiCallLength, uint8_t *tapiCallData);

#endif

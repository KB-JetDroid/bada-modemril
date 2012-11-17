/**
 * This file is part of libsamsung-ipc.
 *
 * Copyright (C) 2010-2011 Joerie de Gram <j.de.gram@gmail.com>
 *
 * Modified for Jet & Wave - 	KB <kbjetdroid@gmail.com>
 *								Dominik Marszk <dmarszk@gmail.com>
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
 
#ifndef _WAVE_IPC_H_
#define _WAVE_IPC_H_

#include <radio.h>

#define MODEMCTL_PATH			"/dev/modem_ctl"

#define FRAME_START	0x7f
#define FRAME_END	0x7e

struct hdlc_header {
	uint16_t length;
	uint8_t unknown;

	struct ipc_header ipc;
} __attribute__((__packed__));

struct multiPacketHeader {
	uint32_t command;
	uint32_t packtLen;
	uint32_t packetType;
};


#endif


/**
 * This file is part of libsamsung-ipc.
 *
 * Copyright (C) 2013 Dominik Marszk <dmarszk@gmail.com>
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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <radio.h>
#include <sound.h>

#define LOG_TAG "Mocha-RIL-SOUND"
#include <utils/Log.h>

void ipc_parse_sound(struct ipc_client* client, struct modem_io *ipc_frame)
{
	soundPacket *packet;

    packet = (soundPacket*)(ipc_frame->data);
	DEBUG_I("Sound packet type = 0x%x\n  Total packet length = 0x%X", packet->buffer[0], ipc_frame->datasize);
	ipc_hex_dump(client, ipc_frame->data, ipc_frame->datasize);	
}

/**
 * This file is part of libsamsung-ipc.
 *
 * Copyright (C) 2011 KB <kbjetdroid@gmail.com>
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

#ifndef __PROTO_H__
#define __PROTO_H__

#include <radio.h>

#define PROTO_PACKET_ID_STARTUP 			1
#define PROTO_PACKET_ID_CLEANUP 			2
#define PROTO_PACKET_ID_STARTNETWORK 		3
#define PROTO_PACKET_ID_STOPNETWORK 		4
#define PROTO_PACKET_ID_STARTINGNETWORKIND 	5
#define PROTO_PACKET_ID_STARTNETWORKCNF 	6
#define PROTO_PACKET_ID_STARTNETWORKIND 	7
#define PROTO_PACKET_ID_STOPNETWORKCNF 		8
#define PROTO_PACKET_ID_STOPNETWORKIND 		9
#define PROTO_PACKET_ID_SUSPENDNETWORKIND 	10
#define PROTO_PACKET_ID_RESUMENETWORKIND 	11

struct protoPacketHeader {
	uint16_t type;
	uint16_t apiId;
} __attribute__((__packed__));

struct protoPacket {
	struct protoPacketHeader header;
	uint8_t *buf;
	uint32_t bufLen; //this can't be in header because framebuffer shouldn't contain it
} __attribute__((__packed__));

void ipc_parse_proto(struct ipc_client* client, struct modem_io *ipc_frame);

void proto_send_packet(struct protoPacket* protoReq);
void proto_startup(void);

#endif

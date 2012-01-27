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

#ifndef __SIM_H__
#define __SIM_H__

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

/*
 * FIXME: incorrect proto packet header
 */
struct simPacketHeader {
	uint32_t type;
	uint32_t subtype;
	uint32_t buflen;
} __attribute__((__packed__));

struct simRequest {
	struct simPacketHeader header;
	uint8_t *respBuf;
} __attribute__((__packed__));

struct oemSimPacketHeader
{
	uint32_t oemType;
	uint8_t packetSubType;
	uint32_t oemBufLen;
} __attribute__((__packed__));

struct oemSimRequest
{
	struct oemSimPacketHeader header;
	uint8_t *oemBuf;
} __attribute__((__packed__));

void modem_response_sim(struct ipc_client *client, struct modem_io *resp);

#endif

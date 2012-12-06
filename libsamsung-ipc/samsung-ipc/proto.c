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
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <getopt.h>

#include <radio.h>
#include <proto.h>

#define LOG_TAG "Mocha-RIL-PRO"
#include <utils/Log.h>

/*
 * TODO: Implement handling of all the Proto packets
 *
 */

void ipc_parse_proto(struct ipc_client* client, struct modem_io *ipc_frame)
{
	DEBUG_I("Inside ipc_parse_proto - TBD");
	int32_t retval, count;
	struct protoPacketHeader *rx_header;

	struct modem_io request;
    void *frame;
    uint8_t *payload;
    uint32_t frame_length;

    struct fifoPacketHeader *fifoHeader;

	DEBUG_I("Frame header = 0x%x\n Frame type = 0x%x\n Frame length = 0x%x", ipc_frame->magic, ipc_frame->cmd, ipc_frame->datasize);

	ipc_hex_dump(client, ipc_frame->data, ipc_frame->datasize);

    rx_header = (struct protoPacketHeader *)(ipc_frame->data);

    switch (rx_header->type)
    {
	case PROTO_PACKET_ID_STARTUP:
		DEBUG_I("PROTO_PACKET_ID_STARTUP packet received");
		break;
	case PROTO_PACKET_ID_CLEANUP:
		DEBUG_I("PROTO_PACKET_ID_CLEANUP packet received");
		break;
	case PROTO_PACKET_ID_STARTNETWORK:
		DEBUG_I("PROTO_PACKET_ID_STARTNETWORK packet received");
		break;
	case PROTO_PACKET_ID_STOPNETWORK:
		DEBUG_I("PROTO_PACKET_ID_STOPNETWORK packet received");
		break;
	case PROTO_PACKET_ID_STARTINGNETWORKIND:
		DEBUG_I("PROTO_PACKET_ID_STARTINGNETWORKIND packet received");
		break;
	case PROTO_PACKET_ID_STARTNETWORKCNF:
		DEBUG_I("PROTO_PACKET_ID_STARTNETWORKCNF packet received");
		break;
	case PROTO_PACKET_ID_STARTNETWORKIND:
		DEBUG_I("PROTO_PACKET_ID_STARTNETWORKIND packet received");
		break;
	case PROTO_PACKET_ID_STOPNETWORKCNF:
		DEBUG_I("PROTO_PACKET_ID_STOPNETWORKCNF packet received");
		break;
	case PROTO_PACKET_ID_STOPNETWORKIND:
		DEBUG_I("PROTO_PACKET_ID_STOPNETWORKIND packet received");
		break;
	case PROTO_PACKET_ID_SUSPENDNETWORKIND:
		DEBUG_I("PROTO_PACKET_ID_SUSPENDNETWORKIND packet received");
		break;
	case PROTO_PACKET_ID_RESUMENETWORKIND:
		DEBUG_I("PROTO_PACKET_ID_RESUMENETWORKIND packet received");
		break;
	default :
    	DEBUG_I("Unknown Proto Packet");
    	break;
    }

    DEBUG_I("Inside ipc_parse_proto leaving");

}

void proto_send_packet(struct protoPacket* protoReq)
{
	struct modem_io request;
	
	uint32_t bufLen = protoReq->bufLen + sizeof(struct protoPacketHeader);
	uint8_t* fifobuf = malloc(bufLen);
	memcpy(fifobuf, protoReq, sizeof(struct protoPacketHeader));
	if(protoReq->bufLen)
		memcpy(fifobuf + sizeof(struct protoPacketHeader), protoReq->buf, protoReq->bufLen);

	request.magic = 0xCAFECAFE;
	request.cmd = FIFO_PKT_PROTO;
	request.datasize = bufLen;

	request.data = fifobuf;

	ipc_send(&request);

	free(fifobuf);
}

void proto_startup(void)
{
	struct protoPacket pkt;
	pkt.header.type = PROTO_PACKET_ID_STARTUP;
	pkt.header.apiId = 0;
	pkt.buf = NULL;
	pkt.bufLen = 0;
	proto_send_packet(&pkt);
}

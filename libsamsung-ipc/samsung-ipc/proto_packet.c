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
#include <proto_packet.h>
#include "./device/jet/jet_modem_ctl.h"

/*
 * TODO: Implement handling of all the Proto packets
 *
 */

void modem_response_proto(struct ipc_client *client, struct modem_io *resp)
{
	DEBUG_I("Inside modem_response_proto - TBD\n");
	int32_t retval, count;
	struct protoPacketHeader *rx_header;
	struct protoRequest tx_packet;

	struct modem_io request;
    void *frame;
    uint8_t *payload;
    uint32_t frame_length;

    struct fifoPacketHeader *fifoHeader;

	DEBUG_I("Frame header = 0x%x\n Frame type = 0x%x\n Frame length = 0x%x\n", resp->magic, resp->cmd, resp->datasize);

	hexdump(resp->data, resp->datasize);

    rx_header = (struct protoPacketHeader *)(resp->data);

    switch (rx_header->type)
    {
	case PROTO_PACKET_ID_STARTUP:
		DEBUG_I("PROTO_PACKET_ID_STARTUP packet received\n");
		break;
	case PROTO_PACKET_ID_CLEANUP:
		DEBUG_I("PROTO_PACKET_ID_CLEANUP packet received\n");
		break;
	case PROTO_PACKET_ID_STARTNETWORK:
		DEBUG_I("PROTO_PACKET_ID_STARTNETWORK packet received\n");
		break;
	case PROTO_PACKET_ID_STOPNETWORK:
		DEBUG_I("PROTO_PACKET_ID_STOPNETWORK packet received\n");
		break;
	case PROTO_PACKET_ID_STARTINGNETWORKIND:
		DEBUG_I("PROTO_PACKET_ID_STARTINGNETWORKIND packet received\n");
		break;
	case PROTO_PACKET_ID_STARTNETWORKCNF:
		DEBUG_I("PROTO_PACKET_ID_STARTNETWORKCNF packet received\n");
		break;
	case PROTO_PACKET_ID_STARTNETWORKIND:
		DEBUG_I("PROTO_PACKET_ID_STARTNETWORKIND packet received\n");
		break;
	case PROTO_PACKET_ID_STOPNETWORKCNF:
		DEBUG_I("PROTO_PACKET_ID_STOPNETWORKCNF packet received\n");
		break;
	case PROTO_PACKET_ID_STOPNETWORKIND:
		DEBUG_I("PROTO_PACKET_ID_STOPNETWORKIND packet received\n");
		break;
	case PROTO_PACKET_ID_SUSPENDNETWORKIND:
		DEBUG_I("PROTO_PACKET_ID_SUSPENDNETWORKIND packet received\n");
		break;
	case PROTO_PACKET_ID_RESUMENETWORKIND:
		DEBUG_I("PROTO_PACKET_ID_RESUMENETWORKIND packet received\n");
		break;
	default :
    	DEBUG_I("Unknown Proto Packet\n");
    	break;
    }

    DEBUG_I("Inside modem_response_proto leaving\n");

}

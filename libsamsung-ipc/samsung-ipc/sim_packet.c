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
#include <sim_packet.h>

/*
 * TODO: Implement handling of all the SIM packets
 *
 */

void modem_response_sim(struct ipc_client *client, struct modem_io *resp)
{
	DEBUG_I("Entering\n");
	int32_t retval, count;
	struct simPacketHeader *rx_header;
	struct simRequest sim_packet;

	struct modem_io request;
    void *frame;
    uint8_t *payload;
    uint32_t frame_length;

    struct fifoPacketHeader *fifoHeader;

	DEBUG_I("Frame header = 0x%x\n Frame type = 0x%x\n Frame length = 0x%x\n", resp->magic, resp->cmd, resp->datasize);

	hexdump(resp->data, resp->datasize);

    rx_header = (struct simPacketHeader *)(resp->data);
    sim_packet.respBuf = (uint8_t *)(resp->data + sizeof(struct simPacketHeader));

	DEBUG_I("Sim Packet type = 0x%x\n Sim Packet sub-type = 0x%x\n Sim Packet length = 0x%x\n", rx_header->type, rx_header->subtype, rx_header->buflen);

    switch (rx_header->type)
    {
	case 0x00:
		DEBUG_I("SIM_PACKET type 0x00 received\n");
		struct oemSimPacketHeader *oem_header;
		struct oemSimRequest oem_packet;

		oem_header = (struct oemSimPacketHeader *)(sim_packet.respBuf);
		oem_packet.oemBuf = (uint8_t *)(sim_packet.respBuf + sizeof(struct oemSimPacketHeader));

		DEBUG_I("Sim oem type = 0x%x\n Sim Packet sub-type = 0x%x\n Oem length = 0x%x\n", oem_header->oemType, oem_header->packetSubType, oem_header->oemBufLen);

		hexdump(oem_packet.oemBuf, oem_header->oemBufLen);

		break;
	default :
    	DEBUG_I("Unknown SIM Packet\n");
    	break;
    }

    DEBUG_I("leaving\n");

}

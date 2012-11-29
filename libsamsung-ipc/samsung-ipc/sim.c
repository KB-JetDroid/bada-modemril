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
#include <sim.h>

#define LOG_TAG "RIL-SIM"
#include <utils/Log.h>

/*
 * TODO: Implement handling of all the SIM packets
 *
 */

void ipc_parse_sim(struct ipc_client* client, struct modem_io *ipc_frame)
{
	DEBUG_I("Entering");
	int32_t retval, count;
	uint32_t sid;
	struct simPacketHeader *simHeader;
	struct simPacket sim_packet;

	struct modem_io request;
    void *frame;
    uint8_t *payload;
    uint32_t frame_length;

    struct fifoPacketHeader *fifoHeader;

	DEBUG_I("Frame header = 0x%x\n Frame type = 0x%x\n Frame length = 0x%x", ipc_frame->magic, ipc_frame->cmd, ipc_frame->datasize);

	ipc_hex_dump(client, ipc_frame->data, ipc_frame->datasize);

    simHeader = (struct simPacketHeader *)(ipc_frame->data);
    sim_packet.simBuf = (uint8_t *)(ipc_frame->data + sizeof(struct simPacketHeader));

	DEBUG_I("Sim Packet type = 0x%x\n Sim Packet sub-type = 0x%x\n Sim Packet length = 0x%x", simHeader->type, simHeader->subType, simHeader->bufLen);

	if(simHeader->type != 0)
	{
		switch (simHeader->subType)
		{
		case 0x00:
			DEBUG_I("SIM_PACKET OemSimAtkInjectDisplayTextInd rcvd");
			
			/*struct oemSimPacketHeader *oem_header;
			struct oemSimPacket oem_packet;

			oem_header = (struct oemSimPacketHeader *)(sim_packet.respBuf);
			oem_packet.oemBuf = (uint8_t *)(sim_packet.respBuf + sizeof(struct oemSimPacketHeader));

			DEBUG_I("Sim oem type = 0x%x\n Sim Packet sub-type = 0x%x\n Oem length = 0x%x", oem_header->oemType, oem_header->packetSubType, oem_header->oemBufLen);

			ipc_hex_dump(oem_packet.oemBuf, oem_header->oemBufLen);
*/
			break;
		default :
			DEBUG_I("Unknown SIM subType %d", simHeader->subType);
			break;
		}
	}
	else
	{
		if(simHeader->subType >= SIM_SUBTYPE_DIFF)
		{
			diffedSubtype = simHeader->subType - SIM_SUBTYPE_DIFF;
			if(diffedSubtype >= SIM_SUBTYPE_DIFF)
			{
				//do_nothing
			}
			else
			{
				switch(diffedSubtype)
				{
					case 1:
					case 2: //in this subtype
						//TODO: these 2 subtypes are somewhat special - apps does switch some bool if they are used, not sure what way they are special.
						sim_parse_event(sim_packet.simBuf, simHeader->bufLen); //sid is stored in buf too
						break;
					default:
						sim_parse_event(sim_packet.simBuf, simHeader->bufLen); //sid is stored in buf too
						break;
				}
			}
		}
		else
		{
			sim_send_oem_req(sim_packet.simBuf, simHeader->bufLen); //bounceback packet
		}
	}

    DEBUG_I("leaving");

}

void sim_parse_event(uint8_t* buf, uint32_t bufLen)
{

}

void sim_send_oem_req(uint8_t* simBuf, uint8_t simBufLen)
{	
	//simBuf is expected to contain full oemPacket structure
	struct modem_io request;
	struct simPacket sim_packet;	
	sim_packet.header.type = 0;
	sim_packet.header.subType = ((struct oemSimPacketHeader *)(simBuf))->type;
	sim_packet.header.bufLen = simBufLen;
	sim_packet.simBuf = simBuf;
	
	uint32_t bufLen = sim_packet.header.bufLen + sizeof(struct simPacketHeader);
	uint8_t* fifobuf = malloc(bufLen);
	memcpy(fifobuf, &sim_packet.header, sizeof(struct simPacketHeader));
	memcpy(fifobuf + sizeof(struct simPacketHeader), sim_packet.simBuf, sim_packet.header.bufLen);

	request.magic = 0xCAFECAFE;
	request.cmd = FIFO_PKT_SIM;
	request.datasize = bufLen;

	request.data = fifobuf;

	ipc_send(&request);

	free(fifobuf);
}

void sim_send_oem_data(uint8_t hSim, uint8_t packetType, uint8_t* dataBuf, uint32_t oemBufLen)
{	
	SIM_VALIDATE_SID(hSim);

	struct oemSimPacketHeader oem_header;	
	oem_header.type = packetType;
	oem_header.hSim = hSim; //session id
	oem_header.oemBufLen = oemBufLen;
	

	uint32_t simBufLen = oemBufLen + sizeof(struct oemSimPacketHeader);
	uint8_t* simBuf = malloc(simBufLen);
	memcpy(simBuf, &(oem_header), sizeof(struct oemSimPacketHeader));
	if(oemBufLen)
		memcpy(simBuf + sizeof(struct oemSimPacketHeader), dataBuf, oemBufLen);
	
	sim_send_oem_req(simBuf, simBufLen);
	free(simBuf);
}

void sim_verify_chv(uint8_t hSim, uint8_t pinType, char* pin)
{	
	SIM_VALIDATE_SID(hSim);
	//TODO: obtain session context, check if session is busy, print exception if it is busy and return failure
	//TODO: if session is not busy, mark it busy
	uint8_t* packetBuf = malloc(10);	
	memset(packetBuf, 0x00, 10);

	packetBuf[0] = pinType;
	memcpy(packetBuf+1, pin, strlen(pin)); //max pin len is 9 digits
	sim_send_oem_data(hSim, 0xB, packetBuf, 10);
}

void sim_atk_open(uint32_t sid)
{
	//TODO: verify ATK session and create/open it and return handler to it?!
	DEBUG_I("Sending");
	sim_send_oem_data(0xA, 0x1B, NULL, 0); //0xA hSim is hardcoded in bada
}

void sim_open_to_modem(uint8_t hSim)
{
	//TODO: verify, create and initialize session, send real hSim
	DEBUG_I("Sending");
	sim_send_oem_data(0x4, 0x1, NULL, 0); //why it starts from 4? hell knows
}

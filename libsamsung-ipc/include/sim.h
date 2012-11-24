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

#include <radio.h>

#if defined(DEVICE_JET)
#define SESSION_SUBTYPE_DIFF 0x18
#elif defined(DEVICE_WAVE)
#define SESSION_SUBTYPE_DIFF 0x1D
#endif

#define SIM_SESSION_COUNT 0x20
#define SIM_SESSION_START_ID 0
#define SIM_SESSION_END_ID (SIM_SESSION_START_ID+(SIM_SESSION_COUNT-1))

#define SIM_VALIDATE_SID(hSim) {if(!hSim) {DEBUG_E("SIM_VALIDATE_SID failure!\n"); return -1;}}
struct simPacketHeader {
	uint32_t type;
	uint32_t subType;
	uint32_t bufLen;
} __attribute__((__packed__));

struct simPacket {
	struct simPacketHeader header;
	uint8_t *simBuf;
} __attribute__((__packed__));

struct oemSimPacketHeader{
	uint32_t hSim; //not sure if its really session_id
	uint8_t type; //equal to parent packet subtype
	uint32_t oemBufLen;
} __attribute__((__packed__));

struct oemSimPacket{
	struct oemSimPacketHeader header;
	uint8_t *oemBuf;
} __attribute__((__packed__));

void ipc_parse_sim(struct ipc_client* client, struct modem_io *ipc_frame);
void sim_parse_session_event(uint8_t* buf, uint32_t bufLen);

int sim_send_oem_req(uint8_t* simBuf, uint8_t simBufLen);
int sim_send_oem_data(uint8_t hSim, uint8_t packetType, uint8_t* dataBuf, uint32_t oemBufLen);

int sim_verify_chv(uint8_t hSim, uint8_t pinType, char* pin);
int sim_atk_open(uint32_t sid);
int sim_open_to_modem(uint8_t hSim);
#endif

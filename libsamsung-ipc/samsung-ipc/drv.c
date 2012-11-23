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
#include <drv.h>

#define LOG_TAG "RIL-IP"
#include <utils/Log.h>

/*
 * TODO: Implement handling of all the IpcDrv packets
 *
 */

#ifdef DEVICE_JET
#include <device/jet/sound_data.h>
char *nvmFile = "/efs/bml4";
char *fwVersion = "S800MPOJB1";
#elif defined(DEVICE_WAVE)
#include <device/wave/sound_data.h>
char *nvmFile = "/dev/mtdblock0";
char *fwVersion = "S8500JPKA1";
#endif


/*
 * TODO: Read sound config data from file
 */


int32_t get_nvm_data(void *data, uint32_t size)
{
	int32_t fd, retval;
	fd = open(nvmFile, O_RDONLY);

	DEBUG_I("file %s open status = %d", nvmFile, fd);
	retval = read(fd, data, size);
	DEBUG_I("file %s read status = %d", nvmFile, retval);

	if (fd > 0)
		close(fd);
	return 0;
}

void handleReadNvRequest(struct drvNvPacket* rxNvPacket)
{
	struct modem_io request;
	struct drvRequest tx_packet;

	DEBUG_I("size = 0x%x", rxNvPacket->size);

	tx_packet.header.drvPacketType = NV_BACKUP_DATA;
	tx_packet.header.reserved = 0;
	tx_packet.respBuf = NULL;

	request.data = malloc((rxNvPacket->size) + sizeof(struct drvPacketHeader));

	memcpy(request.data, &tx_packet, sizeof(struct drvPacketHeader));

	get_nvm_data(request.data + sizeof(struct drvPacketHeader), rxNvPacket->size);

	request.magic = 0xCAFECAFE;
	request.cmd = FIFO_PKT_DRV;
	request.datasize = rxNvPacket->size +  sizeof(struct drvPacketHeader);

	ipc_send(&request);
	free(request.data);
}

#if defined(DEVICE_JET)
void handleJetPmicRequest(struct modem_io *resp)
{
    uint8_t *payload;
	int32_t retval;
	uint8_t params[3];
	struct drvPMICPacket *pmic_packet;

	pmic_packet = (struct drvPMICPacket *)(resp->data);

	DEBUG_I("PMIC value = 0x%x", pmic_packet->value);

	params[0] = 1;
	params[1] = 0x9B; //SIMLTTV;
	if (pmic_packet->value >= 0x7D0)
		params[2] = 0x2D;
	else
		params[2] = 0x15;

	retval = ipc_modem_io(params, IOCTL_MODEM_PMIC);

	DEBUG_I("ioctl return value = 0x%x", retval);

	ipc_send(resp);
}
#endif

void handleSystemInfoRequest()
{
    uint8_t *payload;
	struct drvRequest tx_packet;
	struct modem_io request;
	
	DEBUG_I("enter");
	
	/* TODO: for WAVE add USB TA info sending here */
	tx_packet.header.drvPacketType = SOUND_CONFIG;
	tx_packet.header.reserved = 0;
	tx_packet.respBuf = NULL;

	payload = malloc(SOUND_CFG_DATA_SIZE + sizeof(struct drvPacketHeader));

	memcpy(payload, &tx_packet, sizeof(struct drvPacketHeader));

	request.magic = 0xCAFECAFE;
	request.cmd = FIFO_PKT_DRV;
	request.datasize = SOUND_CFG_DATA_SIZE + sizeof(struct drvPacketHeader);

	memcpy(payload + sizeof(struct drvPacketHeader), RCV_MSM_Data, sizeof(RCV_MSM_Data));

	request.data = payload;

	ipc_send(&request);

	memcpy(payload + sizeof(struct drvPacketHeader), EAR_MSM_Data, sizeof(EAR_MSM_Data));

	request.data = payload;

	ipc_send(&request);

	memcpy(payload + sizeof(struct drvPacketHeader), SPK_MSM_Data, sizeof(SPK_MSM_Data));

	request.data = payload;

	ipc_send(&request);

	memcpy(payload + sizeof(struct drvPacketHeader), BTH_MSM_Data, sizeof(BTH_MSM_Data));

	request.data = payload;

	ipc_send(&request);
	free(payload);

	tx_packet.header.drvPacketType = HIDDEN_SW_VER;
	tx_packet.header.reserved = 0;
	tx_packet.respBuf = NULL;

	payload = malloc((0x14) + sizeof(struct drvPacketHeader));

	memcpy(payload, &tx_packet, sizeof(struct drvPacketHeader));

	request.magic = 0xCAFECAFE;
	request.cmd = FIFO_PKT_DRV;
	request.datasize = 0x14 +  sizeof(struct drvPacketHeader);

	memcpy(payload + sizeof(struct drvPacketHeader), fwVersion, sizeof(*fwVersion));

	request.data = payload;

	ipc_send(&request);
	free(payload);

	DEBUG_I("Sent all the sound packages");
}

void modem_response_drv(struct modem_io *resp)
{
	DEBUG_I("enter");
	int32_t retval;
	struct drvPacketHeader *rx_header;

	DEBUG_I("Frame header = 0x%x\n Frame type = 0x%x\n Frame length = 0x%x", resp->magic, resp->cmd, resp->datasize);

	hexdump(resp->data, resp->datasize);

    rx_header = (struct drvPacketHeader *)(resp->data);

	DEBUG_I("Packet type = 0x%x", rx_header->drvPacketType);

    switch (rx_header->drvPacketType)
    {
	case READ_NV_BACKUP:
		DEBUG_I("ReadNvBackup IpcDrv packet received");
		handleReadNvRequest((struct drvNvPacket *)(resp->data));
		break;
#if defined (DEVICE_JET)
	case PMIC_PACKET:
		DEBUG_I("PMIC IpcDrv packet received");
		handleJetPmicRequest(resp);
		break;
#endif
	case SYSTEM_INFO_REQ:
		DEBUG_I("SYSTEM_INFO_REQ IpcDrv packet received");
		handleSystemInfoRequest();
		break;
	default:
		DEBUG_I("IpcDrv Packet type 0x%x is not yet handled", rx_header->drvPacketType);

		break;
    }

    DEBUG_I("exit");

}

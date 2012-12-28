/**
 * This file is part of libsamsung-ipc.
 *
 * Copyright (C) 2011 KB <kbjetdroid@gmail.com>
 * Copyright (C) 2012 Dominik Marszk <dmarszk@gmail.com>
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
#include <errno.h>

#define LOG_TAG "Mocha-RIL-IPC_DRV"
#include <utils/Log.h>

/*
 * TODO: Implement handling of all the IpcDrv packets
 *
 */

#ifdef DEVICE_JET
#include <device/jet/sound_data.h>
char *nvmFile = "/efs/bml4";
char *batteryDev = "/sys/devices/platform/i2c-gpio.6/i2c-6/6-0066/max8998-charger/power_supply/"; 
char *fake_apps_version = "S800MPOJB1";
#elif defined(DEVICE_WAVE)
#include <device/wave/sound_data.h>
char *nvmFile = "/dev/mtdblock0";
char *batteryDev = "/sys/devices/platform/i2c-gpio.6/i2c-6/6-0066/max8998-charger/power_supply/"; 
char *fake_apps_version = "S8530JPKA1";
#endif


/*
 * TODO: Read sound config data from file
 */


int32_t get_nvm_data(void *data, uint32_t size)
{
	int32_t fd, retval;
	fd = open(nvmFile, O_RDONLY);

	DEBUG_I("file %s open status = %d", nvmFile, fd);
	if(fd < 0)
	{
		DEBUG_I("%s: error! %s", __func__, strerror(errno));
		return -1;
	}
	
	retval = read(fd, data, size);
	DEBUG_I("file %s read status = %d", nvmFile, retval);
	if(retval < 0)
		DEBUG_I("%s: error! %s", __func__, strerror(errno));
		
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
	//tx_packet.header.reserved = 0;
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
void handleJetPmicRequest(struct modem_io *ipc_frame)
{
    uint8_t *payload;
	int32_t retval;
	uint8_t params[3];
	struct drvPMICPacket *pmic_packet;

	pmic_packet = (struct drvPMICPacket *)(ipc_frame->data);

	DEBUG_I("PMIC value = 0x%x", pmic_packet->value);

	params[0] = 1;
	params[1] = 0x9B; //SIMLTTV;
	if (pmic_packet->value >= 0x7D0)
		params[2] = 0x2D;
	else
		params[2] = 0x15;

	retval = ipc_modem_io(params, IOCTL_MODEM_PMIC);

	DEBUG_I("ioctl return value = 0x%x", retval);

	ipc_send(ipc_frame);
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
	//tx_packet.header.reserved = 0;
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
	//tx_packet.header.reserved = 0;
	tx_packet.respBuf = NULL;

	payload = malloc((0x14) + sizeof(struct drvPacketHeader));

	memcpy(payload, &tx_packet, sizeof(struct drvPacketHeader));

	request.magic = 0xCAFECAFE;
	request.cmd = FIFO_PKT_DRV;
	request.datasize = 0x14 +  sizeof(struct drvPacketHeader);

	memcpy(payload + sizeof(struct drvPacketHeader), fake_apps_version, sizeof(*fake_apps_version));

	request.data = payload;

	ipc_send(&request);
	free(payload);

	DEBUG_I("Sent all the sound packages");
}

void handleFuelGaugeStatus(uint8_t percentage)
{
	char buf[60];
	int32_t fd, len;
	sprintf(buf, "%s/capacity", batteryDev);
	fd = open(buf, O_RDWR);
	if(fd < 0)
	{
		DEBUG_E("%s: Failed to open %s.", __func__, buf);
		return;
	}
	sprintf(buf, "%d", percentage);
	len = strlen(buf);
	if(write(fd, buf, strlen(buf)) != len)	
		DEBUG_E("%s: Failed to write battery capacity, error: %s", __func__, strerror(errno));
	close(fd);
}

void ipc_parse_drv(struct ipc_client* client, struct modem_io *ipc_frame)
{
	DEBUG_I("enter");
	int32_t retval;
	struct drvPacketHeader *rx_header;

	DEBUG_I("Frame header = 0x%x\n Frame type = 0x%x\n Frame length = 0x%x", ipc_frame->magic, ipc_frame->cmd, ipc_frame->datasize);

	ipc_hex_dump(client, ipc_frame->data, ipc_frame->datasize);

    rx_header = (struct drvPacketHeader *)(ipc_frame->data);

	DEBUG_I("Packet type = 0x%x", rx_header->drvPacketType);

    switch (rx_header->drvPacketType)
    {
	case READ_NV_BACKUP:
		DEBUG_I("ReadNvBackup IpcDrv packet received");
		handleReadNvRequest((struct drvNvPacket *)(ipc_frame->data));
		break;
#if defined (DEVICE_JET)
	case PMIC_PACKET:
		DEBUG_I("PMIC IpcDrv packet received");
		handleJetPmicRequest(ipc_frame);
		break;
#endif
	case SYSTEM_INFO_REQ:
		DEBUG_I("SYSTEM_INFO_REQ IpcDrv packet received");
		handleSystemInfoRequest();
		break;
	case BATT_GAUGE_STATUS_CHANGE_IND:	
		DEBUG_I("BATT_GAUGE_STATUS_CHANGE_IND IpcDrv packet received");
		handleFuelGaugeStatus(*((uint8_t*)ipc_frame->data));
		break;
	default:
		DEBUG_I("IpcDrv Packet type 0x%X is not yet handled", rx_header->drvPacketType);

		break;
    }

    DEBUG_I("exit");

}

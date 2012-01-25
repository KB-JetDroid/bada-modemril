/**
 * This file is part of libsamsung-ipc.
 *
 * Copyright (C) 2010-2011 Joerie de Gram <j.de.gram@gmail.com>
 *
 * Modified for Jet - KB <kbjetdroid@gmail.com>
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

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include <radio.h>

#include "ipc_private.h"
#include "jet_ipc.h"

#define isprint(c)	((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
void hexdump(const char *buf, int len)
{
	char str[80], octet[10];
	int ofs, i, l;

	for (ofs = 0; ofs < len; ofs += 16) {
		sprintf( str, "0x%02x: ", ofs );

		for (i = 0; i < 16; i++) {
			if ((i + ofs) < len)
				sprintf( octet, "%02x ", buf[ofs + i] );
			else
				strcpy( octet, "   " );

			strcat( str, octet );
		}
			strcat( str, "  " );
			l = strlen( str );

		for (i = 0; (i < 16) && ((i + ofs) < len); i++)
			str[l++] = isprint( buf[ofs + i] ) ? buf[ofs + i] : '.';

		str[l] = '\0';
		printf( "%s\n", str );
	}
}

int jet_modem_bootstrap(struct ipc_client *client)
{

	int rc = 0;

    int dpram_fd = -1;


	int fd, retval;

    printf("jet_ipc_bootstrap: enter\n");

    printf("jet_ipc_bootstrap: open modem_ctl\n");

    dpram_fd = open(DPRAM_TTY, O_RDWR | O_NDELAY);

    if(dpram_fd < 0) {
    	printf("jet_ipc_bootstrap: failed to open dev/dpram0\n");
    	return 1;
    }

    printf("jet_ipc_bootstrap: send amss_run_request\n");

    ioctl(dpram_fd, IOCTL_MODEM_AMSSRUNREQ);

    printf("jet_ipc_bootstrap: closing dev/modem_ctl\n");

    close(dpram_fd);

    printf("jet_ipc_bootstrap: exit\n");

    return 0;
}

int jet_ipc_open(void *data, unsigned int size, void *io_data)
{
    struct termios termios;

    int fd = -1;

    if(io_data == NULL)
        return -1;

    fd = *((int *) io_data);

    fd = open(DPRAM_TTY, O_RDWR);

    printf("dpram fd = 0x%x\n", fd);

    if(fd < 0) {
        return 1;
    }

    tcgetattr(fd, &termios);
    cfmakeraw(&termios);
    tcsetattr(fd, TCSANOW, &termios);

    memcpy(io_data, &fd, sizeof(int));

    return 0;
}

int jet_ipc_close(void *io_data)
{
    int fd = -1;

    if(io_data == NULL)
        return -1;

    fd = *((int *) io_data);

    if(fd) {
        return close(fd);
    }

    return 0;
}

int jet_ipc_power_on(void *data)
{
    int fd = -1;

    if(data == NULL)
        return -1;

    fd = *((int *) data);

    ioctl(fd, IOCTL_PHONE_ON);

    return 0;
}

int jet_ipc_power_off(void *data)
{
    int fd = -1;

    if(data == NULL)
        return -1;

    fd = *((int *) data);

    ioctl(fd, IOCTL_PHONE_OFF);

    return 0;
}

int send_packet(struct ipc_client *client, struct modem_io *request)
{
	int retval;
    struct fifoPacketHeader *ipc;
    unsigned char *frame;
    unsigned char *payload;
    int frame_length;

    /* Frame length: FIFO header + payload length */
    frame_length = (sizeof(*ipc) + request->datasize);

    frame = (unsigned char*)malloc(frame_length);

    /* FIFO header */
    ipc = (struct fifoPacketHeader*)(frame);

    ipc->magic = request->magic;
    ipc->cmd = request->cmd;
    ipc->datasize = request->datasize;

    /* FIFO payload */
    payload = (frame + sizeof(*ipc));
    memcpy(payload, request->data, request->datasize);

	retval = client->handlers->write(frame, frame_length, client->handlers->write_data);

    free(frame);

    return 0;
}

int jet_ipc_send(struct ipc_client *client, struct modem_io *request)
{
	int left_data;
	struct modem_io multi_request;
	struct multiPacketHeader *multiHeader;

	if (request->datasize > MAX_SIGNLE_FRAME_DATA)
	{
		printf("KB: packet to send is larger than 0x1000\n");

		multi_request.magic = 0xCAFECAFE;
		multi_request.cmd = FIFO_PKT_FIFO_INTERNAL;
		multi_request.datasize = 0x0C;

		multiHeader = (struct multiPacketHeader *)malloc(sizeof(struct multiPacketHeader));

		multiHeader->command = 0x02;
		multiHeader->packtLen = request->datasize;
		multiHeader->packetType = request->cmd;

		multi_request.data = multiHeader;

		send_packet(client, &multi_request);

		left_data = request->datasize;

		multi_request.data = request->data;

		while (left_data > 0)
		{
			if (left_data > MAX_SIGNLE_FRAME_DATA)
			{
				multi_request.datasize = MAX_SIGNLE_FRAME_DATA;
			}
			else
			{
				multi_request.datasize = left_data;
			}

			send_packet(client, &multi_request);

			multi_request.data += MAX_SIGNLE_FRAME_DATA;

			left_data -= MAX_SIGNLE_FRAME_DATA;
		}
	}
	else
	{
		send_packet(client, request);
	}

	return 0;
}

int jet_ipc_recv(struct ipc_client *client, struct modem_io *response)
{
    unsigned char buf[SIZ_PACKET_HEADER];
    unsigned char *data;
    unsigned short *frame_length;
    struct fifoPacketHeader *ipc;
    struct modem_io modem_packet;
    int num_read;
    int left;

    num_read = client->handlers->read((void*)buf, sizeof(buf), client->handlers->read_data);

    ipc = (struct fifoPacketHeader *)buf;

    if(num_read == sizeof(buf) && ipc->magic == 0xCAFECAFE) {

        frame_length = ipc->datasize;
        left = frame_length;

        data = (unsigned char*)malloc(left);
        num_read = client->handlers->read((void*)data, left, client->handlers->read_data);

        if(num_read == left) {
            response->magic = ipc->magic;
            response->cmd = ipc->cmd;
            response->datasize = ipc->datasize;

            response->data = (unsigned char*)malloc(left);
            memcpy(response->data, data , response->datasize);

            return 0;
        }
    }

    return 0;
}

int jet_ipc_read(void *data, unsigned int size, void *io_data)
{
    int fd = -1;

    if(io_data == NULL)
        return -1;

    fd = *((int *) io_data);

    if(fd < 0)
        return -1;

    return read(fd, data, size);
}

int jet_ipc_write(void *data, unsigned int size, void *io_data)
{
    int fd = -1;

    if(io_data == NULL)
        return -1;

    fd = *((int *) io_data);

    if(fd < 0)
        return -1;

    return write(fd, data, size);
}

int jet_modem_operations(struct ipc_client *client, void *data, unsigned int cmd)
{
    int fd = -1;

    fd = *((int *)client->handlers->write_data);

    if(fd < 0)
        return -1;
    printf("KB: modem_operations ioctl cmd = 0x%x\n", cmd);

    return ioctl(fd, cmd, data);
}

struct ipc_handlers ipc_default_handlers = {
    .open = jet_ipc_open,
    .close = jet_ipc_close,
    .power_on = jet_ipc_power_on,
    .power_off = jet_ipc_power_off,
    .read = jet_ipc_read,
    .write = jet_ipc_write,
};

struct ipc_ops ipc_ops = {
    .send = jet_ipc_send,
    .recv = jet_ipc_recv,
    .bootstrap = jet_modem_bootstrap,
    .modem_operations = jet_modem_operations,
};

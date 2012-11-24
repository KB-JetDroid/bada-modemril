/**
 * This file is part of libsamsung-ipc.
 *
 * Copyright (C) 2010-2011 Joerie de Gram <j.de.gram@gmail.com>
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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <asm/types.h>
#include <mtd/mtd-abi.h>

#include <radio.h>
#include "ipc_private.h"


void imei_bcd2ascii(char* out, const char* in)
{
	int i;
	int outi;
	outi = 0;
	/* first byte of in is 0x08, second byte is 0xNA where N is the first digit of imei */
	out[outi++] = (in[1] >> 4)+0x30;
	for(i = 2; i < 9; i++)
	{
		out[outi++] = (in[i] & 0xF) + 0x30;
		out[outi++] = ((in[i] >> 4) & 0xF) + 0x30;
	}
}

void ipc_hex_dump(struct ipc_client *client, void *data, int size)
{
    /* dumps size bytes of *data to stdout. Looks like:
* [0000] 75 6E 6B 6E 6F 77 6E 20
* 30 FF 00 00 00 00 39 00 unknown 0.....9.
* (in a single line of course)
*/

    unsigned char *p = data;
    unsigned char c;
    int n;
    char bytestr[4] = {0};
    char addrstr[10] = {0};
    char hexstr[ 16*3 + 5] = {0};
    char charstr[16*1 + 5] = {0};
    for(n=1;n<=size;n++) {
        if (n%16 == 1) {
            /* store address for this line */
            snprintf(addrstr, sizeof(addrstr), "%.4x",
               ((unsigned int)p-(unsigned int)data) );
        }

        c = *p;
        if (isalnum(c) == 0) {
            c = '.';
        }

        /* store hex str (for left side) */
        snprintf(bytestr, sizeof(bytestr), "%02X ", *p);
        strncat(hexstr, bytestr, sizeof(hexstr)-strlen(hexstr)-1);

        /* store char str (for right side) */
        snprintf(bytestr, sizeof(bytestr), "%c", c);
        strncat(charstr, bytestr, sizeof(charstr)-strlen(charstr)-1);

        if(n%16 == 0) {
            /* line completed */
            ipc_client_log(client, "[%4.4s] %-50.50s %s", addrstr, hexstr, charstr);
            hexstr[0] = 0;
            charstr[0] = 0;
        } else if(n%8 == 0) {
            /* half line: add whitespaces */
            strncat(hexstr, " ", sizeof(hexstr)-strlen(hexstr)-1);
            strncat(charstr, " ", sizeof(charstr)-strlen(charstr)-1);
        }
        p++; /* next byte */
    }

    if (strlen(hexstr) > 0) {
        /* print rest of buffer if not empty */
        ipc_client_log(client, "[%4.4s] %-50.50s %s\n", addrstr, hexstr, charstr);
    }
}

void *ipc_mtd_read(struct ipc_client *client, char *mtd_name, int size, int block_size)
{
    void *mtd_p=NULL;
    uint8_t *data_p=NULL;

    loff_t offs;
    int fd;
    int i;

    if(mtd_name == NULL || size <= 0 || block_size <= 0)
        goto error;

    ipc_client_log(client, "mtd_read: reading 0x%x bytes from %s with 0x%x bytes block size\n", size, mtd_name, block_size);

    fd=open(mtd_name, O_RDONLY);
    if(fd < 0)
        goto error;

    mtd_p=malloc(size);
    if(mtd_p == NULL)
        goto error;

    memset(mtd_p, 0, size);

    data_p=(uint8_t *) mtd_p;

    for(i=0 ; i < size / block_size ; i++)
    {
        offs = i * block_size;
        if(ioctl(fd, MEMGETBADBLOCK, &offs) == 1)
        {
            ipc_client_log(client, "mtd_read: warning: bad block at offset %lld\n", (long long int) offs);
            data_p+=block_size;
            continue;
        }

        read(fd, data_p, block_size);
        data_p+=block_size;
    }

    close(fd);

    return mtd_p;

error:
    ipc_client_log(client, "%s: something went wrong\n", __func__);
    return NULL;
}

void *ipc_file_read(struct ipc_client *client, char *file_name, int size, int block_size)
{
    void *file_p=NULL;
    uint8_t *data_p=NULL;

    int fd;
    int i;

    if(file_name == NULL || size <= 0 || block_size <= 0)
        goto error;

    ipc_client_log(client, "file_read: reading 0x%x bytes from %s with 0x%x bytes block size\n", size, file_name, block_size);

    fd=open(file_name, O_RDONLY);
    if(fd < 0)
        goto error;

    file_p=malloc(size);
    if(file_p == NULL)
        goto error;

    memset(file_p, 0, size);

    data_p=(uint8_t *) file_p;

    for(i=0 ; i < size / block_size ; i++)
    {
        read(fd, data_p, block_size);
        data_p+=block_size;
    }

    close(fd);

    return file_p;

error:
    ipc_client_log(client, "%s: something went wrong\n", __func__);
    return NULL;
}


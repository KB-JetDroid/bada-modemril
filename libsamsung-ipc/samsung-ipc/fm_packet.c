/**
 * This file is part of libsamsung-ipc.
 *
 * Copyright (C) 2011-2012 KB <kbjetdroid@gmail.com>
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

/*
 * TODO: Optimize this file
 * 		 Implement following functions properly:
 * 		 	FmMoveFile
 * 		 	FmGetFileAttrFile
 * 		 	FmFGetFileAttrFile
 * 		 	FmSetFileAttrFile
 * 		 	FmTruncateFile
 * 		 	FmReadDirFile
 * 		 	FmRemoveDirFile
 * 		 	FmGetQuotaSpaceFile
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

#include <fm_packet.h>
#include <radio.h>
#include <dirent.h>
#include <errno.h>

#define MAX_OPEN_DIRS 	10

unsigned int dirArray[MAX_OPEN_DIRS];
unsigned int dirIndex = 1;

char *mochaRoot = "/KFAT0";

int lastOpen = 0;
int lastFile = 0;

int (*fileOps[MAX_FILE_OPS])(struct fmRequest *, struct fmResponse *) =
{
	&FmOpenFile,
	&FmCloseFile,
	&FmCreateFile,
	&FmReadFile,
	&FmWriteFile,
	&FmFlushFile,
	&FmSeekFile,
	&FmTellFile,
	&FmRemoveFile,
	&FmMoveFile,
	&FmGetFileAttrFile,
	&FmFGetFileAttrFile,
	&FmSetFileAttrFile,
	&FmTruncateFile,
	&FmOpenDirFile,
	&FmCloseDirFile,
	&FmReadDirFile,
	&FmCreateDirFile,
	&FmRemoveDirFile,
	&FmGetQuotaSpaceFile
};

int FmOpenFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	printf("KB: Inside FmOpenFile\n");
	int retval = 0;
	char *fName;
	int mode;
	unsigned int flags = O_RDONLY;

	mode = *(int *)(rx_packet->reqBuf);
	fName = (char *)malloc(sizeof(*mochaRoot) + strlen((rx_packet->reqBuf) + sizeof(mode)));
	strcpy(fName, mochaRoot);
	strcat(fName, (const char *)(rx_packet->reqBuf + sizeof(mode)));
	printf("KB: fName %s, mode = 0x%x\n", fName, mode);

	if (!strcmp(fName, "/KFAT0/nvm/num/87_19"))
		lastOpen = 1;

	if(mode & FM_CREATE)
		flags |= O_CREAT;
	if(mode & FM_WRITE)
		flags |= O_RDWR;
	if(mode & FM_TRUNCATE)
		flags |= O_TRUNC;
	if(mode & FM_APPEND)
		flags |= O_APPEND;
#if 0
	else if(mode & FM_NOSHARE)
		flags |= O_RDWR;
	else if(mode & FM_DIRECTIO)
		flags |= O_RDWR;
	else if(mode & FM_CONCRETE_WRITE)
		flags |= O_RDWR;
	else if(mode & FM_NOUPDATE_TIME)
		flags |= O_RDWR;
#endif
	retval = open(fName, flags); //0777);

	tx_packet->funcRet = retval; //-1; //retval;
	tx_packet->errorVal = (retval < 0 ? errno : 0); //-1; //retval;

	tx_packet->header->packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet); //0x08; //0x100;
	tx_packet->respBuf = NULL;

//	printf("KB: Leaving FmOpenFile fd = %d\n", retval);
	return 0;
}

int FmCloseFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	printf("KB: Inside FmCloseFile\n");
	int retval = 0;
	int fd;

	fd = *(int *)(rx_packet->reqBuf);

	retval = close(fd);

	tx_packet->errorVal = (retval < 0 ? errno : 0); //0; //retval;
	tx_packet->funcRet = retval; //0; //retval;

	tx_packet->header->packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet);
	tx_packet->respBuf = NULL;

//	printf("KB: Leaving FmCloseFile fd = %d\n", fd);
	if (lastOpen)
	{
		lastFile = 1;
		lastOpen = 0;
	}
	else
		lastFile = 0;

	return 0;
}

int FmCreateFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	printf("KB: Inside FmCreateFile\n");
	int retval = 0;
	struct stat sb;
	char *fName;

	fName = (char *)malloc(sizeof(*mochaRoot) + strlen(rx_packet->reqBuf));
	strcpy(fName, mochaRoot);
	strcat(fName, (const char *)(rx_packet->reqBuf));
	printf("KB: fName %s\n", fName);

	retval = creat(fName, 0777);

	tx_packet->errorVal = (retval < 0 ? errno : 0); //0; //retval;
	tx_packet->funcRet = retval; //0; //retval;

	tx_packet->header->packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet); //0x08; //0x100;
	tx_packet->respBuf = NULL;

	printf("KB: Leaving FmCreateFile\n");
	return 0;
}

int FmReadFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	printf("KB: Inside FmReadFile\n");
	int retval = 0;
	int fd;
	unsigned int size, numRead;
	char *readBuf;

	fd = *(int *)(rx_packet->reqBuf);
	size = *(int *)((rx_packet->reqBuf) + sizeof(fd));

	readBuf = (char *)malloc(size + sizeof(numRead));

	numRead = read(fd, (readBuf+ sizeof(numRead)), size);

	memcpy(readBuf, &numRead, sizeof(numRead));

	tx_packet->errorVal = ((int)numRead < 0 ? errno : 0); //0; //retval;
	tx_packet->funcRet = numRead; //0; //retval;

	tx_packet->header->packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet) + numRead; //0x08; //0x100;
	tx_packet->respBuf = readBuf;

//	printf("KB: Leaving FmReadFile\n");
	return 0;
}

int FmWriteFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	printf("KB: Inside FmWriteFile\n");
	int retval = 0;
	int fd;
	unsigned int size, numWrite;
	char *writeBuf;

	fd = *(int *)(rx_packet->reqBuf);
	size = *(int *)((rx_packet->reqBuf) + sizeof(fd));

	writeBuf = (char *)((rx_packet->reqBuf) + sizeof(fd) + sizeof(size));

	numWrite = write(fd, writeBuf, size);

	tx_packet->errorVal = ((int) numWrite < 0 ? errno : 0); //0; //retval;
	tx_packet->funcRet = numWrite; //0; //retval;

	tx_packet->header->packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet); //0x08; //0x100;
	tx_packet->respBuf = NULL;

//	printf("KB: Leaving FmWriteFile\n");
	return 0;
}

int FmFlushFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	printf("KB: Inside FmFlushFile\n");
	int retval = 0;
	int fd;

	fd = *(int *)(rx_packet->reqBuf);

	retval = fsync(fd);

	tx_packet->errorVal = 0; //retval;
	tx_packet->funcRet = retval; //0; //retval;

	tx_packet->header->packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet);
	tx_packet->respBuf = NULL;

	printf("KB: Leaving FmFlushFile\n");
	return 0;
}

int FmSeekFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	printf("KB: Inside FmSeekFile\n");
	int retval = 0;
	int fd;
	unsigned int offset, origin;

	fd = *(int *)(rx_packet->reqBuf);
	offset = *(int *)((rx_packet->reqBuf) + sizeof(fd));
	origin = *(int *)((rx_packet->reqBuf) + sizeof(fd) + sizeof(offset));

	printf("KB: Inside FmSeekFile fd = %d, offset = 0x%x, origin = 0x%x\n", fd, offset, origin);

	retval = lseek(fd, offset, origin);

	tx_packet->errorVal = 0; //retval;
	tx_packet->funcRet = 0; //retval;

	tx_packet->header->packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet); //0x08; //0x100;
	tx_packet->respBuf = NULL;

	printf("KB: Leaving FmSeekFile fd = %d\n", fd);
	return 0;
}

int FmTellFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	printf("KB: Inside FmTellFile\n");
	int retval = 0;
	int fd;

	fd = *(int *)(rx_packet->reqBuf);

	retval = lseek(fd, 0, SEEK_CUR); //ftell(fd);

	tx_packet->errorVal = 0; //retval;
	tx_packet->funcRet = retval; //0; //retval;

	tx_packet->header->packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet);
	tx_packet->respBuf = NULL;

	printf("KB: Leaving FmTellFile fd = %d\n", fd);
	return 0;
}

int FmRemoveFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	//printf("KB: Inside FmRemoveFile\n");
	int retval = 0;
	char *fName;

	fName = (char *)malloc(sizeof(*mochaRoot) + strlen(rx_packet->reqBuf));
	strcpy(fName, mochaRoot);
	strcat(fName, (const char *)(rx_packet->reqBuf));
	printf("KB: fName %s\n", fName);

	retval = remove(fName);

	tx_packet->errorVal = 0; //retval;
	tx_packet->funcRet = 0; //retval;

	tx_packet->header->packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet); //0x08; //0x100;
	tx_packet->respBuf = NULL;

	printf("KB: Leaving FmRemoveFile\n");
	return retval;

}

int FmMoveFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	printf("KB: Inside FmMoveFile\n");

	printf("KB: Leaving FmMoveFile\n");
	return 0;
}

/*
 * FIXME: Put proper timestamp in FileAttribute structure
 */
FmFileAttribute *fAttr;
TmDateTime fmTime = {
		.year = 2011,
		.month = 12,
		.day = 29,
		.hour = 10,
		.minute = 45,
		.second = 45,
};

int FmGetFileAttrFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
//	printf("KB: Inside FmGetFileAttrFile\n");
	int retval = 0;
	struct stat sb;
	char *fName;

	fName = (char *)malloc(sizeof(*mochaRoot) + strlen(rx_packet->reqBuf));
	strcpy(fName, mochaRoot);
	strcat(fName, (const char *)(rx_packet->reqBuf));
//	printf("KB: fName %s\n", fName);

	retval = stat(fName, &sb);

	fAttr = (FmFileAttribute *)malloc(sizeof(FmFileAttribute));

	tx_packet->funcRet = retval; //(retval < 0 ? 0 : 1); //0;
	tx_packet->errorVal = (retval < 0 ? errno : 0); // ENOENT; retval; //0;
	tx_packet->header->packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet) + sizeof(FmFileAttribute); //0x08; //0x100;

#if 1
	if(retval >= 0)
	{
#if 1
		fAttr->oldFileSize = sb.st_size;
		fAttr->startAddr = 0;
		fAttr->attribute = sb.st_mode;
		fAttr->iVol = sb.st_dev;
		fAttr->dt.year = fmTime.year;
		fAttr->dt.month = fmTime.month;
		fAttr->dt.day = fmTime.day;
		fAttr->dt.hour = fmTime.hour;
		fAttr->dt.minute = fmTime.minute;
		fAttr->dt.second = fmTime.second;
		fAttr->oldAllocatedSize = sb.st_size;
		fAttr->stModifiedDataTime.year = fmTime.year;
		fAttr->stModifiedDataTime.month = fmTime.month;
		fAttr->stModifiedDataTime.day = fmTime.day;
		fAttr->stModifiedDataTime.hour = fmTime.hour;
		fAttr->stModifiedDataTime.minute = fmTime.minute;
		fAttr->stModifiedDataTime.second = fmTime.second;
		fAttr->u64EntryUniqID = sb.st_ino;
		fAttr->uReservedField = 0;
#endif
		fAttr->fileSize = sb.st_size;
		fAttr->allocatedSize = sb.st_size;
	}
	else
	{
#if 1
		fAttr->oldFileSize = 0;
		fAttr->startAddr = 0;
		fAttr->attribute = 0;
		fAttr->iVol = 0;
		fAttr->dt.year = fmTime.year;
		fAttr->dt.month = fmTime.month;
		fAttr->dt.day = fmTime.day;
		fAttr->dt.hour = fmTime.hour;
		fAttr->dt.minute = fmTime.minute;
		fAttr->dt.second = fmTime.second;
		fAttr->oldAllocatedSize = 0;
		fAttr->stModifiedDataTime.year = fmTime.year;
		fAttr->stModifiedDataTime.month = fmTime.month;
		fAttr->stModifiedDataTime.day = fmTime.day;
		fAttr->stModifiedDataTime.hour = fmTime.hour;
		fAttr->stModifiedDataTime.minute = fmTime.minute;
		fAttr->stModifiedDataTime.second = fmTime.second;
		fAttr->u64EntryUniqID = 0;
		fAttr->uReservedField = 0;
#endif
		fAttr->fileSize = 0;
		fAttr->allocatedSize = 0;
	}
#endif

	//memcpy(tx_packet->respBuf, &fAttr, sizeof(FmFileAttribute));
	tx_packet->respBuf = fAttr;

//	printf("KB: Leaving FmGetFileAttrFile\n");
	return 0;
}

int FmFGetFileAttrFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	printf("KB: Inside FmFGetFileAttrFile\n");
	int retval = 0;
	struct stat sb;
	int fd;

	fd = *(int *)(rx_packet->reqBuf);

	retval = fstat(fd, &sb);

	fAttr = malloc(sizeof(FmFileAttribute));

	tx_packet->funcRet = (retval < 0 ? 0 : 1); //0;
	tx_packet->errorVal = (retval < 0 ? errno : 0); // ENOENT; retval; //0;
	tx_packet->header->packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet) + sizeof(FmFileAttribute); //0x08; //0x100;

	if(retval >= 0)
	{
		fAttr->oldFileSize = sb.st_size;
		fAttr->startAddr = 0;
		fAttr->attribute = sb.st_mode;
		fAttr->iVol = sb.st_dev;
		fAttr->dt.year = fmTime.year;
		fAttr->dt.month = fmTime.month;
		fAttr->dt.day = fmTime.day;
		fAttr->dt.hour = fmTime.hour;
		fAttr->dt.minute = fmTime.minute;
		fAttr->dt.second = fmTime.second;
		fAttr->oldAllocatedSize = sb.st_size;
		fAttr->stModifiedDataTime.year = fmTime.year;
		fAttr->stModifiedDataTime.month = fmTime.month;
		fAttr->stModifiedDataTime.day = fmTime.day;
		fAttr->stModifiedDataTime.hour = fmTime.hour;
		fAttr->stModifiedDataTime.minute = fmTime.minute;
		fAttr->stModifiedDataTime.second = fmTime.second;
		fAttr->u64EntryUniqID = sb.st_ino;
		fAttr->uReservedField = 0;
		fAttr->fileSize = sb.st_size;
		fAttr->allocatedSize = sb.st_size;
	}
	else
	{
		fAttr->oldFileSize = 0;
		fAttr->startAddr = 0;
		fAttr->attribute = 0;
		fAttr->iVol = 0;
		fAttr->dt.year = fmTime.year;
		fAttr->dt.month = fmTime.month;
		fAttr->dt.day = fmTime.day;
		fAttr->dt.hour = fmTime.hour;
		fAttr->dt.minute = fmTime.minute;
		fAttr->dt.second = fmTime.second;
		fAttr->oldAllocatedSize = 0;
		fAttr->stModifiedDataTime.year = fmTime.year;
		fAttr->stModifiedDataTime.month = fmTime.month;
		fAttr->stModifiedDataTime.day = fmTime.day;
		fAttr->stModifiedDataTime.hour = fmTime.hour;
		fAttr->stModifiedDataTime.minute = fmTime.minute;
		fAttr->stModifiedDataTime.second = fmTime.second;
		fAttr->u64EntryUniqID = 0;
		fAttr->uReservedField = 0;
		fAttr->fileSize = 0;
		fAttr->allocatedSize = 0;
	}

	//memcpy(tx_packet->respBuf, &fAttr, sizeof(FmFileAttribute));
	tx_packet->respBuf = fAttr;

	return retval;
}

int FmSetFileAttrFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	printf("KB: Inside FmSetFileAttrFile - TBD\n");

	printf("KB: Leaving FmSetFileAttrFile\n");
	return 0;
}

int FmTruncateFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	printf("KB: Inside FmTruncateFile - TBD\n");

	printf("KB: Leaving FmTruncateFile\n");
	return 0;
}

int FmOpenDirFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	printf("KB: Inside FmOpenDirFile\n");
	int retval = 0;
	DIR * dir;
	char *payload;
	char *fName;

	fName = (char *)malloc(sizeof(*mochaRoot) + strlen(rx_packet->reqBuf));
	strcpy(fName, mochaRoot);
	strcat(fName, (const char *)(rx_packet->reqBuf));
	printf("KB: fName %s\n", fName);
	dir = opendir(fName);

	dirArray[dirIndex++] = dir;

	tx_packet->errorVal = 0;
	tx_packet->funcRet = 1;

	tx_packet->header->packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet); //0x08; //0x100;
	tx_packet->respBuf = NULL;

	printf("KB: Leaving FmOpenDirFile\n");
	return 0;
}

int FmCloseDirFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	printf("KB: Inside FmCloseDirFile\n");

	int retval = 0;
	struct dirent * dir;
	char *payload;
	int fd;

	fd = *(int *)(rx_packet->reqBuf);
//	dir = closedir((DIR *)dirArray[fd]);

	tx_packet->errorVal = 0;
	tx_packet->funcRet = 0;

	tx_packet->header->packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet); //0x08; //0x100;
	tx_packet->respBuf = NULL;

	printf("KB: Leaving FmCloseDirFile\n");
	return 0;
}

int FmReadDirFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	printf("KB: Inside FmReadDirFile\n");
	int retval = 0;
	struct dirent * dir;
	int fd;

	fd = *(int *)(rx_packet->reqBuf);
	//dir = readdir((DIR *)dirArray[fd]);

	// TODO :
	// What to return if the directory is not empty?

	tx_packet->errorVal = 0;
	tx_packet->funcRet = 0;

	tx_packet->header->packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet); //0x08; //0x100;
	tx_packet->respBuf = NULL;

	printf("KB: Leaving FmReadDirFile\n");
	return 0;
}

int FmCreateDirFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	printf("KB: Inside FmCreateDirFile\n");
	int retval = 0;
	char *fName;

	fName = (char *)malloc(sizeof(*mochaRoot) + strlen(rx_packet->reqBuf));
	strcpy(fName, mochaRoot);
	strcat(fName, (const char *)(rx_packet->reqBuf));
	printf("KB: fName %s\n", fName);

	retval = mkdir(fName, 0777);

	if(!retval)
		printf("KB: error creating directory %s\n", fName);
	tx_packet->errorVal = 0;
	tx_packet->funcRet = 0;

	tx_packet->header->packetLen = sizeof(tx_packet->errorVal) + sizeof(tx_packet->funcRet); //0x08; //0x100;
	tx_packet->respBuf = NULL;

	printf("KB: Leaving FmCreateDirFile\n");
	return retval;
}

int FmRemoveDirFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	printf("KB: Inside FmRemoveDirFile\n");

	printf("KB: Leaving FmRemoveDirFile\n");
	return 0;
}

int FmGetQuotaSpaceFile(struct fmRequest *rx_packet, struct fmResponse *tx_packet)
{
	printf("KB: Inside FmGetQuotaSpaceFile\n");

	printf("KB: Leaving FmGetQuotaSpaceFile\n");
	return 0;
}

int get_request_packet(void *data, struct fmRequest *rx_packet)
{
	struct fmPacketHeader *header;

	header = (struct fmPacketHeader *)data;
	rx_packet->header = header;
	rx_packet->reqBuf = (char *)(data + sizeof(struct fmPacketHeader));

//	printf("Packet type = 0x%x\n", header->fmPacketType);
//	printf("Packet Length = 0x%x\n", header->packetLen);
	printf("Packet counter = 0x%x\n", header->reqCounter);
//	printf("File name = %s\n", (char *)(rx_packet->reqBuf));

	return 0;

}

int modem_response_fm(struct ipc_client *client, struct modem_io *resp)
{
	//printf("Entering modem_response_fm\n");

	int retval;
	struct fmRequest rx_packet;
	struct fmResponse tx_packet;
	struct fmArgs args;
	struct modem_io request;
    unsigned char *frame;
    unsigned char *payload;
    int frame_length;

    get_request_packet(resp->data, &rx_packet);

	tx_packet.header = rx_packet.header;
	retval = fileOps[(tx_packet.header->fmPacketType + 0xEFFFFFFF)](&rx_packet, &tx_packet);

    frame_length = (sizeof(struct fmPacketHeader) + tx_packet.header->packetLen);

	request.magic = resp->magic;
	request.cmd = resp->cmd;
	request.datasize = frame_length;


    frame = (unsigned char*)malloc(frame_length);

    memcpy(frame, tx_packet.header, sizeof(struct fmPacketHeader));

    payload = (frame + sizeof(struct fmPacketHeader));

    memcpy(payload, &tx_packet.funcRet, sizeof(tx_packet.funcRet));

    payload = (payload + sizeof(tx_packet.funcRet));

    memcpy(payload, &tx_packet.errorVal, sizeof(tx_packet.errorVal));

    payload = (payload + sizeof(tx_packet.errorVal));

    memcpy(payload, tx_packet.respBuf, tx_packet.header->packetLen - (sizeof(tx_packet.errorVal) + sizeof(tx_packet.funcRet)));

	request.data = frame;

	_ipc_client_send(client, &request);

	if(tx_packet.respBuf != NULL)
        free(tx_packet.respBuf);

    if(frame != NULL)
        free(frame);

    if(!lastOpen && lastFile)
    {
    	lastFile = 0;
    	return 1;
    }
    else
    	return 0;
}

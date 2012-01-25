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
 *
 */

#ifndef __FM_H__
#define __FM_H__

#include <radio.h>

/*
 * Declarations copied from Dolphin headers
 */

/* File Manager */
#define FM_FILENAME_LEN_MAX						236
#define FM_FILEPATH_LEN_MAX						256

typedef unsigned char		UCHAR; /* SD_INT */
typedef unsigned short		USHORT;
typedef unsigned int		UINT;
typedef unsigned long		ULONG;

/* Machine independence */
typedef signed   char		INT8;
typedef unsigned char		UINT8;
typedef signed short				INT16;
typedef unsigned short		UINT16;

#ifndef ADDR
typedef unsigned int		ADDR;
#endif // ADDR

/* Machine independence */
typedef  long int				INT32;
typedef unsigned long		UINT32;
typedef long	long			INT64;
typedef unsigned long long	UINT64;

//PACKETTYPE 0x6 FmPacket
struct fmPacketHeader {
	unsigned int fmPacketType; 	//(stored as type-0xEFFFFFFF, just take first byte substract 1 to get actual type)
	unsigned int reserved1; 		//dummy, unused?
	unsigned int packetLen; 		//equal to n, equal to packetLength-16 (headersize)
	unsigned int reqCounter; 	// has to be the same in responsepacket, probably fm request counter
};

struct fmRequest {
	struct fmPacketHeader *header; 	// has to be the same in responsepacket, probably fm request counter
	unsigned char *reqBuf; 		//usually first comes unsigned int params, and then string containing name
};

struct fmResponse {
	struct fmPacketHeader *header; 	// has to be the same in responsepacket, probably fm request counter
	int funcRet; 		//called function return value
	int errorVal; 		//0 if func_ret == 0, otherwise retvalue of platform LastError()
	unsigned char *respBuf;
};

struct fmArgs {
	unsigned int mode;
	char *fileName;
	unsigned int fileHandle;
	void *readBuf;
	void *writeBuf;
	unsigned int size;
	unsigned int numRead;
	unsigned int offset;
	unsigned int origin;
};

/*
 * Access Modes
 */
#define FM_CREATE				0x0001
#define FM_READ					0x0002
#define FM_WRITE				0x0004
#define FM_TRUNCATE				0x0008
#define FM_APPEND				0x0010
#define FM_NOSHARE				0x0020
#define FM_DIRECTIO				0x0040	// this mode does not use write buffer. so slow
#define FM_CONCRETE_WRITE		0x0080	// this mode use backup writing up to  FM_CONCRETE_WRITE_BUFFER_MAX
										// this mode is 2x more times slower than normal writing
										// but it garantees atomic writing via logging
#define FM_NOUPDATE_TIME 		0x0100   // if this flag is on, directory entry update for last modified time will not occurs.
										// this can increase file system endurance

/**
 *  	  DirectoryEntry Type
 */
typedef enum
{
	FM_INVALID_TYPE =0x0000, 		 /**FM_INVALID_TYPE*/
	FM_FILE_TYPE = 0x0001,			 /**FM_FILE_TYPE*/
	FM_DIR_TYPE = 0x0002,			 /**FM_DIR_TYPE*/	// same as FM_ATTRIBUTE_DIR
	FM_UNKNOWN_TYPE = 0x0003,		  /**FM_UNKNOWN_TYPE*/   // Sent from Qualcomm side
	FM_TYPE_MAX = 0xFF000000		// for enumeration type size
} FmEntryType;

/**
 * file system type
 */
typedef enum
{
	FM_FS_TFS2 = 0,
	FM_FS_TFS3 = 1,
	FM_FS_TFS4 = 2,	// MMC, TFS4
	FM_FS_EFS2 = 3,
	FM_FS_LFS = 4,
	FM_FS_RFS = 5,			// Ram File System
	FM_FS_REMOTE =6,
	FM_FS_WIN32 = 7,
	FM_FS_TCCFS =8,
	FM_FS_MTEKFS=9,
	FM_FS_MAX =10
} FmFsType;

typedef struct
{
	int		year; 	/**<Year (1900[1900_BASE] or 1970[1970_BASE] ~ 2030)*/
	int		month;	/**<Month (1-12)*/
	int		day;	/**<Day (1-31)*/
	int		hour;	/**<Hour (1-12)*/
	int		minute; 	/**<Minute (1-60)*/
	int		second;	/**<Second (1-60)*/
} TmDateTime;

typedef struct
{
	unsigned long		oldFileSize;		// for backward compatibilty with previous PC tools
	unsigned long*		startAddr;			/** Only used at LFS*/
	unsigned long		attribute;			/** file attribute like directory or file, hidden, readonly, system, ...	*/
	int			iVol;				/** positioned volume*/
	TmDateTime	dt;					/** Creation date/time*/
	unsigned long		oldAllocatedSize;	// for backward compatibilty with previous PC tools
	TmDateTime	stModifiedDataTime;	/** DON NOT USE THIS FIELD YET!!!*/
	unsigned long long		u64EntryUniqID;		/** uniq ID for file or directory, data cluster number + ctime mtime*/
	unsigned long		uReservedField;		/** DON NOT USE THIS FIELD YET!!!*/
	unsigned long long/*UINT64*/	fileSize;			/** File Size in bytes */
	unsigned long long/*UINT64*/	allocatedSize;	/** real allocated size of file & directory in sub System
									*Note: allocated size for sub directories & sub files are not included
										only allocation size for directory itself!!!
									*/
} FmFileAttribute;

/**
  * @brief		These are basic structures for Directory Attribute
  */
typedef struct
{
	FmEntryType	type;					/** File or Directory*/
	ULONG		oldFileSize;			// for backward compatibilty with previous PC tools
	ULONG		attribute;				/** Attribute*/
	TmDateTime	dt;						/** Creation date/time*/
	char		szDummy[FM_FILEPATH_LEN_MAX];	// for backward compatibility with previous PC tools
	ULONG		oldAllocatedSize;		// for backward compatibilty with previous PC tools
	TmDateTime	stModifiedDataTime;	/** DON NOT USE THIS FIELD YET!!!*/
	UINT64		u64EntryUniqID;		/** uniq ID for file or directory, data cluster number + ctime mtime*/
	ULONG /*UINT64*/fileSize;			/** File size if entry is file. meaningless for directory entry */
	ULONG /*UINT64*/allocatedSize;		/** real allocated size of file or directory in Sub System
										 *Note: allocated size for sub directories & sub files are not included
										  only allocation size for directory itself!!!*/
	char		szName[FM_FILENAME_LEN_MAX];	/** File name*/
	ULONG		uReservedField;		/** DON NOT USE THIS FIELD YET!!!*/

} FmDirEntry;

/**
  * @brief		These are basic structures for VolumeStat
  */
typedef struct
{
	ULONG		availableSizeDummy;	/** Dummy for compatibility with  legacy facotry softwrare*/
	ULONG		allocatedSizeDummy;	/** Dummy for compatibility with  legacy facotry softwrare*/
	FmFsType	fsType;					/** volume type such as FM_FSTYPE_FIXED, FM_FSTYPE_REMOVABLE...*/
	UINT64		availableSize;			/** available size */
	UINT64		allocatedSize;			/** actual usage in File Syste*/
} FmVolumeStat;

/**
  * @brief		These are basic structures for QuotaStat
  */
typedef struct
{
	ULONG			availableSizeDummy;	/**  Dummy for compatibility with  legacy facotry softwrare*/
	ULONG			allocatedSizeDummy;	/** Dummy for compatibility with  legacy facotry softwrare	*/
	ULONG			reservedSizeDummy;	/** Dummy for compatibility with  legacy facotry softwrare		*/
	ULONG			diskSizeDummy;			/** Dummy for compatibility with  legacy facotry softwrare	 */
	UINT64			availableSize;			/** available size means Reserved area plus remaining Shared area*/
	UINT64			allocatedSize;			/** actual usage in File System for registered Quota entry*/
	UINT64			reservedSize;			/** if requested path is not registered. it will be 0*/
	UINT64			diskSize;				/** fixed size of Quota Area in TFS. This value is a practical meaning as disk size to user. */
	UINT32			clusterSize;				/** disk Cluster Size. */
} FmQuotaStat;

typedef struct
{
	char pathname[FM_FILEPATH_LEN_MAX];		// null-terminated pathname
	FmEntryType entryType;					// File or Directory
} FmOpenEntry;


/**
  * @brief		These are file operations requested by CP
  */
#define FM_OPENFILE 		0
#define FM_CLOSEFILE 		1
#define FM_CREATEFILE 		2
#define FM_READFILE 		3
#define FM_WRITEFILE 		4
#define FM_FLUSHFILE 		5
#define FM_SEEKFILE 		6
#define FM_TELLFILE 		7
#define FM_REMOVEFILE 		8
#define FM_MOVEFILE 		9
#define FM_GETFILEATTR 		10
#define FM_FGETFILEATTR 	11
#define FM_SETFILEATTR 		12
#define FM_TRUNCATEFILE 	13
#define FM_OPENDIR 			14
#define FM_CLOSEDIR 		15
#define FM_READDIR 			16
#define FM_CREATEDIR 		17
#define FM_REMOVEDIR 		18
#define FM_GETQUOTASPACE 	19
#define FM_INVALID 			20

#define MAX_FILE_OPS 		20

int FmOpenFile(struct fmRequest *, struct fmResponse *);
int FmCloseFile(struct fmRequest *, struct fmResponse *);
int FmCreateFile(struct fmRequest *, struct fmResponse *);
int FmReadFile(struct fmRequest *, struct fmResponse *);
int FmWriteFile(struct fmRequest *, struct fmResponse *);
int FmFlushFile(struct fmRequest *, struct fmResponse *);
int FmSeekFile(struct fmRequest *, struct fmResponse *);
int FmTellFile(struct fmRequest *, struct fmResponse *);
int FmRemoveFile(struct fmRequest *, struct fmResponse *);
int FmMoveFile(struct fmRequest *, struct fmResponse *);
int FmGetFileAttrFile(struct fmRequest *, struct fmResponse *);
int FmFGetFileAttrFile(struct fmRequest *, struct fmResponse *);
int FmSetFileAttrFile(struct fmRequest *, struct fmResponse *);
int FmTruncateFile(struct fmRequest *, struct fmResponse *);
int FmOpenDirFile(struct fmRequest *, struct fmResponse *);
int FmCloseDirFile(struct fmRequest *, struct fmResponse *);
int FmReadDirFile(struct fmRequest *, struct fmResponse *);
int FmCreateDirFile(struct fmRequest *, struct fmResponse *);
int FmRemoveDirFile(struct fmRequest *, struct fmResponse *);
int FmGetQuotaSpaceFile(struct fmRequest *, struct fmResponse *);
//int FmInvalidFile(unsigned int mode, const char *fileName);

int modem_response_fm(struct ipc_client *client, struct modem_io *resp);

#endif

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

#ifndef __TYPES_H__
#define __TYPES_H__

#if defined(DEVICE_JET)
#include "device/jet/jet_modem_ctl.h"
#elif defined(DEVICE_WAVE)
#include "device/wave/wave_modem_ctl.h"
#endif

#ifndef LOG_FILE
#define LOG_FILE stderr
#endif

#ifdef RIL_SHLIB
#define DEBUG_E(...) LOGE(__VA_ARGS__)
#define DEBUG_W(...) LOGW(__VA_ARGS__)
#define DEBUG_I(...) LOGI(__VA_ARGS__)
#else

#ifdef DEBUG_ERROR
#define DEBUG_E(x, args ...) fprintf(LOG_FILE, "[%s:%u] ERROR: " \
x, __FUNCTION__ ,__LINE__, ## args)
#define DEBUG_W(x, args ...)
#define DEBUG_I(x, args ...)
#else
#ifdef DEBUG_WARNING
#define DEBUG_E(x, args ...) fprintf(LOG_FILE, "[%s:%u] ERROR: " \
x, __FUNCTION__ ,__LINE__, ## args)
#define DEBUG_W(x, args ...) fprintf(LOG_FILE, "[%s:%u] WARNING: " \
x, __FUNCTION__ ,__LINE__, ## args)
#define DEBUG_I(x, args ...)
#else
#ifdef DEBUG_INFO
#define DEBUG_E(x, args ...) fprintf(LOG_FILE, "[%s:%u] ERROR: " \
x, __FUNCTION__ ,__LINE__, ## args)
#define DEBUG_W(x, args ...) fprintf(LOG_FILE, "[%s:%u] WARNING: " \
x, __FUNCTION__ ,__LINE__, ## args)
#define DEBUG_I(x, args ...) fprintf(LOG_FILE, "[%s:%u] INFO: " \
x, __FUNCTION__ ,__LINE__, ## args)
#else
#define DEBUG_E(x, args ...)
#define DEBUG_W(x, args ...)
#define DEBUG_I(x, args ...)
#endif
#endif
#endif

#endif //RIL_SHLIB

#define MAX_SIGNLE_FRAME_DATA 	0xFF4

/* AP -> Baseband */
#define IPC_TYPE_EXEC                               0x01
#define IPC_TYPE_GET                                0x02
#define IPC_TYPE_SET                                0x03
#define IPC_TYPE_CFRM                               0x04
#define IPC_TYPE_EVENT                              0x05

/* Baseband -> AP */
#define IPC_TYPE_INDI                               0x01
#define IPC_TYPE_RESP                               0x02
#define IPC_TYPE_NOTI                               0x03

/**
 * FIFO packet type list
 */
typedef enum
{
	FIFO_PKT_NONE = 0, 		// 0
	FIFO_PKT_KEY,			// 1
	FIFO_PKT_SIM,			// 2
	FIFO_PKT_PROTO,			// 3
	FIFO_PKT_TAPI,			// 4
	FIFO_PKT_PHONESTATUS,	// 5
	FIFO_PKT_FILE,			// 6
	FIFO_PKT_LCD,			// 7
	FIFO_PKT_LED,			// 8
	FIFO_PKT_SOUND,			// 9 Sound means voice here
	FIFO_PKT_SOUND_DATA,	// 10
	FIFO_PKT_H324M,			// 11
	FIFO_PKT_AMR_DATA,		// 12
	FIFO_PKT_AMR_CTRL,		// 13
	FIFO_PKT_CLOCK,			// 14
	FIFO_PKT_BOOT,			// 15
	FIFO_PKT_FLIP,			// 16
	FIFO_PKT_SYSTEM,		// 17
	FIFO_PKT_USBPROTO,		// 18
	FIFO_PKT_USBFILE,		// 19
	FIFO_PKT_USBDIAG,		// 20
	FIFO_PKT_IRDAPROTO,		// 21
	FIFO_PKT_IRDAFILE,		// 22
	FIFO_PKT_IRDADIAG,		// 23
	FIFO_PKT_TIMER,			// 24
	FIFO_PKT_DEBUG,			// 25
	FIFO_PKT_DIAGNOSE,		// 26
	FIFO_PKT_SPECIAL_BOOT,	// 27
	FIFO_PKT_CALL_TIME,		// 28
	FIFO_PKT_ALARM,			// 29
	FIFO_PKT_FIFO_INTERNAL, // 30
	FIFO_PKT_USBCRCPROTO,	// 31
	FIFO_PKT_USBCRCFILE,	// 32
	FIFO_PKT_USBCRCDIAG,	// 33
	FIFO_PKT_VIBRATOR,		// 34
	FIFO_PKT_AMLED,			// 35 AppMgr LED
	FIFO_PKT_AMVIB,			// 36 AppMgr Vibrator
	FIFO_PKT_AMLCD,			// 37 AppMgr LCD Backlight
	FIFO_PKT_DATA_PCSYNC,
	FIFO_PKT_CTRLCMD_PCSYNC,
    FIFO_PKT_DATA_WSSSYNC,
	FIFO_PKT_TIME,			// 41 TimeMgr
	FIFO_PKT_DVB_H_CAS_SIM,	// 42 DVB-H CAS SIM
	FIFO_PKT_DVB_H_CAS_TEST,// 43 DVB-H CAS Test module.
	FIFO_PKT_DVB_H_CAS,		// 44 DVB-H CAS Common usage.
	FIFO_PKT_DVB_H_CAS_IPS, // 45 DVB-H CAS IPS usage.
	FIFO_PKT_DVB_H_DebugLevel, //46 receive debug level from MSM
    FIFO_PKT_Forced_Assert, // 47
	FIFO_PKT_MEMORY,        // 48
	FIFO_PKT_NV,	        // 49		// NvMgrLite
	FIFO_PKT_LBS,			// 50 LBS
    FIFO_PKT_SIM_JSR177,    // 51 S8000_JSR177_kjseo

	FIFO_PKT_USER = 0x80,
	FIFO_PKT_DVBH = FIFO_PKT_USER + 0x06,
	FIFO_PKT_DVBH_SVC,
	FIFO_PKT_DVB_H_LAYER1,

	FIFO_PKT_DVB_PLAYER,
	FIFO_PKT_AV_PLAYER,

	FIFO_PKT_PH,			// BB -> MM : Protocol Handler FIFO Type
	FIFO_PKT_PH_LITE,		// MM -> BB : Protocol Handler Lite FIFO Type

	FIFO_PKT_FX = 0x90,

	FIFO_PKT_BLUETOOTH ,

	FIFO_PKT_TESTMODE,	// Testmode

	FIFO_PKT_DRV,	//  driver
	FIFO_PKT_AGENT,
	FIFO_PKT_DEVMGR,
	FIFO_PKT_SECUREBOOT,

/*
 * Add WAVE specific Packet types here
 */

#if defined(DEVICE_WAVE)

#endif

	FIFO_PKT_MAX

} FifoType;

/* Message groups */
#define IPC_GROUP_PWR                               0x01
#define IPC_GROUP_CALL                              0x02
#define IPC_GROUP_SMS                               0x04
#define IPC_GROUP_SEC                               0x05
#define IPC_GROUP_PB                                0x06
#define IPC_GROUP_DISP                              0x07
#define IPC_GROUP_NET                               0x08
#define IPC_GROUP_SND                               0x09
#define IPC_GROUP_MISC                              0x0A
#define IPC_GROUP_SVC                               0x0B
#define IPC_GROUP_SS                                0x0C
#define IPC_GROUP_GPRS                              0x0D
#define IPC_GROUP_SAT                               0x0E
#define IPC_GROUP_CFG                               0x0F
#define IPC_GROUP_IMEI                              0x10
#define IPC_GROUP_GPS                               0x11
#define IPC_GROUP_SAP                               0x12
#define IPC_GROUP_RFS                               0x42
#define IPC_GROUP_GEN                               0x80

/* Pb */
#define IPC_PB_ACCESS                               0x0601
#define IPC_PB_STORAGE                              0x0602
#define IPC_PB_STORAGE_LIST                         0x0603
#define IPC_PB_ENTRY_INFO                           0x0604
#define IPC_PB_CAPABILITY_INFO                      0x0605

/* SND */

#define IPC_SND_SPKR_VOLUME_CTRL                    0x9001
#define IPC_SND_MIC_MUTE_CTRL                       0x0902
#define IPC_SND_AUDIO_PATH_CTRL                     0x0903
#define IPC_SND_RINGBACK_TONE_CTRL                  0x0908
#define IPC_SND_CLOCK_CTRL                          0x0909

/* SS */
#define IPC_SS_WAITING                              0x0C01
#define IPC_SS_CLI                                  0x0C02
#define IPC_SS_BARRING                              0x0C03
#define IPC_SS_BARRING_PW                           0x0C04
#define IPC_SS_FORWARDING                           0x0C05
#define IPC_SS_INFO                                 0x0C06
#define IPC_SS_MANAGE_CALL                          0x0C07
#define IPC_SS_USSD                                 0x0C08
#define IPC_SS_AOC                                  0x0C09
#define IPC_SS_RELEASE_COMPLETE                     0x0C0A

/* Sat */
#define IPC_SAT_PROFILE_DOWNLOAD                    0x0E01
#define IPC_SAT_ENVELOPE_CMD                        0x0E02
#define IPC_SAT_PROACTIVE_CMD                       0x0E03
#define IPC_SAT_TERMINATE_USAT_SESSION              0x0E04
#define IPC_SAT_EVENT_DOWNLOAD                      0x0E05
#define IPC_SAT_PROVIDE_LOCAL_INFO                  0x0E06
#define IPC_SAT_POLLING                             0x0E07
#define IPC_SAT_REFRESH                             0x0E08
#define IPC_SAT_SETUP_EVENT_LIST                    0x0E09
#define IPC_SAT_CALL_CONTROL_RESULT                 0x0E0A
#define IPC_SAT_IMAGE_CLUT                          0x0E0B
#define IPC_SAT_CALL_PROCESSING                     0x0E0C

/* IMEI */
#define IPC_IMEI_START                              0x1001
#define IPC_IMEI_CHECK_DEVICE_INFO                  0x1002

#endif

// vim:ts=4:sw=4:expandtab

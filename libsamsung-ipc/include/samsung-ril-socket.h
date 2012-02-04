/**
 * This file is part of samsung-ril.
 *
 * Copyright (C) 2011 Paul Kocialkowski <contact@oaulk.fr>
 *
 * samsung-ril is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * samsung-ril is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with samsung-ril.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/* Samsung RIL Socket protocol defines */

#ifndef _SAMSUNG_RIL_SOCKET_H_
#define _SAMSUNG_RIL_SOCKET_H_

#define SRS_COMMAND(f)  ((f->group << 8) | f->index)
#define SRS_GROUP(m)    (m >> 8)
#define SRS_INDEX(m)    (m & 0xff)

#define SRS_SOCKET_NAME			"samsung-ril-socket"
#define SRS_DATA_MAX_SIZE		0x1000

#define SRS_CONTROL			0x01
#define SRS_CONTROL_PING		0x0101

#define SRS_SND				0x02
#define SRS_SND_SET_CALL_VOLUME		0x0201
#define SRS_SND_SET_CALL_AUDIO_PATH	0x0202
#define SRS_SND_SET_CALL_CLOCK_SYNC	0x0203

#define SRS_CONTROL_CAFFE		0xCAFFE

enum srs_snd_type {
	SRS_SND_TYPE_VOICE,
	SRS_SND_TYPE_SPEAKER,
	SRS_SND_TYPE_HEADSET,
	SRS_SND_TYPE_BTVOICE
};

enum srs_snd_path {
	SRS_SND_PATH_HANDSET,
	SRS_SND_PATH_HEADSET,
	SRS_SND_PATH_SPEAKER,
	SRS_SND_PATH_BLUETOOTH,
	SRS_SND_PATH_BLUETOOTH_NO_NR,
	SRS_SND_PATH_HEADPHONE
};

enum srs_snd_clock {
	SND_CLOCK_STOP,
	SND_CLOCK_START
};

struct srs_snd_call_volume {
	enum srs_snd_type type;
	int volume;
} __attribute__((__packed__));

struct srs_header {
	unsigned int length;
	unsigned char group;
	unsigned char index;
} __attribute__((__packed__));

struct srs_message {
	unsigned short command;
	int data_len;
	void *data;
} __attribute__((__packed__));

#endif

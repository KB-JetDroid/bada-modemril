/**
 * This file is part of mocha-ril.
 *
 * Copyright (C) 2011 Paul Kocialkowski <contact@oaulk.fr>
 *
 * mocha-ril is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mocha-ril is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mocha-ril.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#define LOG_TAG "RIL-Mocha-SND"
#include <utils/Log.h>

#include "mocha-ril.h"
#include "util.h"
#include <sound.h>

/**
 * Format conversion utils
 */
#if 0
unsigned char srs2ipc_call_type(int type)
{
	switch(type) {
		case SRS_SND_TYPE_VOICE:
			return IPC_SND_VOLUME_TYPE_VOICE;
		case SRS_SND_TYPE_SPEAKER:
			return IPC_SND_VOLUME_TYPE_SPEAKER;
		case SRS_SND_TYPE_HEADSET:
			return IPC_SND_VOLUME_TYPE_HEADSET;
		case SRS_SND_TYPE_BTVOICE:
			return IPC_SND_VOLUME_TYPE_BTVOICE;
		default:
			ALOGE("Unknown call type: 0x%x", type);
			return 0;
	}
}

unsigned char srs2ipc_audio_path(int path)
{
	switch(path) {
		case SRS_SND_PATH_HANDSET:
			return IPC_SND_AUDIO_PATH_HANDSET;
		case SRS_SND_PATH_HEADSET:
			return IPC_SND_AUDIO_PATH_HEADSET;
		case SRS_SND_PATH_SPEAKER:
			return IPC_SND_AUDIO_PATH_SPEAKER;
		case SRS_SND_PATH_BLUETOOTH:
			return IPC_SND_AUDIO_PATH_BLUETOOTH;
		case SRS_SND_PATH_BLUETOOTH_NO_NR:
			return IPC_SND_AUDIO_PATH_BLUETOOTH_NO_NR;
		case SRS_SND_PATH_HEADPHONE:
			return IPC_SND_AUDIO_PATH_HEADPHONE;
		default:
			ALOGE("Unknown audio path: 0x%x", path);
			return 0;
	}
}

#endif

void ril_request_set_mute(RIL_Token t, void *data, size_t datalen)
{
	soundSetMuteSndPathPacket set_mute;

	ALOGE("%s: Implement me!", __func__);

	unsigned char mute_data = ((int *)data)[0] > 0 ? 1 : 0;

	ALOGD("Mute data is %d\n", mute_data);

	set_mute.packetType = SOUND_PACKET_SET_MUTE; 
	set_mute.align = 0; /* random stuff */
	set_mute.inDevice = 0x100;
	set_mute.outDevice = 2;
	set_mute.inDeviceMuted = mute_data;
	set_mute.outDeviceMuted = mute_data;
	set_mute.soundType = 0xD; //fake 
	set_mute.oemVolume = 6; /* always 6 for mute packet */

	sound_send_packet((uint8_t *)&set_mute, sizeof(soundSetMuteSndPathPacket));

	ril_request_complete(t, RIL_E_SUCCESS, NULL, 0);
}

void srs_snd_set_call_clock_sync(struct srs_message *message)
{
	ALOGE("%s: Implement me!", __func__);

	unsigned char data = *((unsigned char *) message->data);
	ALOGD("Clock sync data is 0x%x\n", data);

}

void srs_snd_set_call_volume(struct srs_message *message)
{
	ALOGE("%s: Implement me!", __func__);

	soundSetVolumePacket set_vol;

	struct srs_snd_call_volume *call_volume = (struct srs_snd_call_volume *) message->data;

	ALOGD("Call volume for: 0x%x vol = 0x%x\n", call_volume->type, call_volume->volume);

	set_vol.packetType = SOUND_PACKET_SET_VOLUME; 
	set_vol.align[0] = 0; /* random stuff */
	set_vol.align[1] = 0; /* random stuff */
	set_vol.align[2] = 0; /* random stuff */
	set_vol.outDevice = 2;
	set_vol.inDeviceMuted = 0;
	set_vol.outDeviceMuted = 0;
	set_vol.soundType = 0xD; //fake
	set_vol.oemVolume = 1; //fake

	sound_send_packet((uint8_t *)&set_vol, sizeof(soundSetVolumePacket));


}

void srs_snd_set_call_audio_path(struct srs_message *message)
{
	ALOGE("%s: Implement me!", __func__);

	soundSetMuteSndPathPacket a_path;

	int audio_path = ((int *) message->data)[0];
	//unsigned char path = srs2ipc_audio_path(audio_path);

	ALOGD("Audio path to: 0x%x\n",audio_path);

	a_path.packetType = SOUND_PACKET_SET_SND_PATH;
	a_path.align = 0; /* random stuff */ 
	a_path.inDevice = 0x100;
	a_path.outDevice = 2;
	a_path.inDeviceMuted = 0;
	a_path.outDeviceMuted = 0;
	a_path.soundType = 0xD; //fake 
	a_path.oemVolume = 1; //fake
	sound_send_packet((uint8_t *)&a_path, sizeof(soundSetMuteSndPathPacket));

}

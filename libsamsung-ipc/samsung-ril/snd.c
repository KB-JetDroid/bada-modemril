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

#define LOG_TAG "RIL-SND"
#include <utils/Log.h>

#include "samsung-ril.h"
#include "util.h"

/**
 * Format conversion utils
 */

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
			LOGE("Unknown call type: 0x%x", type);
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
			LOGE("Unknown audio path: 0x%x", path);
			return 0;
	}
}

void ril_request_set_mute(RIL_Token t, void *data, size_t datalen)
{
	unsigned char mute_data = ((int *)data)[0] > 0 ? 1 : 0;

	LOGD("Mute data is %d\n", mute_data);

	ipc_gen_phone_res_expect_to_complete(reqGetId(t), IPC_SND_MIC_MUTE_CTRL);

	ipc_fmt_send(IPC_SND_MIC_MUTE_CTRL, IPC_TYPE_SET, (void *) &mute_data, sizeof(mute_data), reqGetId(t));
}

void srs_snd_set_call_clock_sync(struct srs_message *message)
{
	unsigned char data = *((unsigned char *) message->data);
	LOGD("Clock sync data is 0x%x\n", data);

	ipc_fmt_send(IPC_SND_CLOCK_CTRL, IPC_TYPE_EXEC, &data, sizeof(data), reqIdNew());
}

void srs_snd_set_call_volume(struct srs_message *message)
{
	struct srs_snd_call_volume *call_volume = (struct srs_snd_call_volume *) message->data;
	struct ipc_snd_spkr_volume_ctrl volume_ctrl;

	LOGD("Call volume for: 0x%x vol = 0x%x\n", call_volume->type, call_volume->volume);

	volume_ctrl.type = srs2ipc_call_type(call_volume->type);
	volume_ctrl.volume = call_volume->volume;

	ipc_fmt_send(IPC_SND_SPKR_VOLUME_CTRL, IPC_TYPE_SET, (void *) &volume_ctrl, sizeof(volume_ctrl), reqIdNew());
}

void srs_snd_set_call_audio_path(struct srs_message *message)
{
	int audio_path = ((int *) message->data)[0];
	unsigned char path = srs2ipc_audio_path(audio_path);

	LOGD("Audio path to: 0x%x\n", path);

	ipc_fmt_send(IPC_SND_AUDIO_PATH_CTRL, IPC_TYPE_SET, (void *) &path, sizeof(path), reqIdNew());
}

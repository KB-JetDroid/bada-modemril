/**
 * This file is part of samsung-ril.
 *
 * Copyright (C) 2010-2011 Joerie de Gram <j.de.gram@gmail.com>
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

#define LOG_TAG "RIL-MISC"
#include <utils/Log.h>

#include "samsung-ril.h"
#include "util.h"

// FIXME: there was a problem with IMEI handling *think hard*

void ril_request_get_imei(RIL_Token t)
{
	char data;

	if(ril_state.radio_state != RADIO_STATE_OFF) {
		data = IPC_MISC_ME_SN_SERIAL_NUM;

		ipc_fmt_send(IPC_MISC_ME_SN, IPC_TYPE_GET, (unsigned char *) &data, sizeof(data), reqGetId(t));
	} else {
		ril_state.tokens.get_imei = t;
	}
}

void ril_request_get_imeisv(RIL_Token t)
{
	ril_state.tokens.get_imeisv = t;
}

void ipc_misc_me_sn_imei(RIL_Token t, void *data, int length)
{
	struct ipc_misc_me_sn *imei_info;
	char imei[33];
	char imeisv[3];

	imei_info = (struct ipc_misc_me_sn *) data;

	if(ril_state.tokens.get_imei != 0 && ril_state.tokens.get_imei != t) 
		LOGE("IMEI tokens mismatch");

	if(imei_info->length > 32)
		return;

	memset(imei, 0, sizeof(imei));
	memset(imeisv, 0, sizeof(imeisv));

	memcpy(imei, imei_info->data, imei_info->length);

	/* Last two bytes of IMEI in imei_info are the SV bytes */
	memcpy(imeisv, (imei_info->data + imei_info->length - 2), 2);

	/* IMEI */
	RIL_onRequestComplete(t, RIL_E_SUCCESS, imei, sizeof(char *));
	ril_state.tokens.get_imei = 0;

	/* IMEI SV */
	if(ril_state.tokens.get_imeisv != 0) {
		RIL_onRequestComplete(ril_state.tokens.get_imeisv, RIL_E_SUCCESS, imeisv, sizeof(char *));
		ril_state.tokens.get_imeisv = 0;
	}
}

void ipc_misc_me_sn(struct ipc_message_info *info)
{
	struct ipc_misc_me_sn *me_sn_info = (struct ipc_misc_me_sn *) info->data;

	switch(me_sn_info->type) {
		case IPC_MISC_ME_SN_SERIAL_NUM:
			ipc_misc_me_sn_imei(reqGetToken(info->aseq), info->data, info->length);
			break;
		case IPC_MISC_ME_SN_SERIAL_NUM_SERIAL:
			LOGD("Got IPC_MISC_ME_SN_SERIAL_NUM_SERIAL: %s\n", me_sn_info->data);
			break;
	}
}

void ril_request_baseband_version(RIL_Token t)
{
	if(ril_state.radio_state != RADIO_STATE_OFF) {
		ipc_fmt_send_get(IPC_MISC_ME_VERSION, reqGetId(t));
	} else {
		ril_state.tokens.baseband_version = t;
	}
}

void ipc_misc_me_version(struct ipc_message_info *info)
{
	char sw_version[33];
	struct ipc_misc_me_version *version = (struct ipc_misc_me_version *) info->data;

	memcpy(sw_version, version->sw_version, 32);
	sw_version[32] = '\0';

	RIL_onRequestComplete(reqGetToken(info->aseq), RIL_E_SUCCESS, sw_version, sizeof(sw_version));
}

/**
 * In: RIL_REQUEST_GET_IMSI
 *   Get the SIM IMSI
 *   Only valid when radio state is "RADIO_STATE_SIM_READY"
 *
 * Out: IPC_MISC_ME_IMSI
 *   Requests ME's IMSI
 */
void ril_request_get_imsi(RIL_Token t)
{
	ipc_fmt_send_get(IPC_MISC_ME_IMSI, reqGetId(t));
}

/**
 * In: IPC_MISC_ME_IMSI
 *   Provides ME's IMSI
 *
 * Out: RIL_REQUEST_GET_IMSI
 *   Get the SIM IMSI
 *   Only valid when radio state is "RADIO_STATE_SIM_READY"
 */
void ipc_misc_me_imsi(struct ipc_message_info *info)
{
	unsigned char *imsi_length;
	char *imsi;

	if(info->length < 1) {
		LOGE("%s: zero data length", __FUNCTION__);
		RIL_onRequestComplete(reqGetToken(info->aseq), RIL_E_GENERIC_FAILURE, NULL, 0);
		return;
	}

	imsi_length = (unsigned char*) info->data;

	if(((int) info->length) < *imsi_length + 1) {
		LOGE("%s: missing IMSI data", __FUNCTION__);
		RIL_onRequestComplete(reqGetToken(info->aseq), RIL_E_GENERIC_FAILURE, NULL, 0);
		return;
	}

	/* Copy IMSI */
	imsi = (char*) malloc(*imsi_length+1);
	memcpy(imsi, ((unsigned char*) info->data) + 1, *imsi_length);
	imsi[*imsi_length] = '\0';

	RIL_onRequestComplete(reqGetToken(info->aseq), RIL_E_SUCCESS, imsi, *imsi_length+1);
}

void ipc_misc_time_info(struct ipc_message_info *info)
{
	struct ipc_misc_time_info *nitz = (struct ipc_misc_time_info*) info->data;
	char str[128];

	sprintf(str, "%02u/%02u/%02u,%02u:%02u:%02u+%02d,%02d",
		nitz->year, nitz->mon, nitz->day, nitz->hour, nitz->min, nitz->sec, nitz->tz, 0);

	RIL_onUnsolicitedResponse(RIL_UNSOL_NITZ_TIME_RECEIVED, str, strlen(str) + 1);
}

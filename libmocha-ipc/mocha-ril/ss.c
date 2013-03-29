/**
 * This file is part of mocha-ril.
 *
 * Copyright (C) 2013 Dominik Marszk <dmarszk@gmail.com>
 * Copyright (C) 2013 Nikolay Volkov <volk204@mail.ru>
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

#define LOG_TAG "Mocha-RIL-SS"
#include <utils/Log.h>

#include "mocha-ril.h"
#include "util.h"
#include <tapi_ss.h>


void ipc_ss_ussd_cnf_response(void* data)
{
	ALOGE("%s: test me!", __func__);

	char *message[2];
	int i, result = 0;
	char *data_dec = NULL;
	int data_dec_len = 0;
	int len;
	tapiSsCnf* ssResp = (tapiSsCnf*)(data);

	memset(message, 0, sizeof(message));

	data_dec_len = ssResp->strLen;

	asprintf(&message[0], "%d", 0);

	switch (ssResp->dcs)
	{
		case 0x48:
	
			ALOGD("USSD Rx encoding is UCS2");

			message[1] = malloc(data_dec_len * 4 + 1);
			char *ucs2 = (char*)ssResp->ussdStr;

			for (i = 0; i < data_dec_len; i += 2) {
				int c = (ucs2[i] << 8) | ucs2[1 + i];
				result += utf8_write(message[1], result, c);
			}
			message[1][result] = '\0';
			break;
		case 0xf:

			ALOGD("USSD Rx encoding is GSM7");

			asprintf(&message[1], "%s", (char*)ssResp->ussdStr);
		DEBUG_I("%s: message -  %s ", __func__, (char*)ssResp->ussdStr);
			message[1][data_dec_len] = '\0';
			break;

		default:

			ALOGD("USSD Rx encoding %x is unknown, assuming ASCII",ssResp->dcs);

			asprintf(&message[1], "%s", (char*)ssResp->ussdStr);
			DEBUG_I("%s: message -  %s ", __func__, (char*)ssResp->ussdStr);
			message[1][data_dec_len] = '\0';

			break;

	}


	ril_request_unsolicited(RIL_UNSOL_ON_USSD, message, sizeof(message));
}

void ipc_ss_ussd_ind_response(void* data)
{
	ALOGE("%s: implement me!", __func__);

	char *message[2];

	asprintf(&message[0], "%d", 2);
	ril_request_unsolicited(RIL_UNSOL_ON_USSD, message, sizeof(message));
}

void ipc_ss_error_response(void* data)
{
	ALOGE("%s: test me!", __func__);
	char *message[2];

	asprintf(&message[0], "%d", 2);
	ril_request_unsolicited(RIL_UNSOL_ON_USSD, message, sizeof(message));	

}

void ril_request_send_ussd(RIL_Token t, void *data, size_t datalen)
{
	ALOGE("%s: test me!", __func__);
	DEBUG_I("%s: data - %s , len = %d", __func__, (char *)data, datalen);	
	tapiSsSendUssd ussd_req;
	ussd_req.bUnknown = 0;
	ussd_req.dcs = 0xF;
	ussd_req.align[0] = 0;
	ussd_req.align[1] = 0;
	ussd_req.align[2] = 0;
	ussd_req.strLen = strlen((char *)data);
	memset(ussd_req.ussdStr, 0, strlen((char *)data));
	memcpy(ussd_req.ussdStr, data, strlen((char *)data));

	tapi_ss_send_ussd_string_request(&ussd_req);

	ril_request_complete(t, RIL_E_SUCCESS, NULL, 0);

}

void ril_request_cancel_ussd(RIL_Token t, void *data, size_t datalen)
{
	ALOGE("%s: implement me!", __func__);
}



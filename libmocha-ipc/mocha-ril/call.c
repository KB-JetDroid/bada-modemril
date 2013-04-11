/**
 * This file is part of mocha-ril.
 *
 * Copyright (C) 2010-2011 Joerie de Gram <j.de.gram@gmail.com>
 * Copyright (C) 2011-2012 Paul Kocialkowski <contact@paulk.fr>
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

#define LOG_TAG "RIL-Mocha-CALL"
#include <utils/Log.h>

#include "mocha-ril.h"
#include <tapi_call.h>

int num_entries, callId, callType;
char number[64];
tapiCallInfo* callInfo;
unsigned int call_state;

void ipc_call_incoming(void* data)
{
	ALOGE("%s: test me!", __func__);
	
	tapiCallInfo* callInfo = (tapiCallInfo*)(data);
	num_entries = 1;
	strcpy(number, callInfo->phoneNumber);
	callId = callInfo->callId;
	callType = callInfo->callType;
	call_state = RIL_CALL_INCOMING;
	ril_request_unsolicited(RIL_UNSOL_CALL_RING, NULL, 0);
	ril_request_unsolicited(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
}

void ipc_call_end(void* data)
{
	ALOGE("%s: test me!", __func__);

	num_entries = 0;
	ril_request_unsolicited(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);

}

void ipc_call_setup_ind(void* data)
{
	ALOGE("%s: test me!", __func__);

	callId = *(uint32_t *)((uint8_t *)(data));
	DEBUG_I("%s : callId = %d", __func__, callId);
	num_entries = 1;
	call_state = RIL_CALL_DIALING;
	ril_request_unsolicited(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
	ril_request_complete(ril_data.tokens.dial, RIL_E_SUCCESS, NULL, 0);

}
void ipc_call_connected_number_ind(void* data)
{
	ALOGE("%s: test me!", __func__);

	call_state = RIL_CALL_ACTIVE;
	ril_request_unsolicited(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);	
}

void ril_request_dial(RIL_Token t, void *data, size_t datalen)
{	
	ALOGE("%s: Test me!", __func__);

	tapiCallSetup *callSetup;
	RIL_Dial *dial;
	int clir;

	if (data == NULL || datalen < sizeof(RIL_Dial))
		goto error;

	dial = (RIL_Dial *) data;

	strcpy(number, dial->address);

	callSetup = (tapiCallSetup *)malloc(sizeof(tapiCallSetup));
	memset(callSetup, 0, sizeof(tapiCallSetup));

	callSetup->contextType = TAPI_CALL_CONTEXT_TYPE_VOICE;
	callSetup->bUsed = 1;
	callSetup->hCall = 3;
	callSetup->hClient = 0x41C8255C;//5C 25 C8 41
	callSetup->callNo = 0xFF;
	callSetup->bOriginated = 1;
	callSetup->nameMode = 2;
	strcpy(callSetup->callNum1, dial->address);
	callSetup->startTime = time(NULL);
	callSetup->endTime = time(NULL);
	callSetup->callType1 = TAPI_CALL_CALLTYPE_VOICE1;
	callSetup->callState = TAPI_CALL_STATE_ORIG;
	callSetup->unknown10 = 1;
	callSetup->unknown12 = 0x3A;
	callSetup->unknown14 = 1;
	callSetup->callType2 = TAPI_CALL_CALLTYPE_VOICE1;
	strcpy(callSetup->callNum2, dial->address);

	tapi_call_setup(callSetup);

	ril_data.tokens.dial = t;

	free(callSetup);
	return;

error:
	ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void ril_request_get_current_calls(RIL_Token t)
{
	ALOGE("%s: test me!", __func__);

	int i;

       if (num_entries == 0) {
		DEBUG_I("num_entries == 0");	
		ril_request_complete(t, RIL_E_SUCCESS, NULL, 0);
		return;
	}

	RIL_Call **calls = (RIL_Call **) malloc(num_entries * sizeof(RIL_Call *));

	for (i = 0; i < num_entries; i++) {
		RIL_Call *call = (RIL_Call *) malloc(sizeof(RIL_Call));
		call->state = call_state;
		call->index = callId + 1;
		call->toa = (strlen(number) > 0 && number[0] == '+') ? 145 : 129;
		call->isMpty = 0;
		call->isMT = 1;
		call->als = 0;
		call->isVoice  = 1;
		call->isVoicePrivacy = 0;
		call->number = number;
		call->numberPresentation = (strlen(number) > 0) ? 0 : 2;
		call->name = NULL;
		call->namePresentation = 2;
		call->uusInfo = NULL;
		calls[i] = call;

	}

	ril_request_complete(t, RIL_E_SUCCESS, calls, (num_entries * sizeof(RIL_Call *)));

	for (i = 0; i < num_entries; i++) {
		free(calls[i]);
	}
	free(calls);
}

	
void ril_request_hangup(RIL_Token t)
{
	ALOGE("%s: Test me!, callId = %d", __func__, callId);
	
	tapi_call_release(callType, callId, 0x0);
	num_entries = 0;

	/* FIXME: This should actually be sent based on the response from baseband */
	ril_request_complete(t, RIL_E_SUCCESS, NULL, 0);
}

void ril_request_answer(RIL_Token t)
{
	ALOGE("%s: Test me!", __func__);
	tapi_call_answer(callType, callId);
	call_state = RIL_CALL_ACTIVE;
	/* FIXME: This should actually be sent based on the response from baseband */
	ril_request_complete(t, RIL_E_SUCCESS, NULL, 0);

	/* FIXME: Do we really need to send this? */
	ril_request_unsolicited(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
}

/**
 * In: RIL_REQUEST_LAST_CALL_FAIL_CAUSE
 *   Reason why last call was terminated
 */
void ril_request_last_call_fail_cause(RIL_Token t)
{
	RIL_LastCallFailCause fail_cause;

	ALOGE("%s: Implement me!", __func__);
	
	fail_cause = CALL_FAIL_NORMAL;

	ril_request_complete(t, RIL_E_SUCCESS, &fail_cause, sizeof(RIL_LastCallFailCause));
}

void ril_request_dtmf(RIL_Token t, void *data, int length)
{
	ALOGE("%s: Implement me!", __func__);
	
	ril_request_complete(t, RIL_E_SUCCESS, NULL, 0);
}

void ril_request_dtmf_start(RIL_Token t, void *data, int length)
{
	ALOGE("%s: Implement me!", __func__);

	ril_request_complete(t, RIL_E_SUCCESS, NULL, 0);
}

void ril_request_dtmf_stop(RIL_Token t)
{
	ALOGE("%s: Implement me!", __func__);

	ril_request_complete(t, RIL_E_SUCCESS, NULL, 0);
}

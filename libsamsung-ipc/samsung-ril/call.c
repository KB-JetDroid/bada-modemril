/**
 * This file is part of samsung-ril.
 *
 * Copyright (C) 2010-2011 Joerie de Gram <j.de.gram@gmail.com>
 * Copyright (C) 2011-2012 Paul Kocialkowski <contact@paulk.fr>
 * Copyright (C) 2013 Dominik Marszk <dmarszk@gmail.com>
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

#define LOG_TAG "Mocha-RIL-CALL"
#include <utils/Log.h>

#include "samsung-ril.h"
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
	/* FIXME: Do we really need to send this? */
	ril_request_unsolicited(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);

}

void ipc_call_status(void* data)
{
	ALOGE("%s: Implement me!", __func__);
	ril_request_unsolicited(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
}

void ipc_call_end(void* data)
{
	ALOGE("%s: test me!", __func__);

	num_entries = 0;
	ril_request_unsolicited(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);

}


void ril_request_dial(RIL_Token t, void *data, size_t datalen)
{	
	ALOGE("%s: Implement me!", __func__);


	const RIL_Dial *dial = (const RIL_Dial*)data;
	int clir;

	strcpy(number, dial->address);
	num_entries = 1;
	call_state = RIL_CALL_DIALING;

	/* FIXME: This should actually be sent based on the response from baseband */
	ril_request_complete(t, RIL_E_SUCCESS, NULL, 0);
}

void ril_request_get_current_calls(RIL_Token t)
{
	int i;
	
	ALOGE("%s: test me!", __func__);

       if (num_entries == 0) {
		DEBUG_I("num_entries == 0");	
		ril_request_complete(t, RIL_E_SUCCESS, NULL, 0);
		return;
	}

	RIL_Call **calls = (RIL_Call **) malloc(num_entries * sizeof(RIL_Call *));

	for (i = 0; i < num_entries; i++) {
		RIL_Call *call = (RIL_Call *) malloc(sizeof(RIL_Call));
		call->state = call_state;
		call->index = 1;
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
	ALOGE("%s: Test me!", __func__);
	
	tapi_call_release(callType,callId, 0x0);
	num_entries = 0;

	/* FIXME: This should actually be sent based on the response from baseband */
	ril_request_complete(t, RIL_E_SUCCESS, NULL, 0);

	/* FIXME: Do we really need to send this? */
	ril_request_unsolicited(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
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

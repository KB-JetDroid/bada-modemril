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
#include <time.h>
#include <utils/Log.h>

#include "mocha-ril.h"
#include <tapi_call.h>

callContext* findActiveCall()
{
	int i;
	for(i = 0; i < MAX_CALLS; i++)
	{
		if(ril_data.calls[i] && ril_data.calls[i]->callId != 0xFFFFFFFF && 	ril_data.calls[i]->call_state == RIL_CALL_ACTIVE)
		{
			return ril_data.calls[i];
		}
	}
	ALOGE("Cannot find active call context!");
	return NULL;
}

callContext* newCallContext()
{
	int i;
	for(i = 0; i < MAX_CALLS; i++)
	{
		if(!ril_data.calls[i])
		{
			ril_data.calls[i] = calloc(1, sizeof(callContext));
			return ril_data.calls[i];
		}
	}
	ALOGE("No free call contexts!");
	return NULL;
}

callContext* findCallContext(uint32_t callId)
{
	int i;
	for(i = 0; i < MAX_CALLS; i++)
	{
		if(ril_data.calls[i] && ril_data.calls[i]->callId == callId)
		{
			return ril_data.calls[i];
		}
	}
	ALOGE("Call context with Id %d not found!", callId);
	return NULL;
}

void releaseCallContext(callContext* ptr)
{
	int i;
	for(i = 0; i < MAX_CALLS; i++)
	{
		if(ril_data.calls[i] == ptr)
		{
			free(ril_data.calls[i]);
			ril_data.calls[i] = NULL;
			return;
		}
	}
	ALOGE("Tried to release non registered call context!");
}

void ipc_call_incoming(void* data)
{

	ALOGE("%s: Test me!", __func__);

	uint8_t newCallState = RIL_CALL_INCOMING;
	int i;
	for(i = 0; i < MAX_CALLS; i++)
	{
		if(ril_data.calls[i] && ril_data.calls[i]->callId != 0xFFFFFFFF && 	(ril_data.calls[i]->call_state == RIL_CALL_ACTIVE || ril_data.calls[i]->call_state == RIL_CALL_HOLDING))
		{
			newCallState = RIL_CALL_WAITING;
			break;
		}
	}


	callContext* callCtxt = newCallContext();
	tapiCallInfo* callInfo = (tapiCallInfo*)(data);
	
	DEBUG_I("ipc_call_incoming: Incoming call received from %s", callInfo->phoneNumber);
	DEBUG_I("ipc_call_incoming: callId = %d", callInfo->callId );
	
	if(!callCtxt)
		return;
	strcpy(callCtxt->number, callInfo->phoneNumber);
	callCtxt->callId = callInfo->callId;
	callCtxt->callType = callInfo->callType;
	callCtxt->call_state = newCallState;
	callCtxt->bMT = 1;
	
	ril_request_unsolicited(RIL_UNSOL_CALL_RING, NULL, 0);
	ril_request_unsolicited(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
}

void ipc_call_end(void* data)
{
	ALOGE("%s: Test me!", __func__);
	callContext* callCtxt;
	tapiCallEnd* callEndInfo = (tapiCallEnd*)(data);
	DEBUG_I("%s : callId = %d, cause = %d", __func__, callEndInfo->callId, callEndInfo->cause);
	callCtxt = findCallContext(callEndInfo->callId);
	if(!callCtxt)
		return;
	if(callCtxt->token != 0)
	{
		ril_request_complete(callCtxt->token, RIL_E_SUCCESS, NULL, 0);
		releaseCallContext(callCtxt);
		return;
	}
	releaseCallContext(callCtxt);
	ril_request_unsolicited(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
}

void ipc_call_setup_ind(void* data)
{
	callContext* callCtxt;
	ALOGE("%s: Test me!", __func__);
	uint32_t callId = *(uint32_t *)(data);
	DEBUG_I("%s : callId = %d", __func__, callId);
	callCtxt = findCallContext(0xFFFFFFFF);
	if(!callCtxt)
		return;
	callCtxt->callId = callId;
	callCtxt = findCallContext(callId);
	if(callCtxt->token != 0)
	{
		ril_request_complete(callCtxt->token, RIL_E_SUCCESS, NULL, 0);
		callCtxt->token = 0;
	}
}

void ipc_call_alert(void* data)
{
	ALOGE("%s: Test me!", __func__);
	callContext* callCtxt;
	uint32_t callId = *(uint32_t *)(data);
	uint32_t bAudioOn = *(uint32_t *)((uint8_t*)(data) + 4);
	DEBUG_I("%s : callId = %d, bAudioOn = %d", __func__, callId, bAudioOn);
	callCtxt = findCallContext(callId);
	if(!callCtxt)
		return;
	callCtxt->call_state = RIL_CALL_ALERTING;
	ril_request_unsolicited(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
}

void ipc_call_connected(void* data)
{
	ALOGE("%s: Test me!", __func__);
	callContext* callCtxt;
	uint32_t callId = *(uint32_t *)(data);
	DEBUG_I("%s : callId = %d", __func__, callId);
	callCtxt = findCallContext(callId);
	if(!callCtxt)
		return;
	callCtxt->call_state = RIL_CALL_ACTIVE;
	if(callCtxt->token != 0)
	{
		ril_request_complete(callCtxt->token, RIL_E_SUCCESS, NULL, 0);
		callCtxt->token = 0;
	}
}

void ipc_call_connected_number_ind(void* data)
{
	ALOGE("%s: Test me!", __func__);
	callContext* callCtxt;
	uint32_t callId = *(uint32_t *)(data);
	DEBUG_I("%s : callId = %d", __func__, callId);
	callCtxt = findCallContext(callId);
	if(!callCtxt)
		return;
	callCtxt->call_state = RIL_CALL_ACTIVE;
	ril_request_unsolicited(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);	
}

void ipc_call_dtmf_start(void* data)
{
	ALOGE("%s: test me!", __func__);

	tapiDtmfCnf* dtmfCnf = (tapiDtmfCnf*)(data);

	if (dtmfCnf->reason != 0) {
		ALOGD("%s: Apparently, something went wrong with DTMF (code=0x%x)",__func__, dtmfCnf->reason);
		goto error;
	}

	ril_request_complete(ril_data.tokens.dtmf_start, RIL_E_SUCCESS, NULL, 0);

	return;

error:
	ALOGE("%s: Error!", __func__);
	ril_request_complete(ril_data.tokens.dtmf_start, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void ipc_call_dtmf_stop(void* data)
{
	ALOGE("%s: test me!", __func__);
	tapiDtmfCnf* dtmfCnf = (tapiDtmfCnf*)(data);

	if (dtmfCnf->reason != 0) {
		ALOGD("%s: Apparently, something went wrong with DTMF (code=0x%x)", __func__, dtmfCnf->reason);
		goto error;
	}

	ril_request_complete(ril_data.tokens.dtmf_stop, RIL_E_SUCCESS, NULL, 0);

	return;

error:
	ALOGE("%s: Error!", __func__);
	ril_request_complete(ril_data.tokens.dtmf_stop, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void ipc_call_hold(void* data)
{
	callContext* heldCtxt;
	tapiCallEnd* holdCnf = (tapiCallEnd*)(data);
	ALOGE("%s: test me!(callId:%d, cause:%d)", __func__, 
	holdCnf->callId, holdCnf->cause);
	
	heldCtxt = findCallContext(holdCnf->callId);
	if(!heldCtxt)
		return;

	heldCtxt->call_state = RIL_CALL_HOLDING;
	
	if(heldCtxt->token != 0)
	{
		ril_request_complete(heldCtxt->token, RIL_E_SUCCESS, NULL, 0);
		heldCtxt->token = 0;
	}
}

void ipc_call_swap(void* data)
{
	callContext* activatedCtxt, *heldCtxt;
	tapiSwapCnf* swapCnf = (tapiSwapCnf*)(data);
	ALOGE("%s: test me!(ActivatedCallNo:%d,HeldCallNo:%d, cause:%d)", __func__, 
	swapCnf->activatedCallId, swapCnf->heldCallId, swapCnf->cause);
	
	activatedCtxt = findCallContext(swapCnf->activatedCallId);
	heldCtxt = findCallContext(swapCnf->heldCallId);
	if(!activatedCtxt || !heldCtxt)
		return;

	activatedCtxt->call_state = RIL_CALL_ACTIVE;
	heldCtxt->call_state = RIL_CALL_HOLDING;
	
	if(heldCtxt->token != 0)
	{
		ril_request_complete(heldCtxt->token, RIL_E_SUCCESS, NULL, 0);
	}
	else if(activatedCtxt->token != 0)
	{
		ril_request_complete(activatedCtxt->token, RIL_E_SUCCESS, NULL, 0);
	}
	heldCtxt->token = 0;
	activatedCtxt->token = 0;
}

void ipc_call_activate(void* data)
{
	callContext* activatedCtxt;
	tapiCallEnd* activateCnf = (tapiCallEnd*)(data);
	ALOGE("%s: test me!(callNo:%d, cause:%d)", __func__, 
	activateCnf->callId, activateCnf->cause);
	
	activatedCtxt = findCallContext(activateCnf->callId);
	if(!activatedCtxt)
		return;

	activatedCtxt->call_state = RIL_CALL_ACTIVE;
	
	if(activatedCtxt->token != 0)
	{
		ril_request_complete(activatedCtxt->token, RIL_E_SUCCESS, NULL, 0);
		activatedCtxt->token = 0;
	}
}

void ipc_call_error(void* data)
{
	callContext* errorCtxt;
	tapiCallError* errorInd = (tapiCallError*)(data);
	ALOGE("%s: test me!(callNo:%d, error:%d)", __func__, errorInd->callId, errorInd->error);

	errorCtxt = findCallContext(errorInd->callId);
	if(!errorCtxt)
		return;

	if(errorCtxt->token != 0)
		ril_request_complete(errorCtxt->token, RIL_E_GENERIC_FAILURE, NULL, 0);
	releaseCallContext(errorCtxt);
	ril_request_unsolicited(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
}

void ril_request_dial(RIL_Token t, void *data, size_t datalen)
{	
	ALOGE("%s: Test me!", __func__);
	callContext* callCtxt;
	tapiCallSetup *callSetup;
	RIL_Dial *dial;
	int clir;

	if (findCallContext(0xFFFFFFFF) != NULL || data == NULL || datalen < sizeof(RIL_Dial))
		goto error;

	dial = (RIL_Dial *) data;
	callCtxt = newCallContext();
	if(!callCtxt)
		goto error;
	
	callCtxt->callId = 0xFFFFFFFF;
	callCtxt->call_state = RIL_CALL_DIALING;
	callCtxt->bMT = 0;
	strcpy(callCtxt->number, dial->address);

	callSetup = (tapiCallSetup *)malloc(sizeof(tapiCallSetup));
	memset(callSetup, 0, sizeof(tapiCallSetup));

	callSetup->contextType = TAPI_CALL_CONTEXT_TYPE_VOICE;
	callSetup->bUsed = 1;
	callSetup->hCall = 3;
	callSetup->hClient = 0x0; //0x41C8255C;
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

	if (callCtxt->token != 0)
		/* pass error to the current request, another one is pending */
		goto error;

	callCtxt->token = t;

	free(callSetup);
	return;

error:
	ALOGE("%s: Error!", __func__);
	ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void ril_request_get_current_calls(RIL_Token t)
{
	ALOGE("%s: test me!", __func__);

	int i, j;

	RIL_Call **calls = NULL;
	j = 0;
	for (i = 0; i < MAX_CALLS; i++) {
		if(ril_data.calls[i] == NULL || ril_data.calls[i]->callId == 0xFFFFFFFF)
			continue;
		RIL_Call *call = (RIL_Call *) malloc(sizeof(RIL_Call));
		call->state = ril_data.calls[i]->call_state;
		call->index = i + 1;
		call->toa = (strlen(ril_data.calls[i]->number) > 0 && ril_data.calls[i]->number[0] == '+') ? 145 : 129;
		call->isMpty = 0;
		call->isMT = ril_data.calls[i]->bMT;
		call->als = 0;
		call->isVoice  = 1;
		call->isVoicePrivacy = 0;
		call->number = ril_data.calls[i]->number;
		call->numberPresentation = (strlen(ril_data.calls[i]->number) > 0) ? 0 : 2;
		call->name = NULL;
		call->namePresentation = 2;
		call->uusInfo = NULL;
		calls = (RIL_Call **) realloc(calls, (j + 1) * sizeof(RIL_Call *));
		calls[j++] = call;
	}

	ril_request_complete(t, RIL_E_SUCCESS, calls, (j * sizeof(RIL_Call *)));

	for (i = 0; i < j; i++) {
		free(calls[i]);
	}
	free(calls);
}

void ril_request_hangup(RIL_Token t, void *data, size_t datalen)
{
	ALOGE("%s: Test me!", __func__);
	callContext* callCtxt;
	if(datalen < 4)
		goto error;
	callCtxt = ril_data.calls[*(int *)(data) - 1];
	if(!callCtxt)
		goto error;
	
	if(callCtxt->token != 0)
		/* pass error to the current request, another one is pending */
		goto error;

	callCtxt->token = t;

	tapi_call_release(callCtxt->callType, callCtxt->callId, 0x0);
	
	return;
error:
	ALOGE("%s: Error!", __func__);
	ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void ril_request_hangup_waiting_or_background(RIL_Token t)
{
	int i;
	ALOGE("%s: Test me!", __func__);
	
	for(i = 0; i < MAX_CALLS; i++)
	{
		if(ril_data.calls[i] && ril_data.calls[i]->callId != 0xFFFFFFFF && 
			(ril_data.calls[i]->call_state == RIL_CALL_INCOMING || ril_data.calls[i]->call_state == RIL_CALL_WAITING || ril_data.calls[i]->call_state == RIL_CALL_HOLDING))
		{
			ALOGE("%s: hanging up callId = %d", __func__, ril_data.calls[i]->callId);
			if (ril_data.calls[i]->token != 0)
				/* pass error to the current request, another one is pending */
				goto error;
			ril_data.calls[i]->token = t;
			tapi_call_release(ril_data.calls[i]->callType, ril_data.calls[i]->callId, 0x0);
			break;
		}
	}
	return;
error:
	ALOGE("%s: Error!", __func__);
	ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}	

void ril_request_hangup_foreground_resume_background(RIL_Token t)
{
	int i;
	uint32_t activeId = 0xFFFFFFFF;
	uint32_t holdId = 0xFFFFFFFF;
	uint32_t callType = 0xFFFFFFFF;
	ALOGE("%s: Test me!", __func__);

	for(i = 0; i < MAX_CALLS; i++)
	{
		if(ril_data.calls[i] && ril_data.calls[i]->callId != 0xFFFFFFFF)
		{
			if(ril_data.calls[i]->call_state == RIL_CALL_ACTIVE)
			{
				activeId = ril_data.calls[i]->callId;
				callType = ril_data.calls[i]->callType;
				if (ril_data.calls[i]->token != 0)
					/* pass error to the current request, another one is pending */
					goto error;
				ril_data.calls[i]->token = t;
				ALOGE("%s: active callId = %d", __func__, activeId);
			}
			if(ril_data.calls[i]->call_state == RIL_CALL_HOLDING)
			{
				holdId = ril_data.calls[i]->callId;
				if (ril_data.calls[i]->token != 0)
					/* pass error to the current request, another one is pending */
					goto error;
				ril_data.calls[i]->token = t;
				ALOGE("%s: holding callId = %d", __func__, activeId);
			}
		}
	}
	if(activeId != 0xFFFFFFFF && holdId != 0xFFFFFFFF)
	{
		ALOGE("%s: active/hangup callId = %d", __func__, activeId);
		ALOGE("%s: hold/active callId = %d", __func__, holdId);
		/* we have to remove token for active call,
			to receive correct end of function after completion tapi_call_activate */
		callContext* callCtxt = findCallContext(activeId);
		if(!callCtxt)
			goto error;
		callCtxt->token = 0;
		tapi_call_release(callType, activeId, 0x0);
		usleep(300000);
		tapi_call_activate(holdId);
	}
	else if(activeId != 0xFFFFFFFF)
	{
		ALOGE("%s: active/hangup callId = %d", __func__, activeId);
		tapi_call_release(callType, activeId, 0x0);
	}
	else if(holdId != 0xFFFFFFFF)
	{
		ALOGE("%s: hold/hangup callId = %d", __func__, holdId);
		tapi_call_release(callType, holdId, 0x0);
	}
	return;
error:
	ALOGE("%s: Error!", __func__);
	ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void ril_request_answer(RIL_Token t)
{
	int i;
	ALOGE("%s: Test me!", __func__);
	
	for(i = 0; i < MAX_CALLS; i++)
	{
		if(ril_data.calls[i] && ril_data.calls[i]->callId != 0xFFFFFFFF && ril_data.calls[i]->call_state == RIL_CALL_INCOMING)
		{
			ALOGE("%s: answering callId = %d", __func__, ril_data.calls[i]->callId);
			if (ril_data.calls[i]->token != 0)
				/* pass error to the current request, another one is pending */
				goto error;
			ril_data.calls[i]->token = t;
			tapi_call_answer(ril_data.calls[i]->callType, ril_data.calls[i]->callId);
		}
	}
	return;
error:
	ALOGE("%s: Error!", __func__);
	ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
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
	ALOGE("%s: test me!", __func__);
	unsigned char tone;
	callContext* activeCall = findActiveCall();
	
	if (activeCall == NULL || data == NULL || length < (int) sizeof(unsigned char))
		goto error;

	tone = *((unsigned char *) data);

	if (ril_data.state.dtmf_tone != 0) {
		ALOGD("%s: Another tone wasn't stopped, stopping it before anything", __func__);
		tapi_stop_dtmf(activeCall->callId);
		usleep(300);
	}

	ril_data.state.dtmf_tone = tone;

	tapi_start_dtmf(activeCall->callId, tone);

	ril_data.tokens.dtmf_start = t;

	return;

error:
	ALOGE("%s: Error!", __func__);
	ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void ril_request_dtmf_stop(RIL_Token t)
{
	ALOGE("%s: test me!", __func__);

	callContext* activeCall = findActiveCall();
	
	if (activeCall == NULL)
		goto error;
		
	ril_data.state.dtmf_tone = 0;

	tapi_stop_dtmf(activeCall->callId);

	ril_data.tokens.dtmf_stop = t;
error:
	ALOGE("%s: Error!", __func__);
	ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

void ril_request_switch_waiting_or_holding_and_active(RIL_Token t)
{
	int i;
	uint32_t activeId = 0xFFFFFFFF;
	uint32_t holdId = 0xFFFFFFFF;
	uint32_t waitId = 0xFFFFFFFF;
	uint32_t callType = 0xFFFFFFFF;
	for(i = 0; i < MAX_CALLS; i++)
	{
		if(ril_data.calls[i] && ril_data.calls[i]->callId != 0xFFFFFFFF)
		{
			if(ril_data.calls[i]->call_state == RIL_CALL_ACTIVE)
			{
				activeId = ril_data.calls[i]->callId;
				if (ril_data.calls[i]->token != 0)
					/* pass error to the current request, another one is pending */
					goto error;
				ril_data.calls[i]->token = t;
				ALOGE("%s: active callId = %d", __func__, activeId);			
			}
			if(ril_data.calls[i]->call_state == RIL_CALL_HOLDING)
			{
				holdId = ril_data.calls[i]->callId;
				if (ril_data.calls[i]->token != 0)
					/* pass error to the current request, another one is pending */
					goto error;
				ril_data.calls[i]->token = t;
				ALOGE("%s: hold callId = %d", __func__, holdId);
			}
			if(ril_data.calls[i]->call_state == RIL_CALL_WAITING)
			{
				waitId = ril_data.calls[i]->callId;
				callType = ril_data.calls[i]->callType;
				if (ril_data.calls[i]->token != 0)
					/* pass error to the current request, another one is pending */
					goto error;
				ril_data.calls[i]->token = t;
				ALOGE("%s: wait callId = %d", __func__, waitId);
			}
		}
	}

	if(activeId != 0xFFFFFFFF && holdId != 0xFFFFFFFF)
	{
		ALOGE("%s: active/holding callId = %d", __func__, activeId);
		ALOGE("%s: hold/activating callId = %d", __func__, holdId);
		tapi_calls_swap(activeId, holdId);
	}
	else if(activeId != 0xFFFFFFFF && waitId != 0xFFFFFFFF)
	{
		ALOGE("%s: active/holding callId = %d", __func__, activeId);
		ALOGE("%s: waiting/activating callId = %d", __func__, waitId);
		/* we have to remove token for waiting call,
			to receive correct end of function after completion tapi_call_swap */
		callContext* callCtxt = findCallContext(waitId);
		if(!callCtxt)
			goto error;
		callCtxt->token = 0;
		tapi_call_hold(waitId);

		usleep(500000);
		tapi_calls_swap(activeId, waitId);
	}
	else if(holdId != 0xFFFFFFFF && waitId != 0xFFFFFFFF)
	{
		ALOGE("%s: hold/holding callId = %d", __func__, holdId);
		ALOGE("%s: wait/activating callId = %d", __func__, waitId);
		tapi_call_answer(callType, waitId);
	}
	else if(activeId != 0xFFFFFFFF)
	{
		ALOGE("%s: holding callId = %d", __func__, activeId);
		tapi_call_hold(activeId);
	}
	else if(holdId != 0xFFFFFFFF)
	{
		ALOGE("%s: active/hangup callId = %d", __func__, activeId);
		tapi_call_release(callType, activeId, 0x0);
	}
	return;

error:
	ALOGE("%s: Error!", __func__);
	ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
}

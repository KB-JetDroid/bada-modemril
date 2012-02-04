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

#define LOG_TAG "RIL-CALL"
#include <utils/Log.h>

#include "samsung-ril.h"

/**
 * Format conversion utils
 */

unsigned char ipc2ril_call_list_entry_state(unsigned char call_state)
{
	switch(call_state) {
		case IPC_CALL_LIST_ENTRY_STATE_ACTIVE:
			return RIL_CALL_ACTIVE;
		case IPC_CALL_LIST_ENTRY_STATE_HOLDING:
			return RIL_CALL_HOLDING;
		case IPC_CALL_LIST_ENTRY_STATE_DIALING:
			return RIL_CALL_DIALING;
		case IPC_CALL_LIST_ENTRY_STATE_ALERTING:
			return RIL_CALL_ALERTING;
		case IPC_CALL_LIST_ENTRY_STATE_INCOMING:
			return RIL_CALL_INCOMING;
		case IPC_CALL_LIST_ENTRY_STATE_WAITING:
			return RIL_CALL_WAITING;
		default:
			LOGE("Unknown IPC_CALL_LIST_ENTRY_STATE!");
			return -1;
	}
}

/**
 * In: RIL_UNSOL_CALL_RING
 *   Ring indication for an incoming call (eg, RING or CRING event).
 */
void ipc_call_incoming(struct ipc_message_info *info)
{
	RIL_onUnsolicitedResponse(RIL_UNSOL_CALL_RING, NULL, 0);

	/* FIXME: Do we really need to send this? */
	RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
}

/**
 * In: IPC_CALL_STATUS
 *   Indicates that a call's status has changed
 *
 * RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED
 *   Indicate when call state has changed
 */
void ipc_call_status(struct ipc_message_info *info)
{
	RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
}

/**
 * In: RIL_REQUEST_DIAL
 *   Initiate voice call
 *
 * Out: IPC_CALL_OUTGOING
 */
void ril_request_dial(RIL_Token t, void *data, size_t datalen)
{
	const RIL_Dial *dial = (const RIL_Dial*)data;
	struct ipc_call_outgoing call;
	int clir;

	if(strlen(dial->address) > sizeof(call.number)) {
		printf("Outgoing call number too long\n");
		return;
	}

	/* FIXME: separate method? */
	switch(dial->clir) {
		case 0:
			clir = IPC_CALL_IDENTITY_DEFAULT;
			break;
		case 1:
			clir = IPC_CALL_IDENTITY_SHOW;
			break;
		case 2:
			clir = IPC_CALL_IDENTITY_HIDE;
			break;
		default:
			clir = IPC_CALL_IDENTITY_DEFAULT;
			break;
	}

	memset(&call, 0x00, sizeof(call));

	call.type = IPC_CALL_TYPE_VOICE;
	call.identity = clir;
	call.prefix = dial->address[0] == '+' ? IPC_CALL_PREFIX_INTL : IPC_CALL_PREFIX_NONE;

	call.length = strlen(dial->address);
	memcpy(call.number, dial->address, strlen(dial->address));

	ipc_fmt_send(IPC_CALL_OUTGOING, IPC_TYPE_EXEC, (unsigned char *) &call, sizeof(call), reqGetId(t));
	
	/* FIXME: This should actually be sent based on the response from baseband */
	RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}

/**
 * In: RIL_REQUEST_GET_CURRENT_CALLS
 *   Requests current call list
 *
 * Out: IPC_CALL_LIST GET
 *   Requests a list of active calls
 */
void ril_request_get_current_calls(RIL_Token t)
{
	ipc_fmt_send_get(IPC_CALL_LIST, reqGetId(t));
}

/**
 * In: IPC_CALL_LIST GET
 *   Provides a list of active calls
 *
 * Out: RIL_REQUEST_GET_CURRENT_CALLS
 *   Requests current call list
 */
void ipc_call_list(struct ipc_message_info *info)
{
	struct ipc_call_list_entry *entry;
	unsigned char num_entries;
	char *number, *number_ril;
	int i;

	num_entries = *((unsigned char *) info->data);
	entry = (struct ipc_call_list_entry *) ((char *) info->data + 1);

	RIL_Call **calls = (RIL_Call **) malloc(num_entries * sizeof(RIL_Call *));

	for(i = 0; i < num_entries; i++) {
		RIL_Call *call = (RIL_Call *) malloc(sizeof(RIL_Call));

		/* Number is located after call list entry */
		number = ((char *) entry) + sizeof(*entry);

		number_ril = (char *) malloc(entry->number_len + 1);
		memset(number_ril, 0, (entry->number_len + 1));
		memcpy(number_ril, number, entry->number_len);

		call->state = ipc2ril_call_list_entry_state(entry->state);
		call->index = (entry->idx+1);
		call->toa = (entry->number_len > 0 && number[0] == '+') ? 145 : 129;
		call->isMpty = entry->mpty;
		call->isMT = (entry->term == IPC_CALL_TERM_MT);
		call->als = 0;
		call->isVoice  = (entry->type == IPC_CALL_TYPE_VOICE);
		call->isVoicePrivacy = 0;
		call->number = number_ril;
		call->numberPresentation = (entry->number_len > 0) ? 0 : 2;
		call->name = NULL;
		call->namePresentation = 2;
		call->uusInfo = NULL;

		calls[i] = call;

		/* Next entry after current number */
		entry = (struct ipc_call_list_entry *) (number + entry->number_len);
	}

	RIL_onRequestComplete(reqGetToken(info->aseq), RIL_E_SUCCESS, calls, (num_entries * sizeof(RIL_Call *)));

	for(i = 0; i < num_entries; i++) {
		free(calls[i]);
	}

	free(calls);
}

/**
 * In: RIL_REQUEST_HANGUP
 *   Hang up a specific line (like AT+CHLD=1x)
 *
 * Out: IPC_CALL_RELEASE EXEC
 */
void ril_request_hangup(RIL_Token t)
{
	ipc_fmt_send_exec(IPC_CALL_RELEASE, reqGetId(t));

	/* FIXME: This should actually be sent based on the response from baseband */
	RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);

	/* FIXME: Do we really need to send this? */
	RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
}

/**
 * In: RIL_REQUEST_ANSWER
 *   Answer incoming call
 *
 * Out: IPC_CALL_ANSWER
 */
void ril_request_answer(RIL_Token t)
{
	ipc_fmt_send_exec(IPC_CALL_ANSWER, reqGetId(t));

	/* FIXME: This should actually be sent based on the response from baseband */
	RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);

	/* FIXME: Do we really need to send this? */
	RIL_onUnsolicitedResponse(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, NULL, 0);
}

/**
 * In: IPC_CALL_BURST_DTMF
 *   Send DTMF burst. RILJ only sends 1 DTMF tone to send at a time.
 * 
 * Out: IPC_CALL_BURST_DTMF
 *   It should be possible to send multiple DTMF tones at once in this message.
 *   First byte should be DTMF tones count.
 */
void ril_request_dtmf(RIL_Token t, void *data, int length)
{
	struct ipc_call_cont_dtmf cont_dtmf;

	unsigned char *burst;
	int burst_len;

	unsigned char dtmf_count = 1;
	int i;

	burst_len = sizeof(struct ipc_call_cont_dtmf) * dtmf_count + 1;
	burst = malloc(burst_len);
	memset(burst, 0, burst_len);

	burst[0] = dtmf_count;

	for(i=0 ; i < dtmf_count ; i++) {
		// Apparently, it's possible to set multiple DTMF tones on this message

		cont_dtmf.state = IPC_CALL_DTMF_STATE_START;
		cont_dtmf.tone = ((char *) data)[0];

		memcpy(burst + 1 + sizeof(struct ipc_call_cont_dtmf) * i, &cont_dtmf, sizeof(cont_dtmf));
	}

	ipc_gen_phone_res_expect_to_abort(reqGetId(t), IPC_CALL_BURST_DTMF);

	ipc_fmt_send(IPC_CALL_BURST_DTMF, IPC_TYPE_EXEC, (void *) burst, burst_len, reqGetId(t));

	free(burst);
}

void ipc_call_burst_dtmf(struct ipc_message_info *info)
{
	unsigned char ret = *((unsigned char *) info->data);

	// This apparently should return 1, or perhaps that is the DTMF tones count
	if(ret == 0) {
		LOGD("Apparently, something went wrong with DTMF burst");

		RIL_onRequestComplete(reqGetToken(info->aseq), RIL_E_GENERIC_FAILURE, NULL, 0);
	}

	RIL_onRequestComplete(reqGetToken(info->aseq), RIL_E_SUCCESS, NULL, 0);
}

void ril_request_dtmf_start(RIL_Token t, void *data, int length)
{
	//TODO: Check if there is already a DTMF going on and cancel it if so

	struct ipc_call_cont_dtmf cont_dtmf;
	cont_dtmf.state = IPC_CALL_DTMF_STATE_START;
	cont_dtmf.tone = ((char *)data)[0];

	ipc_fmt_send(IPC_CALL_CONT_DTMF, IPC_TYPE_SET, (void *) &cont_dtmf, sizeof(cont_dtmf), reqGetId(t));

	RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}

void ril_request_dtmf_stop(RIL_Token t)
{
	struct ipc_call_cont_dtmf cont_dtmf;
	cont_dtmf.state = IPC_CALL_DTMF_STATE_STOP;
	cont_dtmf.tone = 0;

	ipc_fmt_send(IPC_CALL_CONT_DTMF, IPC_TYPE_SET, (void *) &cont_dtmf, sizeof(cont_dtmf), reqGetId(t));

	RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
}

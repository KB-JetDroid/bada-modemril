/**
 * This file is part of samsung-ril.
 *
 * Copyright (C) 2010-2011 Joerie de Gram <j.de.gram@gmail.com>
 * Copyright (C) 2011 Paul Kocialkowski <contact@oaulk.fr>
 * Copyright (C) 2012 KB <kbjetdroid@gmail.com>
 * Copyright (C) 2012 Dominik Marszk <dmarszk@gmail.com>
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

#include <time.h>
#include <pthread.h>

#define LOG_TAG "RIL"
#include <utils/Log.h>
#include <telephony/ril.h>

#include "samsung-ril.h"
#include "util.h"

#include <fm.h>
#include <drv.h>
#include <tapi.h>
#include <proto.h>
#include <sim.h>

#define RIL_VERSION_STRING "Samsung RIL"

/**
 * Samsung-RIL TODO:
 * Everything? :]
 */

/**
 * RIL global vars
 */

struct ril_client *ipc_packet_client;
struct ril_client *srs_client;

const struct RIL_Env *ril_env;
struct ril_state ril_state;


/**
 * RIL request token
 */

struct ril_request_token ril_requests_tokens[0x100];
int ril_request_id = 0;

void ril_requests_tokens_init(void)
{
	memset(ril_requests_tokens, 0, sizeof(struct ril_request_token) * 0x100);
}

int ril_request_id_new(void)
{
	ril_request_id++;
	ril_request_id %= 0x100;
	return ril_request_id;
}

int ril_request_reg_id(RIL_Token token)
{
	int id = ril_request_id_new();

	ril_requests_tokens[id].token = token;
	ril_requests_tokens[id].canceled = 0;

	return id;
}

int ril_request_get_id(RIL_Token token)
{
	int i;

	for(i=0 ; i < 0x100 ; i++)
		if(ril_requests_tokens[i].token == token)
			return i;

	// If the token isn't registered yet, register it
	return ril_request_reg_id(token);
}

RIL_Token ril_request_get_token(int id)
{
	return ril_requests_tokens[id].token;
}

int ril_request_get_canceled(RIL_Token token)
{
	int id;

	id = ril_request_get_id(token);

	if(ril_requests_tokens[id].canceled > 0)
		return 1;
	else
		return 0;
}

void ril_request_set_canceled(RIL_Token token, int canceled)
{
	int id;

	id = ril_request_get_id(token);

	ril_requests_tokens[id].canceled = canceled;
}

void RIL_onRequestComplete(RIL_Token t, RIL_Errno e, void *response, size_t responselen)
{
	if(!ril_request_get_canceled(t))
		RIL_onRequestCompleteReal(t, e, response, responselen);
	else
		RIL_onRequestCompleteReal(t, RIL_E_CANCELLED, response, responselen);
}

/**
 * RIL tokens
 * this should be called after syssec data loading and after receiving system info from modem
 */

void ril_tokens_check(void)
{
	if(ril_state.tokens.baseband_version != 0) {
		if(ril_state.radio_state != RADIO_STATE_OFF) {
			ril_request_baseband_version(ril_state.tokens.baseband_version);
			ril_state.tokens.baseband_version = 0;
		}
	}
	if(ril_state.tokens.get_imei != 0) {
		if(cached_imei[0] != 0x00) {
			ril_request_get_imei(ril_state.tokens.get_imei);
			ril_state.tokens.get_imei = 0;
		}
	}
}


void srs_dispatch(struct srs_message *message)
{
	switch(message->command) {
		case SRS_CONTROL_PING:
			srs_control_ping(message);
			break;
		case SRS_SND_SET_CALL_CLOCK_SYNC:
//			srs_snd_set_call_clock_sync(message);
			break;
		case SRS_SND_SET_CALL_VOLUME:
//			srs_snd_set_call_volume(message);
			break;
		case SRS_SND_SET_CALL_AUDIO_PATH:
//			srs_snd_set_call_audio_path(message);
			break;
		default:
			ALOGD("Unhandled command: (%04x)", message->command);
			break;
	}
}

int ril_modem_check(void)
{
	if(ipc_packet_client == NULL)
		return -1;

	if(ipc_packet_client->state != RIL_CLIENT_READY)
		return -1;

	return 0;
}

void onRequest(int request, void *data, size_t datalen, RIL_Token t)
{
	if(ril_modem_check() < 0)
		RIL_onRequestComplete(t, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);

	switch(request) {
		/* MISC */
		case RIL_REQUEST_GET_IMEI:
			ril_request_get_imei(t);
			break;
		case RIL_REQUEST_GET_IMSI:
			ril_request_get_imsi(t);
			break;
#if 0
		/* PWR */
		case RIL_REQUEST_RADIO_POWER:
			ril_request_radio_power(t, data, datalen);
			break;
		case RIL_REQUEST_BASEBAND_VERSION:
			ril_request_baseband_version(t);
			break;
		/* SAT */
		case RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE:
			requestSatSendTerminalResponse(t, data, datalen);
			break;
		case RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND:
			requestSatSendEnvelopeCommand(t, data, datalen);
			break;
		case RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM:
			RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
			break;
		/* SIM */
		case RIL_REQUEST_GET_SIM_STATUS:
			ril_request_sim_status(t);
			break;
		case RIL_REQUEST_SIM_IO:
			ril_request_sim_io(t, data, datalen);
			break;
		case RIL_REQUEST_ENTER_SIM_PIN:
			ril_request_enter_sim_pin(t, data, datalen);
			break;
		case RIL_REQUEST_QUERY_FACILITY_LOCK:
			ril_request_query_facility_lock(t, data, datalen);
			break;
		case RIL_REQUEST_SET_FACILITY_LOCK:
			ril_request_set_facility_lock(t, data, datalen);
			break;
		/* NET */
		case RIL_REQUEST_OPERATOR:
			ril_request_operator(t);
			break;
		case RIL_REQUEST_REGISTRATION_STATE:
			ril_request_registration_state(t);
			break;
		case RIL_REQUEST_GPRS_REGISTRATION_STATE:
			ril_request_gprs_registration_state(t);
			break;
		case RIL_REQUEST_QUERY_AVAILABLE_NETWORKS:
			ril_request_query_available_networks(t);
			break;
		case RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE:
			ril_request_query_network_selection_mode(t);
			break;
		case RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE:
			ril_request_get_preferred_network_type(t);
			break;
		case RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE:
			ril_request_set_preffered_network_type(t, data, datalen);
			break;
		/* SMS */
		case RIL_REQUEST_SEND_SMS:
			ril_request_send_sms(t, data, datalen);
			break;
		case RIL_REQUEST_SEND_SMS_EXPECT_MORE:
			ril_request_send_sms_expect_more(t, data, datalen);
			break;
		case RIL_REQUEST_SMS_ACKNOWLEDGE:
			ril_request_sms_acknowledge(t, data, datalen);
			break;
		/* CALL */
		case RIL_REQUEST_DIAL:
			ril_request_dial(t, data, datalen);
			break;
		case RIL_REQUEST_GET_CURRENT_CALLS:
			ril_request_get_current_calls(t);
			break;
		case RIL_REQUEST_HANGUP:
		case RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND:
		case RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND:
			ril_request_hangup(t);
			break;
		case RIL_REQUEST_ANSWER:
			ril_request_answer(t);
			break;
		case RIL_REQUEST_DTMF:
			ril_request_dtmf(t, data, datalen);
                       break;
		case RIL_REQUEST_DTMF_START:
			ril_request_dtmf_start(t, data, datalen);
                       break;
		case RIL_REQUEST_DTMF_STOP:
			ril_request_dtmf_stop(t);
                       break;
		/* GPRS */
		case RIL_REQUEST_SETUP_DATA_CALL:
			ril_request_setup_data_call(t, data, datalen);
			break;
		case RIL_REQUEST_DEACTIVATE_DATA_CALL:
			ril_request_deactivate_data_call(t, data, datalen);
			break;
		/* SND */
		case RIL_REQUEST_SET_MUTE:
			ril_request_set_mute(t, data, datalen);
		/* OTHER */
		case RIL_REQUEST_SCREEN_STATE:
			/* This doesn't affect anything */
			RIL_onRequestComplete(t, RIL_E_SUCCESS, NULL, 0);
			break;
#endif
		default:
			ALOGE("Request not implemented: %d", request);
			RIL_onRequestComplete(t, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
			break;
	}
}

/**
 * RILJ related functions
 */

RIL_RadioState currentState()
{
	return ril_state.radio_state;
}

int onSupports(int requestCode)
{
	switch(requestCode) {
		default:
			return 1;
	}
}

void onCancel(RIL_Token t)
{
	ril_request_set_canceled(t, 1);
}

const char *getVersion(void)
{
	return RIL_VERSION_STRING;
}

/**
 * RIL init function
 */

void ril_install_ipc_callbacks(void)
{
	ipc_register_ril_cb(CP_SYSTEM_START, ipc_cp_system_start);
}
 
void ril_globals_init(void)
{
	memset(&ril_state, 0, sizeof(ril_state));
	memset(&(ril_state.tokens), 0, sizeof(struct ril_tokens));

	ril_requests_tokens_init();
//	ipc_gen_phone_res_expects_init();
//	ril_request_sms_init();
//	ipc_sms_tpid_queue_init();
}

void ril_state_lpm(void)
{
	ril_state.radio_state = RADIO_STATE_OFF;
	ril_state.power_mode = POWER_MODE_LPM;
}


static const RIL_RadioFunctions ril_ops = {
	RIL_VERSION,
	onRequest,
	currentState,
	onSupports,
	onCancel,
	getVersion
};

const RIL_RadioFunctions *RIL_Init(const struct RIL_Env *env, int argc, char **argv)
{
	int rc;

	ril_env = env;

	ipc_init();
	ril_globals_init();
	ril_state_lpm();
	ril_install_ipc_callbacks();

	ALOGD("Creating IPC client");

	ipc_packet_client = ril_client_new(&ipc_client_funcs);
	rc = ril_client_create(ipc_packet_client);

	if(rc < 0) {
		ALOGE("IPC client creation failed.");
		goto srs;
	}

	rc = ril_client_thread_start(ipc_packet_client);

	if(rc < 0) {
		ALOGE("IPC thread creation failed.");
		goto srs;
	}

	ALOGD("IPC client ready");

srs:
	ALOGD("Creating SRS client");

	srs_client = ril_client_new(&srs_client_funcs);
	rc = ril_client_create(srs_client);

	if(rc < 0) {
		ALOGE("SRS client creation failed.");
		goto end;
	}

	rc = ril_client_thread_start(srs_client);

	if(rc < 0) {
		ALOGE("SRS thread creation failed.");
		goto end;
	}

	ALOGD("SRS client ready");

	tapi_init();
	proto_startup();
	lbs_init();
end:
	return &ril_ops;
}

int main(int argc, char *argv[])
{
	return 0;
}


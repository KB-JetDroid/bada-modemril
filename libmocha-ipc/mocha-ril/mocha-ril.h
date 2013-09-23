/**
 * This file is part of mocha-ril.
 *
 * Copyright (C) 2010-2011 Joerie de Gram <j.de.gram@gmail.com>
 * Copyright (C) 2011-2012 Paul Kocialkowski <contact@paulk.fr>
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

#ifndef _SAMSUNG_RIL_H_
#define _SAMSUNG_RIL_H_

#include <pthread.h>

#include <utils/Log.h>
#include <telephony/ril.h>

#include <radio.h>

#include "ipc.h"
#include "srs.h"

#include <tapi_network.h>
#include <tapi_call.h>

/**
 * Defines
 */

#define SAMSUNG_RIL_VERSION 6

#define RIL_VERSION_STRING "Samsung RIL"

#define RIL_LOCK() pthread_mutex_lock(&ril_data.mutex)
#define RIL_UNLOCK() pthread_mutex_unlock(&ril_data.mutex)
#define RIL_CLIENT_LOCK(client) pthread_mutex_lock(&(client->mutex))
#define RIL_CLIENT_UNLOCK(client) pthread_mutex_unlock(&(client->mutex))

#define RIL_TOKEN_DATA_WAITING	(RIL_Token) 0xff

/**
 * RIL structures
 */

struct ril_client;
struct ril_token;
struct ril_tokens;
struct ril_state;


/**
 * RIL client
 */

struct ril_client;

struct ril_client_funcs {
	int (*create)(struct ril_client *client);
	int (*destroy)(struct ril_client *client);
	int (*read_loop)(struct ril_client *client);
};

typedef enum {
	RIL_CLIENT_NULL		= 0,
	RIL_CLIENT_CREATED	= 1,
	RIL_CLIENT_READY	= 2,
	RIL_CLIENT_DESTROYED	= 3,
	RIL_CLIENT_ERROR	= 4,
} ril_client_state;

struct ril_client {
	struct ril_client_funcs funcs;
	ril_client_state state;

	void *data;

	pthread_t thread;
	pthread_mutex_t mutex;
};

struct ril_client *ril_client_new(struct ril_client_funcs *client_funcs);
int ril_client_free(struct ril_client *client);
int ril_client_create(struct ril_client *client);
int ril_client_destroy(struct ril_client *client);
int ril_client_thread_start(struct ril_client *client);

/**
 * RIL requests
 */

struct ril_request_info {
	RIL_Token token;
	int id;
	int canceled;
};

int ril_request_id_get(void);
int ril_request_id_set(int id);
int ril_request_register(RIL_Token t, int id);
void ril_request_unregister(struct ril_request_info *request);
struct ril_request_info *ril_request_info_find_id(int id);
struct ril_request_info *ril_request_info_find_token(RIL_Token t);
int ril_request_set_canceled(RIL_Token t, int canceled);
int ril_request_get_canceled(RIL_Token t);
RIL_Token ril_request_get_token(int id);
int ril_request_get_id(RIL_Token t);

void ril_request_complete(RIL_Token t, RIL_Errno e, void *data, size_t length);
void ril_request_unsolicited(int request, void *data, size_t length);
void ril_request_timed_callback(RIL_TimedCallback callback, void *data, const struct timeval *time);

/**
 * RIL tokens
 */

struct ril_tokens {
	RIL_Token radio_power;
	RIL_Token pin_status;
	RIL_Token get_imei;
	RIL_Token get_imeisv;
	RIL_Token baseband_version;
	RIL_Token operator;
	RIL_Token outgoing_sms;
	RIL_Token dtmf_start;
	RIL_Token dtmf_stop;
};

void ril_tokens_check(void);

/**
 * RIL state
 */
 
 typedef enum {
	POWER_STATE_OFF                  = 0,
	POWER_STATE_LPM                  = 1,
	POWER_STATE_NORMAL               = 2,
	POWER_STATE_SIM_INIT_COMPLETE    = 4,
} modem_power_state;

typedef enum {
	SIM_STATE_ABSENT			= 0,
	SIM_STATE_NOT_READY			= 1,
	SIM_STATE_READY				= 2,
	SIM_STATE_PIN				= 3,
	SIM_STATE_PUK				= 4,
	SIM_STATE_BLOCKED			= 5,
	SIM_STATE_NETWORK_PERSO 		= 6,
	SIM_STATE_NETWORK_SUBSET_PERSO		= 7,
	SIM_STATE_CORPORATE_PERSO		= 8,
	SIM_STATE_SERVICE_PROVIDER_PERSO	= 9,
} ril_sim_state;

typedef enum {
	USSD_NO_ACTION_REQUIRE 			= 0,
	USSD_ACTION_REQUIRE			= 1,
	USSD_TERMINATED_BY_NET			= 2,
	USSD_OTHER_CLIENT			= 3,
	USSD_NOT_SUPPORT 			= 4,
	USSD_TIME_OUT				= 5,
} ril_ussd_state;

struct ril_state {
	RIL_RadioState radio_state;
	ril_sim_state sim_state;
	int power_state;
	int reg_state;
	int ussd_state;
	uint32_t net_mode;
	uint32_t cell_id;
	uint8_t rac_id;
	uint16_t lac_id;
	char proper_plmn[9];
	char SPN[NET_MAX_SPN_LEN];
	unsigned char dtmf_tone;
#if 0
	struct ipc_sec_sim_status_response sim_pin_status;
	struct ipc_sec_sim_icc_type sim_type;

	struct ipc_net_regist_response netinfo;
	struct ipc_net_regist_response gprs_netinfo;
	struct ipc_net_current_plmn_response plmndata;

	struct ipc_call_status call_status;

	int gprs_last_failed_cid;

	unsigned char ussd_state;

	unsigned char sms_incoming_msg_tpid;
#endif
};

void ril_state_lpm(void);

/**
 * RIL data
 */

typedef struct {
	uint32_t callId, callType;
	uint8_t bMT;
	char number[64];
	uint32_t call_state;
	RIL_Token token;
} callContext;

struct ril_data {
	struct RIL_Env *env;

	struct ril_state state;
	struct ril_tokens tokens;
	struct list_head *gprs_connections;
	struct list_head *incoming_sms;
	struct list_head *outgoing_sms;
	struct list_head *generic_responses;
	struct list_head *requests;
	int request_id;
	char smsc_number[30];
	callContext *calls[MAX_CALLS];
	struct ril_client *ipc_packet_client;
	struct ril_client *srs_client;

	pthread_mutex_t mutex;
};

extern struct ril_data ril_data;

int ril_modem_check(void);

/**
 * Clients dispatch functions
 */

void srs_dispatch(struct srs_message *message);

/* PWR */
void ipc_cp_system_start(void* data);
void ril_request_radio_power(RIL_Token t, void *data, size_t datalen);

/* MISC */
void ril_request_get_imei(RIL_Token t);
void ril_request_get_imeisv(RIL_Token t);
void ril_request_baseband_version(RIL_Token t);
void ril_request_get_imsi(RIL_Token t);
void ril_request_screen_state(RIL_Token t, void *data, size_t datalen);

/* CALL */
void ipc_call_incoming(void* data);
void ipc_call_end(void* data);
void ipc_call_setup_ind(void* data);
void ipc_call_alert(void* data);
void ipc_call_connected(void* data);
void ipc_call_connected_number_ind(void* data);
void ipc_call_dtmf_start(void* data);
void ipc_call_dtmf_stop(void* data);
void ipc_call_hold(void* data);
void ipc_call_swap(void* data);
void ipc_call_activate(void* data);
void ipc_call_error(void* data);
void ril_request_dial(RIL_Token t, void *data, size_t datalen);
void ril_request_get_current_calls(RIL_Token t);
void ril_request_hangup(RIL_Token t, void *data, size_t datalen);
void ril_request_hangup_waiting_or_background(RIL_Token t);
void ril_request_hangup_foreground_resume_background(RIL_Token t);
void ril_request_answer(RIL_Token t);
void ril_request_last_call_fail_cause(RIL_Token t);
void ril_request_dtmf(RIL_Token t, void *data, int length);
void ril_request_dtmf_start(RIL_Token t, void *data, int length);
void ril_request_dtmf_stop(RIL_Token t);
void ril_request_switch_waiting_or_holding_and_active(RIL_Token t);

/* NETWORK */
int ipc2ril_net_mode(uint32_t mode);
uint32_t ril2ipc_net_mode(int mode);
void ipc_network_radio_info(void* data);
void ipc_network_select(void* data);
void ipc_cell_info(void* data);
void ipc_network_nitz_info(void* data);
void network_start(void);
void ril_request_operator(RIL_Token t);
void ril_request_voice_registration_state(RIL_Token t);
void ril_request_get_preferred_network_type(RIL_Token t);
void ril_request_set_preferred_network_type(RIL_Token t, void *data, size_t datalen);

/* SIM */
void ril_sim_init(void);
void ipc_sim_status(void *data);
void ipc_pin_status(void* data);
void ipc_sim_io_response(void* data);
void ril_request_get_sim_status(RIL_Token t);
void ril_state_update(ril_sim_state sim_state);
void ril_request_enter_sim_pin(RIL_Token t, void *data, size_t datalen);

/* SMS */
void ipc_sms_send_status(void* data);
void ipc_incoming_sms(void* data);
void ril_request_send_sms(RIL_Token t, void *data, size_t length);
void nettext_cb_setup(void);

/* SS */
void ril_request_send_ussd(RIL_Token t, void *data, size_t datalen);
void ril_request_cancel_ussd(RIL_Token t, void *data, size_t datalen);
void ipc_ss_ussd_response(void* data);
void ipc_ss_error_response(void* data);

/* SND */
void ril_request_set_mute(RIL_Token t, void *data, size_t datalen);
void srs_control_ping(struct srs_message *message);
void srs_snd_set_volume(struct srs_message *message);
void srs_snd_set_audio_path(struct srs_message *message);
void srs_snd_1mic_ns_ctrl(struct srs_message *message);
void srs_snd_pcm_if_ctrl(struct srs_message *message);

#endif

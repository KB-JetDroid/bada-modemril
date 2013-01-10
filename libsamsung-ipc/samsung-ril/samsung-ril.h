/**
 * This file is part of samsung-ril.
 *
 * Copyright (C) 2010-2011 Joerie de Gram <j.de.gram@gmail.com>
 * Copyright (C) 2011-2012 Paul Kocialkowski <contact@paulk.fr>
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

#ifndef _SAMSUNG_RIL_H_
#define _SAMSUNG_RIL_H_

#include <pthread.h>

#include <utils/Log.h>
#include <telephony/ril.h>

#include <radio.h>

#include "ipc.h"
#include "srs.h"

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

	RIL_Token registration_state;
	RIL_Token gprs_registration_state;
	RIL_Token operator;
	
	RIL_Token outgoing_sms;
};

void ril_tokens_check(void);

/**
 * RIL state
 */
 
 typedef enum {
	POWER_STATE_LPM                  = 0,
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

struct ril_state {
	RIL_RadioState radio_state;
	ril_sim_state sim_state;
	int power_state;
#if 0
	struct ipc_sec_sim_status_response sim_pin_status;
	struct ipc_sec_sim_icc_type sim_type;

	struct ipc_net_regist_response netinfo;
	struct ipc_net_regist_response gprs_netinfo;
	struct ipc_net_current_plmn_response plmndata;

	struct ipc_call_status call_status;

	int gprs_last_failed_cid;

	unsigned char dtmf_tone;
	unsigned char ussd_state;

	unsigned char sms_incoming_msg_tpid;
#endif
};

void ril_state_lpm(void);

/**
 * RIL data
 */

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

/* Call */
void ipc_call_incoming(void* data);
void ipc_call_status(void* data);
void ril_request_dial(RIL_Token t, void *data, size_t datalen);
void ril_request_get_current_calls(RIL_Token t);
void ril_request_hangup(RIL_Token t);
void ril_request_answer(RIL_Token t);
void ril_request_last_call_fail_cause(RIL_Token t);
void ril_request_dtmf(RIL_Token t, void *data, int length);
void ril_request_dtmf_start(RIL_Token t, void *data, int length);
void ril_request_dtmf_stop(RIL_Token t);


#endif

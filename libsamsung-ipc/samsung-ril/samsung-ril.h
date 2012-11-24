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

#ifndef _SAMSUNG_RIL_H_
#define _SAMSUNG_RIL_H_

#include <pthread.h>

#include <telephony/ril.h>
#include <radio.h>

#include "ipc.h"
#include "srs.h"

/**
 * Defines
 */

#define RIL_CLIENT_LOCK(client) pthread_mutex_lock(&(client->mutex));
#define RIL_CLIENT_UNLOCK(client) pthread_mutex_unlock(&(client->mutex));

#define RIL_onRequestCompleteReal(t, e, response, responselen) ril_env->OnRequestComplete(t,e, response, responselen)
#define RIL_onUnsolicitedResponse(a,b,c) ril_env->OnUnsolicitedResponse(a,b,c)
#define RIL_requestTimedCallback(a,b,c) ril_env->RequestTimedCallback(a,b,c)

#define reqIdNew() ril_request_id_new()
#define reqGetId(t) ril_request_get_id(t)
#define reqGetToken(i) ril_request_get_token(i)

/**
 * RIL structures
 */

struct ril_client;
struct ril_token;
struct ril_tokens;
struct ril_state;

/**
 * RIL globals
 */

extern struct ril_client *ipc_packet_client;
extern struct ril_client *srs_client;

extern const struct RIL_Env *ril_env;
extern struct ril_state ril_state;

/**
 * RIL client
 */

typedef int (*ril_client_func)(struct ril_client *client);

typedef enum {
	RIL_CLIENT_NULL		= 0,
	RIL_CLIENT_CREATED	= 1,
	RIL_CLIENT_READY	= 2,
	RIL_CLIENT_DESTROYED	= 3,
	RIL_CLIENT_ERROR	= 4,

} ril_client_state;

struct ril_client {
	ril_client_func create;
	ril_client_func destroy;
	ril_client_func read_loop;

	void *object;

	pthread_t thread;
	pthread_mutex_t mutex;

	ril_client_state state;
};

struct ril_client_funcs {
	ril_client_func create;
	ril_client_func destroy;
	ril_client_func read_loop;
};

struct ril_client *ril_client_new(struct ril_client_funcs *client_funcs);
int ril_client_free(struct ril_client *client);
int ril_client_create(struct ril_client *client);
int ril_client_destroy(struct ril_client *client);
int ril_client_thread_start(struct ril_client *client);

/**
 * RIL request token
 */

struct ril_request_token {
	RIL_Token token;
	int canceled;
};

void ril_requests_tokens_init(void);
int ril_request_reg_id(RIL_Token token);
int ril_request_get_id(RIL_Token token);
RIL_Token ril_request_get_token(int id);
int ril_request_get_canceled(RIL_Token token);
void ril_request_set_canceled(RIL_Token token, int canceled);

void RIL_onRequestComplete(RIL_Token t, RIL_Errno e, void *response, size_t responselen);

/**
 * RIL tokens
 */

// FIXME: Move RIL_Token token_ps, token_cs; here
struct ril_tokens {
	RIL_Token radio_power;
	RIL_Token get_imei;
	RIL_Token get_imeisv;
	RIL_Token baseband_version;

	RIL_Token registration_state;
	RIL_Token gprs_registration_state;
	RIL_Token operator;
};

void ril_tokens_check(void);

/**
 * RIL state
 */
 
typedef enum {
	SIM_ABSENT			= 0,
	SIM_NOT_READY			= 1,
	SIM_READY			= 2,
	SIM_PIN				= 3,
	SIM_PUK				= 4,
	SIM_BLOCKED			= 5,
	SIM_NETWORK_PERSO 		= 6,
	SIM_NETWORK_SUBSET_PERSO	= 7,
	SIM_CORPORATE_PERSO		= 8,
	SIM_SERVICE_PROVIDER_PERSO	= 9,
} SIM_Status;

typedef enum {
	POWER_MODE_LPM			= 0,
	POWER_MODE_NORMAL		= 2,
	POWER_MODE_SIM_INIT_COMPLETE	= 4,
} Modem_PowerMode;

struct ril_state {
	RIL_RadioState radio_state;
	RIL_CardState card_state;
	SIM_Status sim_status;
	Modem_PowerMode power_mode;

	struct ril_tokens tokens;
#if 0
	struct ipc_net_regist netinfo;
	struct ipc_net_regist gprs_netinfo;
	struct ipc_net_current_plmn plmndata;

	struct ipc_gprs_pdp_context gprs_context;
#endif

};

void ril_globals_init(void);
void ril_state_lpm(void);
int ril_modem_check(void);

/**
 * Clients dispatch functions
 */

void srs_dispatch(struct srs_message *message);


/* MISC */
void ril_request_get_imei(RIL_Token t);
void ril_request_get_imeisv(RIL_Token t);
void ril_request_baseband_version(RIL_Token t);
void ril_request_get_imsi(RIL_Token t);



#endif

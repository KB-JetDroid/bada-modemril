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

void ipc_dispatch(struct modem_io *resp);
void srs_dispatch(struct srs_message *message);

/* GEN */

struct ipc_gen_phone_res_expect {
	unsigned char aseq;
	unsigned short command;
	void (*func)(struct ipc_message_info *info);
	int to_complete;
	int to_abort;
};

void ipc_gen_phone_res_expects_init(void);
void ipc_gen_phone_res_expect_to_func(unsigned char aseq, unsigned short command, 
		void (*func)(struct ipc_message_info *info));
void ipc_gen_phone_res_expect_to_complete(unsigned char aseq, unsigned short command);
void ipc_gen_phone_res_expect_to_abort(unsigned char aseq, unsigned short command);
void ipc_gen_phone_res(struct ipc_message_info *info);

/* PWR */
void ipc_pwr_phone_pwr_up(void);
void ipc_pwr_phone_state(struct ipc_message_info *info);
void ril_request_radio_power(RIL_Token t, void *data, size_t datalen);

/* DISP */
void ipc_disp_icon_info(struct ipc_message_info *info);
void ipc_disp_rssi_info(struct ipc_message_info *info);

/* MISC */
void ril_request_get_imei(RIL_Token t);
void ril_request_get_imeisv(RIL_Token t);
void ipc_misc_me_sn(struct ipc_message_info *info);
void ril_request_baseband_version(RIL_Token t);
void ipc_misc_me_version(struct ipc_message_info *info);
void ril_request_get_imsi(RIL_Token t);
void ipc_misc_me_imsi(struct ipc_message_info *info);
void ipc_misc_time_info(struct ipc_message_info *info);

/* SAT */
void respondSatProactiveCmd(struct ipc_message_info *request);
void requestSatSendTerminalResponse(RIL_Token t, void *data, size_t datalen);
void requestSatSendEnvelopeCommand(RIL_Token t, void *data, size_t datalen);
void respondSatEnvelopeCmd(struct ipc_message_info *request);

/* SIM */
void ipc_sec_pin_status(struct ipc_message_info *info);
void ril_request_sim_status(RIL_Token t);
void ril_request_sim_io(RIL_Token t, void *data, size_t datalen);
void ipc_sec_rsim_access(struct ipc_message_info *info);
void ril_request_enter_sim_pin(RIL_Token t, void *data, size_t datalen);
void ipc_sec_pin_status_complete(struct ipc_message_info *info);
void ipc_sec_lock_info(struct ipc_message_info *info);
void ril_request_query_facility_lock(RIL_Token t, void *data, size_t datalen);
void ipc_sec_phone_lock(struct ipc_message_info *info);
void ril_request_set_facility_lock(RIL_Token t, void *data, size_t datalen);

/* NET */
void ril_request_operator(RIL_Token t);
void ipc_net_current_plmn(struct ipc_message_info *message);
void ril_request_registration_state(RIL_Token t);
void ril_request_gprs_registration_state(RIL_Token t);
void ipc_net_regist(struct ipc_message_info *message);
void ril_request_query_available_networks(RIL_Token t);
void ipc_net_plmn_list(struct ipc_message_info *info);
void ril_request_query_network_selection_mode(RIL_Token t);
void ril_request_get_preferred_network_type(RIL_Token t);
void ril_request_set_preffered_network_type(RIL_Token t, void *data, size_t datalen);
void ipc_net_mode_sel(struct ipc_message_info *info);

/* SMS */
struct ril_request_sms {
	char *pdu;
	int pdu_len;
	char *smsc;
	int smsc_len;

	unsigned char aseq;
};

void ril_request_sms_init(void);
void ril_request_sms_del(int id);
void ril_request_sms_clear(int id);
int ril_request_sms_add(unsigned char aseq,
			char *pdu, int pdu_len, 
			char *smsc, int smsc_len);
int ril_request_sms_get_id(unsigned char aseq);
int ril_request_sms_get_next(void);
int ril_request_sms_lock_acquire(void);
void ril_request_sms_lock_release(void);

void ril_request_send_sms(RIL_Token t, void *data, size_t datalen);
void ril_request_send_sms_expect_more(RIL_Token t, void *data, size_t datalen);
int ril_request_send_sms_next(void);
void ril_request_send_sms_complete(RIL_Token t, char *pdu, char *smsc);
void ipc_sms_send_msg_complete(struct ipc_message_info *info);
void ipc_sms_svc_center_addr(struct ipc_message_info *info);
void ipc_sms_send_msg(struct ipc_message_info *info);

void ipc_sms_tpid_queue_init(void);
void ipc_sms_tpid_queue_del(int id);
int ipc_sms_tpid_queue_add(unsigned char sms_tpid);
int ipc_sms_tpid_queue_get_next(void);

void ipc_sms_incoming_msg(struct ipc_message_info *info);
void ril_request_sms_acknowledge(RIL_Token t, void *data, size_t datalen);
void ipc_sms_deliver_report(struct ipc_message_info *info);

void ipc_sms_device_ready(struct ipc_message_info *info);

/* Call */
void ipc_call_incoming(struct ipc_message_info *info);
void ipc_call_status(struct ipc_message_info *info);
void ril_request_dial(RIL_Token t, void *data, size_t datalen);
void ril_request_get_current_calls(RIL_Token t);
void ipc_call_list(struct ipc_message_info *info);
void ril_request_hangup(RIL_Token t);
void ril_request_answer(RIL_Token t);
void ril_request_dtmf(RIL_Token t, void *data, int length);
void ipc_call_burst_dtmf(struct ipc_message_info *info);
void ril_request_dtmf_start(RIL_Token t, void *data, int length);
void ril_request_dtmf_stop(RIL_Token t);

/* SND */

void ril_request_set_mute(RIL_Token t, void *data, size_t datalen);
void srs_snd_set_call_clock_sync(struct srs_message *message);
void srs_snd_set_call_volume(struct srs_message *message);
void srs_snd_set_call_audio_path(struct srs_message *message);

/* GPRS */

void ril_request_setup_data_call(RIL_Token t, void *data, int length);
void ril_request_deactivate_data_call(RIL_Token t, void *data, int length);
void ipc_gprs_ip_configuration(struct ipc_message_info *info);


#endif

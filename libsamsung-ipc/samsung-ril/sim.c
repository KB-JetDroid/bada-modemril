/**
 * This file is part of samsung-ril.
 *
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

#define LOG_TAG "Mocha-RIL-SIM"
#include <utils/Log.h>

#include "samsung-ril.h"
#include "util.h"
#include <sim.h>

RIL_Token token;

void ril_sim_init(void)
{
	sim_atk_open();	
	sim_open_to_modem(4);
	
}

void ipc_sim_status(void *data)
{
	ALOGE("%s: test me!", __func__);

	ril_sim_state sim_state;
	sim_state =(int) data;

	/* Update radio state based on SIM state */
	ril_state_update(sim_state);

	ril_request_unsolicited(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0);	

}

void ipc_pin_status(void* data)
{
	ALOGE("%s: test me!", __func__);
	pinStatus* pinSt = (pinStatus*)(data);
	int attempts = -1;
	switch(pinSt->status){
		case 0:
			DEBUG_I("%s : Correct password ", __func__);
			ril_request_complete(token, RIL_E_SUCCESS, &attempts, sizeof(attempts));
			return;
		case 1:
			if (pinSt->attempts == 0) {
				DEBUG_I("%s : Wrong password and no attempts left!", __func__);
				attempts = 0;
				ril_request_complete(token, RIL_E_PASSWORD_INCORRECT, &attempts, sizeof(attempts));
				ril_request_unsolicited(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, NULL, 0);
				return;	
			} else {
				DEBUG_I("%s : Wrong password ", __func__);
				ril_request_complete(token, RIL_E_PASSWORD_INCORRECT, &attempts, sizeof(attempts));}
			return;
	}
}

void ril_request_get_sim_status(RIL_Token t)
{
	ALOGE("%s: test me!", __func__);

	RIL_CardStatus_v6 card_status;
	ril_sim_state sim_state;
	int app_status_array_length;
	int app_index;
	int i;

	static RIL_AppStatus app_status_array[] = {
		/* SIM_ABSENT = 0 */
		{ RIL_APPTYPE_UNKNOWN, RIL_APPSTATE_UNKNOWN, RIL_PERSOSUBSTATE_UNKNOWN,
		NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
		/* SIM_NOT_READY = 1 */
		{ RIL_APPTYPE_SIM, RIL_APPSTATE_DETECTED, RIL_PERSOSUBSTATE_UNKNOWN,
		NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
		/* SIM_READY = 2 */
		{ RIL_APPTYPE_SIM, RIL_APPSTATE_READY, RIL_PERSOSUBSTATE_READY,
		NULL, NULL, 0, RIL_PINSTATE_UNKNOWN, RIL_PINSTATE_UNKNOWN },
		/* SIM_PIN = 3 */
		{ RIL_APPTYPE_SIM, RIL_APPSTATE_PIN, RIL_PERSOSUBSTATE_UNKNOWN,
		NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
		/* SIM_PUK = 4 */
		{ RIL_APPTYPE_SIM, RIL_APPSTATE_PUK, RIL_PERSOSUBSTATE_UNKNOWN,
		NULL, NULL, 0, RIL_PINSTATE_ENABLED_BLOCKED, RIL_PINSTATE_UNKNOWN },
		/* SIM_BLOCKED = 4 */
		{ RIL_APPTYPE_SIM, RIL_APPSTATE_PUK, RIL_PERSOSUBSTATE_UNKNOWN,
		NULL, NULL, 0, RIL_PINSTATE_ENABLED_PERM_BLOCKED, RIL_PINSTATE_UNKNOWN },
		/* SIM_NETWORK_PERSO = 6 */
		{ RIL_APPTYPE_SIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_NETWORK,
		NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
		/* SIM_NETWORK_SUBSET_PERSO = 7 */
		{ RIL_APPTYPE_SIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_NETWORK_SUBSET,
		NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
		/* SIM_CORPORATE_PERSO = 8 */
		{ RIL_APPTYPE_SIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_CORPORATE,
		NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
		/* SIM_SERVICE_PROVIDER_PERSO = 9 */
		{ RIL_APPTYPE_SIM, RIL_APPSTATE_SUBSCRIPTION_PERSO, RIL_PERSOSUBSTATE_SIM_SERVICE_PROVIDER,
		NULL, NULL, 0, RIL_PINSTATE_ENABLED_NOT_VERIFIED, RIL_PINSTATE_UNKNOWN },
		};

	sim_state = ril_data.state.sim_state;

	/* Card is assumed to be present if not explicitly absent */
	if(ril_data.state.sim_state == SIM_STATE_ABSENT) {
		card_status.card_state = RIL_CARDSTATE_ABSENT;
	} else {
		card_status.card_state = RIL_CARDSTATE_PRESENT;
	}
		
	// Initialize the apps
	for (i = 0 ; i < RIL_CARD_MAX_APPS ; i++) {
		card_status.applications[i] = app_status_array[0];
	}

	// FIXME: How do we know that?
	card_status.universal_pin_state = RIL_PINSTATE_UNKNOWN;


	/* Initialize apps */
	for (i = 0; i < RIL_CARD_MAX_APPS; i++) {
	card_status.applications[i] = app_status_array[i];
	}

	// sim_state corresponds to the app index on the table
	card_status.gsm_umts_subscription_app_index = (int) sim_state;
	card_status.cdma_subscription_app_index = (int) sim_state;
	card_status.num_applications = RIL_CARD_MAX_APPS;

	ril_request_complete(t, RIL_E_SUCCESS, &card_status, sizeof(card_status));
	
}

void ril_state_update(ril_sim_state sim_state)
{
	RIL_RadioState radio_state;

	/* If power mode isn't at least normal, don't update RIL state */
	if (ril_data.state.power_state != POWER_STATE_NORMAL)
		return;

	ril_data.state.sim_state = sim_state;

	switch(sim_state) {
		case SIM_STATE_READY:
			radio_state = RADIO_STATE_SIM_READY;
			break;
		case SIM_STATE_NOT_READY:
			radio_state = RADIO_STATE_SIM_NOT_READY;
			break;
		case SIM_STATE_ABSENT:
		case SIM_STATE_PIN:
		case SIM_STATE_PUK:
		case SIM_STATE_BLOCKED:
		case SIM_STATE_NETWORK_PERSO:
		case SIM_STATE_NETWORK_SUBSET_PERSO:
		case SIM_STATE_CORPORATE_PERSO:
		case SIM_STATE_SERVICE_PROVIDER_PERSO:
			radio_state = RADIO_STATE_SIM_LOCKED_OR_ABSENT;
			break;
		default:
			radio_state = RADIO_STATE_SIM_NOT_READY;
			break;
	}
	ril_data.state.radio_state = radio_state;
	ril_tokens_check();
	ril_request_unsolicited(RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED, NULL, 0);
}

void ril_request_enter_sim_pin(RIL_Token t, void *data, size_t datalen)
{
	char *pin = ((char **) data)[0];
	/* 1. Send PIN */
	if (strlen(data) > 16) {
		ALOGE("%s: pin exceeds maximum length", __FUNCTION__);
		ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
	}
	ALOGE("%s: pin = %s", __FUNCTION__, pin);
	sim_verify_chv(0x4, 0x0, pin);
	token = t;

}

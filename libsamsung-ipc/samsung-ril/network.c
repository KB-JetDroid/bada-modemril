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
 
#define LOG_TAG "Mocha-RIL-NETWORK"
#include <utils/Log.h>

#include "samsung-ril.h"
#include "util.h"
#include <tapi_network.h>
#include <tapi_nettext.h>
#include <plmn_list.h>

int reg_state = 0;
char proper_plmn[10];
 
void ipc_network_radio_info(void* data)
{
	tapiRadioInfo* radioInfo = (tapiRadioInfo*)(data);
	RIL_SignalStrength_v6 ss;
	int rssi;

	/* Don't consider this if modem isn't in normal power mode. */
	if(ril_data.state.power_state < POWER_STATE_NORMAL)
		return;

	memset(&ss, 0, sizeof(ss));
	memset(&ss.LTE_SignalStrength, -1, sizeof(ss.LTE_SignalStrength));	
	
	/* rxLevel appears to be value between 0 and 100, rescale it to 0-31 */
	rssi = (((uint32_t)radioInfo->rxLevel) * 31) / 100;

	ss.GW_SignalStrength.signalStrength = rssi;
	ss.GW_SignalStrength.bitErrorRate = 99;

	ALOGD("Signal Strength is %d\n", rssi);

	ril_request_unsolicited(RIL_UNSOL_SIGNAL_STRENGTH, &ss, sizeof(ss));
}

void ipc_network_select(void* data)
{	
	tapiNetworkInfo* netInfo = (tapiNetworkInfo*)(data);

        /* Converts IPC network registration status to Android RIL format */
	switch(netInfo->serviceType) {
		case 0x1:
			reg_state = 0;//Not registered, MT is not currently searching a new operator to register
			break;
		case 0x2:
			reg_state = 2;//Not registered, but MT is currently searching a new operator to register
			break;
		case 0x4:
			reg_state = 1; //Registered, home network
			break;
		default:
			reg_state = 0;
			break;
	}

	ril_request_unsolicited(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED, NULL, 0);
	
}

void ipc_cell_info(void* data)
{
	tapiCellInfo* cellInfo = (tapiCellInfo*)(data);

	uint16_t mcc = ((cellInfo->plmnId[0] & 0xF) * 100) + (((cellInfo->plmnId[0] >> 4) & 0xF) * 10) + (((cellInfo->plmnId[1]) & 0xF) * 1);
	uint16_t mnc = ((cellInfo->plmnId[2] & 0xF) * 10) + (((cellInfo->plmnId[2] >> 4) & 0xF) * 1);
	sprintf(proper_plmn, "%3u%2u", mcc, mnc);
	//TODO: implement cell id and LAC convertion to android RIL format 
}

void network_start(void)
{

	int i;
	tapiStartupNetworkInfo start_info;
	tapi_nettext_cb_settings* cb_sett_buf;
	start_info.bAutoSelection = 1;
	start_info.bPoweronGprsAttach = 1;
	start_info.networkOrder = 1;
	start_info.serviceDomain = 0;
	start_info.unknown1[0] = 0xE5;
	start_info.unknown1[1] = 0x69;
	start_info.networkMode = 0x00007FF8;
	start_info.subscriptionMode = 0;
	start_info.bFlightMode = 0;	
	start_info.unknown2[0] = 0x02;
	start_info.unknown2[1] = 0x41;
	/* TODO: Check if it can be executed from tapi_init, or do we need to wait for network select or some other packet. */
	tapi_network_startup(&start_info);
	
	tapi_nettext_set_preferred_memory(1); /* let's hope it means phone, not sim */
	tapi_nettext_set_net_burst(0); /* disable */
	
	ril_sim_init();

	tapi_set_subscription_mode(0x1);

	cb_sett_buf = (tapi_nettext_cb_settings *)malloc(sizeof(tapi_nettext_cb_settings));
	memset(cb_sett_buf, 0, sizeof(tapi_nettext_cb_settings));		
	cb_sett_buf->ext_cb = 0x0;
	cb_sett_buf->ext_cb_enable = 0x0;
	cb_sett_buf->enable_all_combined_cb_channels = 0x1;
	cb_sett_buf->combined_language_type = 0x0;
	cb_sett_buf->number_of_combined_cbmi = 0x367FFF;
	for (i = 0; i < 40; i++) {
	cb_sett_buf->cb_info[i] = 0x0;}
	tapi_nettext_set_cb_settings((uint8_t *)cb_sett_buf);
	free(cb_sett_buf);
}

void ril_plmn_split(char *plmn_data, char **plmn, unsigned int *mcc, unsigned int *mnc)
{
	char plmn_t[7];

	memset(plmn_t, 0, sizeof(plmn_t));
	memcpy(plmn_t, plmn_data, 6);
//	if (plmn_t[5] == 'f')
//		plmn_t[5] = '\0';
//	DEBUG_I("TAPI: plmn_t = %s",plmn_t);
	if (plmn != NULL) {
		*plmn = malloc(sizeof(plmn_t));
		memcpy(*plmn, plmn_t, sizeof(plmn_t));
	}
	if (mcc == NULL || mnc == NULL)
		return;
	sscanf(plmn_t, "%3u%2u", mcc, mnc);
}

void ril_request_operator(RIL_Token t)
{
	char *response[3];
	char *plmn;
	unsigned int mcc, mnc;
	int plmn_entries;
	int i;



	if (reg_state == 1) {
		
		ril_plmn_split(proper_plmn, &plmn, &mcc, &mnc);
		plmn_entries = sizeof(plmn_list) / sizeof(struct plmn_list_entry);
		
		memset(response, 0, sizeof(response));

		asprintf(&response[2], "%s", plmn);
		free(plmn);

		for (i=0 ; i < plmn_entries ; i++) {
			if (plmn_list[i].mcc == mcc && plmn_list[i].mnc == mnc) {
				asprintf(&response[0], "%s", plmn_list[i].operator_short);
				asprintf(&response[1], "%s", plmn_list[i].operator_long);
			}
		}
		ril_request_complete(t, RIL_E_SUCCESS, response, sizeof(response));
	}
	else
	{
		ril_request_complete(t, RIL_E_OP_NOT_ALLOWED_BEFORE_REG_TO_NW, NULL, 0);
		
	}


}

void ril_request_registration_state(RIL_Token t)
{
	char *response[15];

	memset(response, 0, sizeof(response));

	asprintf(&response[0], "%d", reg_state);
	asprintf(&response[1], "%x", 0x0f69);
	asprintf(&response[2], "%x", 0x00000637);
	asprintf(&response[3], "%d", 0);

	ril_request_complete(t, RIL_E_SUCCESS, response, sizeof(response));
}

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

}


void network_start(void)
{
	tapiStartupNetworkInfo start_info;
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
}

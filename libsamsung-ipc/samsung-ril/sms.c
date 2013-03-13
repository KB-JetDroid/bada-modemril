/**
 * This file is part of samsung-ril.
 *
 * Copyright (C) 2010-2011 Joerie de Gram <j.de.gram@gmail.com>
 * Copyright (C) 2011 Paul Kocialkowski <contact@oaulk.fr>
 * Copyright (C) 2013 Nikolay Volkov <volk204@mail.ru>
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

#define LOG_TAG "Mocha-RIL-SMS"
#include <utils/Log.h>

#include "samsung-ril.h"
#include "util.h"
#include <tapi_nettext.h>


/**
 * SMS global vars
 */

RIL_Token token;

/**
 * Format conversion utils
 */

void ipc_sms_send_status(void* data)
{
	ALOGE("%s: test me!", __func__);
	tapiNettextCallBack* callBack = (tapiNettextCallBack*)(data);

	RIL_SMS_Response response;

	memset(&response, 0, sizeof(response));
	response.messageRef = 0;
	response.ackPDU = NULL;

	switch(callBack->status){
		case 0:
			DEBUG_I("%s : Message sent  ", __func__);
			response.errorCode = -1;
			ril_request_complete(token, RIL_E_SUCCESS, &response, sizeof(response));
			return;

		default:
			DEBUG_I("%s : Message sending error  ", __func__);
			response.errorCode = 500;
			ril_request_complete(token, RIL_E_GENERIC_FAILURE, &response, sizeof(response));
			return;
	}

}
#if 0
/**
 * RIL request SMS (queue) functions
 */

void ril_request_sms_init(void)
{
	memset(ril_request_sms, 0, sizeof(struct ril_request_sms) * 10);
	ril_request_sms_lock = 0;
}

void ril_request_sms_del(int id)
{
	if(id < 0 || id > 9) {
		ALOGD("Invalid id (%d) for the SMS queue", id);
		return;
	}

	ril_request_sms[id].aseq = 0;
	ril_request_sms[id].pdu_len = 0;
	ril_request_sms[id].smsc_len = 0;

	if(ril_request_sms[id].pdu != NULL)
		free(ril_request_sms[id].pdu);
	if(ril_request_sms[id].smsc != NULL)
		free(ril_request_sms[id].smsc);
}

void ril_request_sms_clear(int id)
{
	if(id < 0 || id > 9) {
		ALOGD("Invalid id (%d) for the SMS queue", id);
		return;
	}

	ril_request_sms[id].aseq = 0;
	ril_request_sms[id].pdu = NULL;
	ril_request_sms[id].pdu_len = 0;
	ril_request_sms[id].smsc = NULL;
	ril_request_sms[id].smsc_len = 0;
}

int ril_request_sms_new(void)
{
	int id = -1;
	int i;

	/* Find the highest place in the queue */
	for(i=10 ; i > 0 ; i--) {
		if(ril_request_sms[i-1].aseq && ril_request_sms[i-1].pdu) {
			break;
		}

		id = i-1;
	}

	if(id < 0) {
		ALOGE("The SMS queue is full, removing the oldest req");
	
		/* Free the request at index 0 (oldest) */
		ril_request_sms_del(0);

		/* Increase all the requests id to have the last one free */
		for(i=1 ; i < 10 ; i++) {
			ALOGD("SMS queue: moving %d -> %d", i, i-1);
			memcpy(&ril_request_sms[i-1], &ril_request_sms[i], sizeof(struct ril_request_sms));
		}

		/* We must not free the pointers here as we copied these at index 8 */

		ril_request_sms_clear(9);

		return 9;
	}

	return id;
}

int ril_request_sms_add(unsigned char aseq,
			char *pdu, int pdu_len, 
			char *smsc, int smsc_len)
{
	int id = ril_request_sms_new();

	ALOGD("Storing new SMS request in the queue at index %d\n", id);

	ril_request_sms[id].aseq = aseq;
	ril_request_sms[id].smsc_len = smsc_len;
	ril_request_sms[id].pdu_len = pdu_len;

	if(pdu != NULL) {
		ril_request_sms[id].pdu = malloc(pdu_len);
		memcpy(ril_request_sms[id].pdu, pdu, pdu_len);
	}

	if(smsc != NULL) {
		ril_request_sms[id].smsc = malloc(smsc_len);
		memcpy(ril_request_sms[id].smsc, smsc, smsc_len);
	}

	return id;
}

int ril_request_sms_get_id(unsigned char aseq)
{
	int i;

	for(i=0 ; i < 10 ; i++) {
		if(ril_request_sms[i].aseq == aseq) {
			return i;
		}
	}

	return -1;
}

int ril_request_sms_get_next(void)
{
	int id = -1;
	int i;

	for(i=0 ; i < 10 ; i++) {
		if(ril_request_sms[i].aseq && ril_request_sms[i].pdu) {
			id = i;
		}
	}

	if(id < 0)
		ALOGD("Nothing left on the queue!");
	else
		ALOGD("Next queued request is at id #%d\n", id);

	return id;
}

int ril_request_sms_lock_acquire(void)
{
	if(ril_request_sms_lock > 0) {
		return 0;
	} else
	{
		ril_request_sms_lock = 1;
		return 1;
	}
}

void ril_request_sms_lock_release(void)
{
	ril_request_sms_lock = 0;
}

#endif

/**
 * Outgoing SMS functions
 */

/**
 * In: RIL_REQUEST_SEND_SMS
 *   Send an SMS message.
 *
 * Out: 
 */
void ril_request_send_sms(RIL_Token t, void *data, size_t length)
{
	char *pdu;
	int pdu_length;
	unsigned char *smsc;
	int smsc_length;
	int rc;

	if (data == NULL || length < 2 * sizeof(char *))
		return;

	pdu = ((char **) data)[1];
	smsc = ((unsigned char **) data)[0];
	pdu_length = 0;
	smsc_length = 0;

	if (pdu != NULL) {
		pdu_length = strlen(pdu) + 1;
		pdu = strdup(pdu);
	}

	if (smsc != NULL) {
		smsc_length = strlen((char *) smsc);
		smsc = (unsigned char *) strdup((char *) smsc);
	}


	/* We first need to get SMS SVC before sending the message */

	if(smsc == NULL) {
		ALOGD("We have no SMSC, let's ask one");
		smsc = (unsigned char *)"07919730071111F1"; //FIXME: Add smsc request
		smsc_length = strlen((char *) smsc);
		smsc = (unsigned char *) strdup((char *) smsc);
		ril_request_send_sms_complete(t, pdu, pdu_length, smsc, smsc_length);
		
	} else {
		ril_request_send_sms_complete(t, pdu, pdu_length, smsc, smsc_length);

		if (pdu != NULL)
			free(pdu);

		if (smsc != NULL)
			free(smsc);
	}
}

#if 0
/**
 * In: RIL_REQUEST_SEND_SMS_EXPECT_MORE
 *   Send an SMS message. Identical to RIL_REQUEST_SEND_SMS,
 *   except that more messages are expected to be sent soon. If possible,
 *   keep SMS relay protocol link open (eg TS 27.005 AT+CMMS command)
 *
 * Out: IPC_SMS_SEND_MSG
 */
void ril_request_send_sms_expect_more(RIL_Token t, void *data, size_t datalen)
{
	/* No particular treatment here, we already have a queue */
	ril_request_send_sms(t, data, datalen);
}

/**
 * Send the next SMS in the queue
 */
int ril_request_send_sms_next(void)
{
	int id = ril_request_sms_get_next();

	char *request[2] = { NULL };
	unsigned char aseq;
	char *pdu;
	char *smsc;

	/* When calling this function, you assume you're done with the previous sms req */
	ril_request_sms_lock_release();

	if(id < 0) 
		return -1;

	ALOGD("Sending queued SMS!");

	aseq = ril_request_sms[id].aseq;
	pdu = ril_request_sms[id].pdu;
	smsc = ril_request_sms[id].smsc;

	request[0] = smsc;
	request[1] = pdu;

	/* We need to clear here to prevent infinite loop, but we can't free mem yet */
	ril_request_sms_clear(id);

	ril_request_send_sms(reqGetToken(aseq), (void *) request, sizeof(request));

	if(pdu != NULL)
		free(pdu);

	if(smsc != NULL)
		free(smsc);

	return id;
}

#endif

/**
 * Complete (continue) the send_sms request (do the real sending)
 */
void ril_request_send_sms_complete(RIL_Token t, char *pdu, int pdu_length, unsigned char *smsc, int smsc_length)
{
	tapiNettextOutgoingMessage* mess;

	unsigned char *pdu_hex;
	int pdu_hex_length;
	void *data;
	int length, i, numberLen, send_msg_type;
	unsigned char *p;
	char *a, *message;
	message = NULL;


	if (pdu == NULL || pdu_length <= 0 || smsc == NULL || smsc_length <= 0) {

		ALOGE("Provided PDU or SMSC is invalid! Aborting");
		ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
		// Send the next SMS in the list
		//ril_request_send_sms_next();
		return;

	}

	if ((pdu_length / 2 + smsc_length) > 0xfe) {

		ALOGE("PDU or SMSC too large, aborting");
		ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);
		// Send the next SMS in the list
		//ril_request_send_sms_next();
		return;

	}

	pdu_hex_length = pdu_length % 2 == 0 ? pdu_length / 2 :

		(pdu_length ^ 1) / 2;

	// Length of the final message

	length = pdu_hex_length + smsc_length;

	ALOGD("Sending SMS message (length: 0x%x)!", length);

	pdu_hex = calloc(1, pdu_hex_length);

	hex2bin(pdu, pdu_length, pdu_hex);

	send_msg_type = 0;

	/* PDU operations */

	int pdu_tp_da_index = 2;

	unsigned char pdu_tp_da_len = pdu_hex[pdu_tp_da_index];

	if (pdu_tp_da_len > 0xff / 2) {

		ALOGE("PDU TP-DA Len failed (0x%x)\n", pdu_tp_da_len);

		goto pdu_end;

	}

	ALOGD("PDU TP-DA Len is 0x%x\n", pdu_tp_da_len);

	int pdu_tp_udh_index = pdu_tp_da_index + pdu_tp_da_len;

	unsigned char pdu_tp_udh_len = pdu_hex[pdu_tp_udh_index];

	if (pdu_tp_udh_len > 0xff / 2 || pdu_tp_udh_len < 5) {

		ALOGE("PDU TP-UDH Len failed (0x%x)\n", pdu_tp_udh_len);

		goto pdu_end;

	}

	ALOGD("PDU TP-UDH Len is 0x%x\n", pdu_tp_udh_len);

	int pdu_tp_udh_num_index = pdu_tp_udh_index + 4;

	unsigned char pdu_tp_udh_num = pdu_hex[pdu_tp_udh_num_index];

	if (pdu_tp_udh_num > 0xf) {

		ALOGE("PDU TP-UDH Num failed (0x%x)\n", pdu_tp_udh_num);

		goto pdu_end;

	}

	int pdu_tp_udh_seq_index = pdu_tp_udh_index + 5;

	unsigned char pdu_tp_udh_seq = pdu_hex[pdu_tp_udh_seq_index];

	if (pdu_tp_udh_seq > 0xf || pdu_tp_udh_seq > pdu_tp_udh_num) {

		ALOGE("PDU TP-UDH Seq failed (0x%x)\n", pdu_tp_udh_seq);

		goto pdu_end;

	}

	ALOGD("We are sending message %d on %d\n", pdu_tp_udh_seq, pdu_tp_udh_num);

	if (pdu_tp_udh_num > 1) {

		ALOGD("We are sending a multi-part message!");

		send_msg_type = 1; //multi-part

	}


	pdu_end:

	DEBUG_I("%s : pdu : %s", __func__, pdu);
	DEBUG_I("%s : pdu_hex : %s", __func__, pdu_hex);

	numberLen = (uint8_t)pdu_tp_da_len;	

	mess = (tapiNettextOutgoingMessage *)malloc(sizeof(tapiNettextOutgoingMessage));
	memset(mess, 0, sizeof(tapiNettextOutgoingMessage));		
	for (i = 0; i < 7; i++) {
	mess->Unknown1[i] = 0x0;}

	mess->Unknown2= 0x00; // 00
	mess->Unknown3= 0x01; // 01
	mess->numberType= 0x01; // 01 - international
	mess->numberLength = numberLen;
	if (numberLen % 2 > 0)
		numberLen = numberLen + 1;		
	i = 0;	
	while (i < numberLen)
	{
		mess->phoneNumber[i] = pdu[i + 9];
		if ( pdu[i + 8] != 'f')
		mess->phoneNumber[i+1] =pdu[i + 8]; 	
		i = i + 2;		
	}

	mess->unknown4[0] = 0xFF; //fake
	mess->unknown4[1] = 0x46; //fake
	mess->unknown4[2] = 0x3A; //fake
	mess->unknown4[3] = 0x51; //fake
	mess->unknown4[4] = 0x00; //fake
	mess->unknown4[5] = 0x01; //fake
	mess->unknown4[6] = 0x01; //fake


	//fake serviceCenter number  - 79037011111
	//FIXME: add convert SMSC to SMS packet format
	mess->serviceCenterLength = 0xB;
	DEBUG_I("%s : smsc = %s", __func__, smsc);
	mess->serviceCenter[0] = 0x37;
	mess->serviceCenter[1] = 0x39;
	mess->serviceCenter[2] = 0x30;
	mess->serviceCenter[3] = 0x33;
	mess->serviceCenter[4] = 0x37;
	mess->serviceCenter[5] = 0x30;
	mess->serviceCenter[6] = 0x31;
	mess->serviceCenter[7] = 0x31;
	mess->serviceCenter[8] = 0x31;
	mess->serviceCenter[9] = 0x31;
	mess->serviceCenter[10] = 0x31;

	mess->unknown5 = 0x01; //01
	mess->unknown6 = 0x03; //03
	for (i = 0; i < 6; i++) {
	mess->unknown7[i] = 0x00; }// all is NULL
	mess->unknown8 = 0xFF; //FF
	for (i = 0; i < 17; i++) {
	mess->unknown9[i] = 0x00;}// all is NULL 
	mess->unknown10 = 0x04; //04	
	mess->unknown11 = 0x04; //04
	mess->unknown12[0] = 0x00;
	mess->unknown12[1] = 0x00;

	if (send_msg_type == 0)
		mess->messageLength = pdu_hex[(numberLen / 2) + 6];
	else	
		mess->messageLength = pdu_hex[(numberLen / 2) + 6];

	if (pdu_hex[(numberLen / 2) + 5] == 8)
	{
		DEBUG_I("%s : DCS - Unicode", __func__);
		mess->messageDCS = 0x03; //Unicode
		int k = (numberLen / 2) + 7;
		for (i = 0; i < pdu_hex[(numberLen / 2) + 6]; i++)
				mess->messageBody[i] = pdu_hex[i + k];
			
	}else{
		DEBUG_I("%s : DCS - GSM7", __func__);
		mess->messageDCS = 0x00; //GSM7
		int k = (numberLen / 2) + 7;
		message = malloc(((pdu_hex[(numberLen / 2) + 6]) * 2) + 1);
		memset(message, 0, ((pdu_hex[(numberLen / 2) + 6]) * 2) + 1);
		for (i = 0; i < pdu_hex[(numberLen / 2) + 6]; i++)
		{
			asprintf(&a, "%02x", pdu_hex[i + k]);
			DEBUG_I("%s : a = %s", __func__, a);							
			strncat(message, a, 2);		
		}
		DEBUG_I("%s : message = %s", __func__, message);
		//FIXME: Add convert GSM7 to ASCII		
		
	}

	tapi_nettext_set_net_burst(0);
	tapi_nettext_send((uint8_t *)mess);
	if (mess->messageDCS == 0x00)
	{
		free(message);
	}

	free(mess);
	free(pdu_hex);

	token = t;
}

#if 0
void ipc_sms_send_msg_complete(struct ipc_message_info *info)
{
	struct ipc_gen_phone_res *phone_res = (struct ipc_gen_phone_res *) info->data;

	if(ipc_gen_phone_res_check(phone_res) < 0) {
		ALOGE("IPC_GEN_PHONE_RES indicates error, abort request to RILJ");

		ril_request_complete(reqGetToken(info->aseq), RIL_E_GENERIC_FAILURE, NULL, 0);

		/* Release the lock so we can accept new requests */
		ril_request_sms_lock_release();
		/* Now send the next message in the queue if any */
		ril_request_send_sms_next();
	}
}

/**
 * In: IPC_SMS_SVC_CENTER_ADDR
 *   SMSC: Service Center Address, needed to send an SMS
 *
 * Out: IPC_SMS_SEND_MSG
 */
void ipc_sms_svc_center_addr(struct ipc_message_info *info)
{
	int id = ril_request_sms_get_id(info->aseq);

	char *pdu;
	int pdu_len;

	if(id < 0) {
		ALOGE("The request wasn't queued, reporting generic error!");

		ril_request_complete(reqGetToken(info->aseq), RIL_E_GENERIC_FAILURE, NULL, 0);

		/* Release the lock so we can accept new requests */
		ril_request_sms_lock_release();
		/* Now send the next message in the queue if any */
		ril_request_send_sms_next();

		return;
	}

	ALOGD("Completing the request");

	pdu = ril_request_sms[id].pdu;
	pdu_len = ril_request_sms[id].pdu_len;

	/* We need to clear here to prevent infinite loop, but we can't free mem yet */
	ril_request_sms_clear(id);

	ril_request_send_sms_complete(reqGetToken(info->aseq), pdu, (char *) info->data);

	/* Now it is safe to free mem */
	if(pdu != NULL)
		free(pdu);
}

/**
 * In: IPC_SMS_SEND_MSG
 *   This comes to ACK the latest sent SMS message
 */
void ipc_sms_send_msg(struct ipc_message_info *info)
{
	struct ipc_sms_deliv_report_msg *report_msg = (struct ipc_sms_deliv_report_msg *) info->data;
	RIL_SMS_Response response;
	
	RIL_Errno ril_ack_err;

	ALOGD("Got ACK for msg_tpid #%d\n", report_msg->msg_tpid);

	response.messageRef = report_msg->msg_tpid;
	response.ackPDU = NULL;
	ril_ack_err = ipc2ril_sms_ack_error(report_msg->error, &(response.errorCode));

	ril_request_complete(reqGetToken(info->aseq), ril_ack_err, &response, sizeof(response));

	/* Release the lock so we can accept new requests */
	ril_request_sms_lock_release();
	/* Now send the next message in the queue if any */
	ril_request_send_sms_next();
}

/**
 * IPC incoming SMS queue functions
 */

void ipc_sms_tpid_queue_init(void)
{
	memset(ipc_sms_tpid_queue, 0, sizeof(unsigned char) * 10);
}

void ipc_sms_tpid_queue_del(int id)
{
	if(id < 0 || id > 9) {
		ALOGD("Invalid id (%d) for the SMS tpid queue", id);
		return;
	}

	ipc_sms_tpid_queue[id] = 0;
}

int ipc_sms_tpid_queue_new(void)
{
	int id = -1;
	int i;

	/* Find the highest place in the queue */
	for(i=10 ; i > 0 ; i--) {
		if(ipc_sms_tpid_queue[i-1]) {
			break;
		}

		id = i-1;
	}

	if(id < 0) {
		ALOGE("The SMS tpid queue is full, removing the oldest tpid");

		ipc_sms_tpid_queue_del(0);

		for(i=1 ; i < 10 ; i++) {
			ALOGD("SMS tpid queue: moving %d -> %d", i, i-1);
			ipc_sms_tpid_queue[i-1] = ipc_sms_tpid_queue[i];
		}

		ipc_sms_tpid_queue_del(9);

		return 9;
	}

	return id;
}

int ipc_sms_tpid_queue_add(unsigned char sms_tpid)
{
	int id = ipc_sms_tpid_queue_new();

	ALOGD("Storing new SMS tpid in the queue at index %d\n", id);

	ipc_sms_tpid_queue[id] = sms_tpid;

	return id;
}

int ipc_sms_tpid_queue_get_next(void)
{
	int id = -1;
	int i;

	for(i=0 ; i < 10 ; i++) {
		if(ipc_sms_tpid_queue[i]) {
			id = i;
		}
	}

	if(id < 0)
		ALOGD("Nothing left on the queue!");
	else
		ALOGD("Next queued tpid is at id #%d\n", id);

	return id;
}


#endif
/**
 * Incoming SMS functions
 */

/**
 * In: IPC_SMS_INCOMING_MSG
 *   Message to notify an incoming message, with PDU
 *
 * Out: RIL_UNSOL_RESPONSE_NEW_SMS or RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT
 *   Notify RILJ about the incoming message
 */

void ipc_incoming_sms(void* data)
{
	tapiNettextInfo* nettextInfo = (tapiNettextInfo*)(data);
	char *number_smsc, *da_len, *sca, *a, *b;
	char *number_oa, *tp_oa;
	char *pdu_type, *tp_pid, *tp_dcs, *tp_scts, *tp_udl, *tp_ud, *message; 
	char pdu[400] = {0}, c[3] = {0};
	char *number, *number2, *len_char;
	char *message_tmp, *number_tmp;
	uint8_t *mess;
	unsigned int i , len_sca, len_oa, message_length;
	char n;

	number_tmp = NULL;
	message_tmp = NULL;

	/* Convert sms packet to SMS-DELIVER PDU type  format
	SMS = SCA + TPDU */
	
	//SCA
	//Convert nettextInfo->serviceCenter to SCA
	
	number_smsc = nettextInfo->serviceCenter;
	if ((strlen(number_smsc) % 2) > 0) {
		strcat(number_smsc, "F");}

	DEBUG_I("%s : number_smsc = %s", __func__, number_smsc);	

	number = malloc(strlen(number_smsc) + 1);
	memset(number, 0, strlen(number_smsc) + 1);

	i = 0;	
	
	if (number)
	{
		while (i < strlen(number_smsc)) 
		{
			a = &(number_smsc[i+1]);
			strncat(number, a, 1);
			b = &(number_smsc[i]);
			strncat(number, b, 1);
			i = i + 2;
		}
	}
	
	DEBUG_I("%s : number = %s", __func__, number);	
	
	sca = malloc(strlen(number) + 5);
	memset(sca, 0, strlen(number) + 5);

	len_sca =  (strlen(number) / 2 ) + 1;
	asprintf(&len_char, "%02X", len_sca);
	strcat(sca, len_char);
	strcat(sca, "91");
	strcat(sca, number);

	DEBUG_I("%s : sca = %s", __func__, sca);

	strcat (pdu, sca);

	if (number != NULL)	
		free (number);

	if (sca != NULL)	
		free (sca);

	len_char = NULL;
	

	//TPDU

	/* Protocol Data Unit Type (PDU Type) 
	SMS-DELIVER

	TP-MTI:   00
	TP-MMS:   04
	TP-SRI:   20
	TP-RP:    00
	TP-UDHI:  00		*/

	pdu_type = "24";
	strcat (pdu, pdu_type);

	// TP-OA: TP- Originating-Address
	//Convert nettextInfo->phoneNumber to TP-OA

	number_oa = nettextInfo->phoneNumber;

	DEBUG_I("%s : number_oa = %s", __func__, number_oa);

	n = number_oa[0];	

	if (n >= '0' && n <= '9')
	{
		len_oa =  strlen(number_oa);

		if ((strlen(number_oa) % 2) > 0) {
			strcat(number_oa, "F");}


		DEBUG_I("%s : number_oa = %s", __func__, number_oa);	

		number2 = malloc(strlen(number_oa) + 1);
		memset(number2, 0, strlen(number_oa) + 1);

		i = 0;	
		if (number2)
		{
			while (i < strlen(number_oa)) 
			{
				a = &(number_oa[i+1]);
				strncat(number2, a, 1);
				b = &(number_oa[i]);
				strncat(number2, b, 1);
				i = i + 2;
			}
		}
		
		DEBUG_I("%s : number2 = %s", __func__, number);	
	
		tp_oa = malloc(strlen(number2) + 5);
		memset(tp_oa, 0, strlen(number2) + 5);
		asprintf(&len_char, "%02X", len_oa);
		strcat(tp_oa, len_char);	
		strcat(tp_oa, "91");
		strcat(tp_oa, number2);
		DEBUG_I("%s : tp_oa = %s", __func__, tp_oa);

	}else{
		
		ascii2gsm7(number_oa, (unsigned char **)&number_tmp, strlen(number_oa));
		DEBUG_I("%s : number_tmp = %s", __func__, number_tmp);

		number2 = malloc((strlen(number_tmp)* 2) + 1);
		memset(number2, 0, (strlen(number_tmp)* 2) + 1);
		
		bin2hex((unsigned char *)number_tmp, strlen(number_tmp), number2);
		DEBUG_I("%s : number2 = %s", __func__, number2);

		tp_oa = malloc(strlen(number2)  + 5);
		memset(tp_oa, 0, strlen(number2) + 5);

		asprintf(&len_char, "%02X", strlen(number2));
		strcat(tp_oa, len_char);
		strcat(tp_oa, "D0");
		strcat(tp_oa, number2);
		DEBUG_I("%s : tp_oa = %s", __func__, tp_oa);		
	}

	strcat (pdu, tp_oa);

	if (number2 != NULL)	
		 free (number2);

	if (tp_oa != NULL)	
		free (tp_oa);

	len_char = NULL;

	//TP-PID : TP-Protocol-Identifier 

	tp_pid = "00";
	strcat (pdu, tp_pid);

	//TP-SCTS: TP-Service-Centre-Time-Stamp
 
	//FIXME: now we use fake value, need to find this in sms packet	
	tp_scts = "11101131521400";


	if (nettextInfo->nUDH == 0)
	{
		tapiNettextSingleInfo* messageInfo = (tapiNettextSingleInfo*)((uint8_t *)data + sizeof(tapiNettextInfo));

		message_length = messageInfo->messageLength;
		mess = messageInfo->messageBody;
	}else{
		tapiNettextMultiInfo* messageInfo = (tapiNettextMultiInfo*)((uint8_t *)data + sizeof(tapiNettextInfo));
	
		message_length = strlen((char *)messageInfo->messageBody);
		mess = messageInfo->messageBody;
	}

	//TP-UDL:TP-User-Data-Length

	asprintf(&tp_udl, "%02X", message_length);
	DEBUG_I("%s : tp_udl = %s", __func__, tp_udl);

	//TP-UD: TP-User Data
	//Convert messageBody to TP-UD

	message = malloc((message_length * 2) + 1);
	memset(message, 0, (message_length * 2) + 1);

	i = 0;
	while (i < message_length)
	{
		sprintf(c, "%02X",mess[i]);
		strcat(message, c);
		i++;
	}
	if (message[0] == 0x30 && message[1] == 0x34) 
	{
		/*TP-DCS: TP-Data-Coding-Scheme */
		tp_dcs = "08"; //Unicode
		DEBUG_I("%s : TP-DCS = Unicode", __func__);

		tp_ud = malloc(strlen(message) + 1);
		memset(tp_ud, 0, strlen(message) + 1);

		strcat(tp_ud,message);
		DEBUG_I("%s : tp_ud = %s", __func__, tp_ud);
	}else{
		/*TP-DCS: TP-Data-Coding-Scheme */
		tp_dcs = "00"; //gsm7
		DEBUG_I("%s : TP-DCS = GSM7", __func__);

		ascii2gsm7((char *)mess, (unsigned char **)&message_tmp, message_length);
		DEBUG_I("%s : message_tmp = %s", __func__, message_tmp);

		tp_ud = malloc((strlen(message_tmp) * 2) + 1);
		memset(tp_ud, 0, (strlen(message_tmp) * 2) + 1);

		bin2hex((unsigned char *)message_tmp, strlen(message_tmp), tp_ud);
		DEBUG_I("%s : tp_ud = %s", __func__, tp_ud);
	}

	strcat (pdu, tp_dcs);
	strcat (pdu, tp_scts);
	strcat (pdu, tp_udl);	
	strcat (pdu, tp_ud);

	DEBUG_I("%s : pdu = %s", __func__, pdu);
	
	ril_request_unsolicited(RIL_UNSOL_RESPONSE_NEW_SMS, pdu, strlen(pdu));

	if (message != NULL)	
		free (message);

	if (tp_ud != NULL)	
		free (tp_ud);

}

#if 0	

/**
 * In: RIL_REQUEST_SMS_ACKNOWLEDGE
 *   Acknowledge successful or failed receipt of SMS previously indicated
 *   via RIL_UNSOL_RESPONSE_NEW_SMS
 *
 * Out: IPC_SMS_DELIVER_REPORT
 *   Sends a SMS delivery report
 */
void ril_request_sms_acknowledge(RIL_Token t, void *data, size_t datalen)
{
	struct ipc_sms_deliv_report_msg report_msg;
	int success = ((int *)data)[0];
	int failcause = ((int *)data)[1];
	int id = ipc_sms_tpid_queue_get_next();

	if(id < 0) {
		ALOGE("There is no SMS message to ACK!");
		ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);

		return;
	}

	report_msg.type = IPC_SMS_TYPE_STATUS_REPORT;
	report_msg.error = ril2ipc_sms_ack_error(success, failcause);
	report_msg.msg_tpid = ipc_sms_tpid_queue[id];
	report_msg.unk = 0;

	ipc_gen_phone_res_expect_to_abort(reqGetId(t), IPC_SMS_DELIVER_REPORT);

	ipc_send(IPC_SMS_DELIVER_REPORT, IPC_TYPE_EXEC, (void *) &report_msg, sizeof(struct ipc_sms_deliv_report_msg), reqGetId(t));

	ipc_sms_tpid_queue_del(id);
}

/**
 * In: IPC_SMS_DELIVER_REPORT
 *   Attest that the modem successfully sent our SMS recv ACK 
 */
void ipc_sms_deliver_report(struct ipc_message_info *info)
{
	// TODO: check error code to eventually resend ACK

	ril_request_complete(reqGetToken(info->aseq), RIL_E_SUCCESS, NULL, 0);
}

/**
 * Apparently non-SMS-messages-related function
 */

void ipc_sms_device_ready(struct ipc_message_info *info)
{
	if(ril_state.radio_state == RADIO_STATE_SIM_READY) {
		ipc_send(IPC_SMS_DEVICE_READY, IPC_TYPE_SET, NULL, 0, info->aseq);
	}

	ril_tokens_check();
}

#endif

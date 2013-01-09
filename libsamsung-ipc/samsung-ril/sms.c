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

#define LOG_TAG "Mocha-RIL-SMS"
#include <utils/Log.h>

#include "samsung-ril.h"
#include "util.h"

/**
 * SMS global vars
 */

struct ril_request_sms ril_request_sms[10];
int ril_request_sms_lock = 0;

unsigned char ipc_sms_tpid_queue[10];

/**
 * Format conversion utils
 */

unsigned short ril2ipc_sms_ack_error(int success, int failcause)
{
	if(success) {
		return IPC_SMS_ACK_NO_ERROR;
	} else {
		switch(failcause) {
			case 0xD3:
				return IPC_SMS_ACK_PDA_FULL_ERROR;
			default:
				return IPC_SMS_ACK_UNSPEC_ERROR;
		}
	}
}

RIL_Errno ipc2ril_sms_ack_error(unsigned short error, int *error_code)
{
	/* error_code is defined in See 3GPP 27.005, 3.2.5 for GSM/UMTS */

	switch(error) {
		case IPC_SMS_ACK_NO_ERROR:
			*error_code = -1;
			return RIL_E_SUCCESS;
		default:
			// unknown error
			*error_code = 500;
			return RIL_E_GENERIC_FAILURE;
	}
}

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

/**
 * Outgoing SMS functions
 */

/**
 * In: RIL_REQUEST_SEND_SMS
 *   Send an SMS message.
 *
 * Out: IPC_SMS_SEND_MSG
 */
void ril_request_send_sms(RIL_Token t, void *data, size_t datalen)
{
	const char **request = (char **) data;
	char *pdu = request[1];
	int pdu_len = pdu != NULL ? strlen(pdu) : 0;
	char *smsc = request[0];
	int smsc_len = smsc != NULL ? strlen(smsc) : 0;

	if(!ril_request_sms_lock_acquire()) {
		ALOGD("The SMS lock is already taken, adding req to the SMS queue");

		ril_request_sms_add(reqGetId(t), pdu, pdu_len, smsc, smsc_len);
		return;
	}

	/* We first need to get SMS SVC before sending the message */
	if(smsc == NULL) {
		ALOGD("We have no SMSC, let's ask one");

		/* Enqueue the request */
		ril_request_sms_add(reqGetId(t), pdu, pdu_len, NULL, 0);

		ipc_send_get(IPC_SMS_SVC_CENTER_ADDR, reqGetId(t));
		
	} else {
		ril_request_send_sms_complete(t, pdu, smsc);
	}
}

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

/**
 * Complete (continue) the send_sms request (do the real sending)
 */
void ril_request_send_sms_complete(RIL_Token t, char *pdu, char *smsc)
{
	struct ipc_sms_send_msg send_msg;
	unsigned char send_msg_type = IPC_SMS_MSG_SINGLE;
	int send_msg_len;

	char *data;
	int data_len;

	char *pdu_dec;
	unsigned char pdu_dec_len;

	int pdu_len;
	unsigned char smsc_len;

	char *p;

	if(pdu == NULL || smsc == NULL) {
		ALOGE("Provided PDU or SMSC is NULL! Aborting");

		ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);

		/* Release the lock so we can accept new requests */
		ril_request_sms_lock_release();
		/* Now send the next message in the queue if any */
		ril_request_send_sms_next();

		return;
	}

	/* Setting various len vars */
	pdu_len = strlen(pdu);

	if(pdu_len / 2 > 0xff) {
		ALOGE("PDU is too large, aborting");

		ril_request_complete(t, RIL_E_GENERIC_FAILURE, NULL, 0);

		/* Release the lock so we can accept new requests */
		ril_request_sms_lock_release();
		/* Now send the next message in the queue if any */
		ril_request_send_sms_next();

		return;
	}

	pdu_dec_len = pdu_len / 2;
	smsc_len = smsc[0];
	send_msg_len = sizeof(struct ipc_sms_send_msg);

	/* Length of the final message */
	data_len = pdu_dec_len + smsc_len + send_msg_len;

	ALOGD("Sending SMS message!");

	ALOGD("data_len is 0x%x + 0x%x + 0x%x = 0x%x\n", pdu_dec_len, smsc_len, send_msg_len, data_len);

	pdu_dec = malloc(pdu_dec_len);
	hex2bin(pdu, pdu_len, pdu_dec);

	/* PDU operations */
	int pdu_tp_da_index = 2;
	unsigned char pdu_tp_da_len = pdu_dec[pdu_tp_da_index];

	if(pdu_tp_da_len > 0xff / 2) {
		ALOGE("PDU TP-DA Len failed (0x%x)\n", pdu_tp_da_len);
		goto pdu_end;
	}

	ALOGD("PDU TP-DA Len is 0x%x\n", pdu_tp_da_len);

	int pdu_tp_udh_index = pdu_tp_da_index + pdu_tp_da_len;
	unsigned char pdu_tp_udh_len = pdu_dec[pdu_tp_udh_index];
	
	if(pdu_tp_udh_len > 0xff / 2 || pdu_tp_udh_len < 5) {
		ALOGE("PDU TP-UDH Len failed (0x%x)\n", pdu_tp_udh_len);
		goto pdu_end;
	}

	ALOGD("PDU TP-UDH Len is 0x%x\n", pdu_tp_udh_len);

	int pdu_tp_udh_num_index = pdu_tp_udh_index + 4;
	unsigned char pdu_tp_udh_num = pdu_dec[pdu_tp_udh_num_index];

	if(pdu_tp_udh_num > 0xf) {
		ALOGE("PDU TP-UDH Num failed (0x%x)\n", pdu_tp_udh_num);
		goto pdu_end;
	}

	int pdu_tp_udh_seq_index = pdu_tp_udh_index + 5;
	unsigned char pdu_tp_udh_seq = pdu_dec[pdu_tp_udh_seq_index];

	if(pdu_tp_udh_seq > 0xf || pdu_tp_udh_seq > pdu_tp_udh_num) {
		ALOGE("PDU TP-UDH Seq failed (0x%x)\n", pdu_tp_udh_seq);
		goto pdu_end;
	}

	ALOGD("We are sending message %d on %d\n", pdu_tp_udh_seq, pdu_tp_udh_num);

	if(pdu_tp_udh_num > 1) {
		ALOGD("We are sending a multi-part message!");
		send_msg_type = IPC_SMS_MSG_MULTIPLE;
	}

pdu_end:
	/* Alloc and clean memory for the final message */
	data = malloc(data_len);
	memset(&send_msg, 0, sizeof(struct ipc_sms_send_msg));

	/* Fill the IPC structure part of the message */
	send_msg.type = IPC_SMS_TYPE_OUTGOING;
	send_msg.msg_type =  send_msg_type;
	send_msg.length = (unsigned char) (pdu_dec_len + smsc_len + 1);
	send_msg.smsc_len = smsc_len;

	/* Copy the other parts of the message */
	p = data;
	memcpy(p, &send_msg, send_msg_len);
	p +=  send_msg_len;
	memcpy(p, (char *) (smsc + 1), smsc_len);
	p += smsc_len;
	memcpy(p, pdu_dec, pdu_dec_len);

	ipc_gen_phone_res_expect_to_func(reqGetId(t), IPC_SMS_SEND_MSG, ipc_sms_send_msg_complete);

	ipc_send(IPC_SMS_SEND_MSG, IPC_TYPE_EXEC, data, data_len, reqGetId(t));

	free(pdu_dec);
	free(data);
}

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

void ipc_sms_incoming_msg(struct ipc_message_info *info)
{
	struct ipc_sms_incoming_msg *msg = (struct ipc_sms_incoming_msg *) info->data;
	char *pdu = ((char *) info->data + sizeof(struct ipc_sms_incoming_msg));

	int resp_length = msg->length * 2 + 1;
	char *resp = (char *) malloc(resp_length);

	bin2hex(pdu, msg->length, resp);

	ipc_sms_tpid_queue_add(msg->msg_tpid);

	if(msg->type == IPC_SMS_TYPE_POINT_TO_POINT) {
		ril_request_unsolicited(RIL_UNSOL_RESPONSE_NEW_SMS, resp, resp_length);
	} else if(msg->type == IPC_SMS_TYPE_STATUS_REPORT) {
		// FIXME: do we need to enqueue for this?

		ril_request_unsolicited(RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT, resp, resp_length);
	} else {
		ALOGE("%s: Unknown message type", __FUNCTION__);
	}

exit:
	free(resp);
}

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

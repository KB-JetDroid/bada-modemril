/**
 * This file is part of samsung-ril.
 *
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

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <cutils/sockets.h>

#define LOG_TAG "RIL-SRS"
#include <utils/Log.h>

#include "samsung-ril.h"
#include "util.h"

struct srs_server *srs_server_new(void)
{
	struct srs_server *srs_server;

	srs_server = malloc(sizeof(struct srs_server));
	memset(srs_server, 0, sizeof(struct srs_server));
	srs_server->server_fd = -1;
	srs_server->client_fd = -1;

	return srs_server;
}

void srs_server_free(struct srs_server *srs_server)
{
	if(srs_server == NULL)
		return;

	free(srs_server);
}

int srs_server_send_message(struct srs_server *srs_server, struct srs_message *message)
{
	fd_set fds;

	struct srs_header header;
	void *data;

	header.length = message->data_len + sizeof(header);
	header.group = SRS_GROUP(message->command);
	header.index = SRS_INDEX(message->command);

	data = malloc(header.length);
	memset(data, 0, header.length);

	memcpy(data, &header, sizeof(header));
	memcpy((void *) (data + sizeof(header)), message->data, message->data_len);

	FD_ZERO(&fds);
	FD_SET(srs_server->client_fd, &fds);

	select(FD_SETSIZE, NULL, &fds, NULL, NULL);

	write(srs_server->client_fd, data, header.length);

	free(data);

	return 0;
}

int srs_server_send(unsigned short command, void *data, int data_len)
{
	struct srs_server *srs_server;
	struct srs_message message;
	int rc;

	srs_server = (struct srs_server *) (srs_client->object);

	message.command = command;
	message.data = data;
	message.data_len = data_len;

	rc = srs_server_send_message(srs_server, &message);

	return rc;
}

int srs_server_recv(struct srs_server *srs_server, struct srs_message *message)
{
	void *raw_data = malloc(SRS_DATA_MAX_SIZE);
	struct srs_header *header;
	int rc;

	rc = read(srs_server->client_fd, raw_data, SRS_DATA_MAX_SIZE);
	if(rc < sizeof(struct srs_header)) {
		return -1;
	}

	header = raw_data;

	message->command = SRS_COMMAND(header);
	message->data_len = header->length - sizeof(struct srs_header);
	message->data = malloc(message->data_len);

	memcpy(message->data, raw_data + sizeof(struct srs_header), message->data_len);

	free(raw_data);

	return 0;
}

int srs_server_accept(struct srs_server *srs_server)
{
	int client_fd = -1;
	struct sockaddr_un client_addr;
	int client_addr_len;

	if(srs_server->client_fd > 0) {
		return 0;
	}

	client_fd = accept(srs_server->server_fd, (struct sockaddr_un *) &client_addr, &client_addr_len);

	if(client_fd > 0) {
		srs_server->client_fd = client_fd;
		srs_server->client_addr = client_addr;
		srs_server->client_addr_len = client_addr_len;

		return 0;
	}

	return -1;
}

void srs_control_ping(struct srs_message *message)
{
	int caffe;

	if(message->data == NULL)
		return;

	caffe=*((int *) message->data);

	if(caffe == SRS_CONTROL_CAFFE) {
		srs_server_send(SRS_CONTROL_PING, &caffe, sizeof(caffe));
	}
}

int srs_server_open(struct srs_server *srs_server)
{
	int server_fd = -1;

	int t = 0;

	while(t < 5) {
		unlink(SRS_SOCKET_NAME);
		server_fd = socket_local_server(SRS_SOCKET_NAME, ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);

		if(server_fd > 0)
			break;

		t++;
	}
	
	if(server_fd < 0)
		return -1;

	srs_server->server_fd = server_fd;

	return 0;
}

int srs_read_loop(struct ril_client *client)
{
	struct srs_server *srs_server;
	struct srs_message srs_message;
	fd_set fds;
	int rc;

	if(client == NULL) {
		LOGE("client is NULL, aborting!");
		return -1;
	}

	if(client->object == NULL) {
		LOGE("client object is NULL, aborting!");
		return -1;
	}

	srs_server = (struct srs_server *) client->object;

	while(1) {
		if(srs_server->server_fd < 0) {
			LOGE("SRS client server_fd is negative, aborting!");
			return -1;
		}

		rc = srs_server_accept(srs_server);

		LOGE("SRS server accept!");

		FD_ZERO(&fds);
		FD_SET(srs_server->client_fd, &fds);

		while(1) {
			if(srs_server->client_fd < 0)
				break;

			select(FD_SETSIZE, &fds, NULL, NULL, NULL);

			if(FD_ISSET(srs_server->client_fd, &fds)) {
				if(srs_server_recv(srs_server, &srs_message) < 0) {
					LOGE("SRS recv failed, aborting!");
					break;
				}

				LOGD("SRS recv: command=%d data_len=%d", srs_message.command, srs_message.data_len);
				hex_dump(srs_message.data, srs_message.data_len);

				srs_dispatch(&srs_message);

				if(srs_message.data != NULL)
					free(srs_message.data);
			}
		}

		if(srs_server->client_fd > 0) {
			close(srs_server->client_fd);
			srs_server->client_fd = -1;
		}

		LOGE("SRS server client ended!");
	}

	return 0;
}

int srs_create(struct ril_client *client)
{
	struct srs_server *srs_server;

	LOGD("Creating new SRS client");

	srs_server = srs_server_new();
	client->object = (void *) srs_server;

	if(srs_server_open(srs_server) < 0) {
		LOGE("%s: samsung-ril-socket server open failed", __FUNCTION__);
		return -1;
	}

	return 0;
}

int srs_destroy(struct ril_client *client)
{
	struct srs_server *srs_server = (struct srs_server *) client->object;
	
	if(srs_server == NULL)
		return 0;

	if(srs_server->client_fd)
		close(srs_server->client_fd);
	
	close(srs_server->server_fd);

	srs_server_free(srs_server);

	return 0;
}

struct ril_client_funcs srs_client_funcs = {
	.create = srs_create,
	.destroy = srs_destroy,
	.read_loop = srs_read_loop,
};

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

#ifndef _SRS_H_
#define _SRS_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <samsung-ril-socket.h>

struct srs_server {
	int server_fd;
	int client_fd;
	struct sockaddr_un client_addr;
	int client_addr_len;
};

extern struct ril_client_funcs srs_client_funcs;

struct srs_server *srs_server_new(void);
int srs_server_send(unsigned short command, void *data, int data_len);
int srs_server_recv(struct srs_server *srs_server, struct srs_message *message);
int srs_server_accept(struct srs_server *srs_server);
int srs_server_open(struct srs_server *srs_server);

void srs_control_ping(struct srs_message *message);

#endif

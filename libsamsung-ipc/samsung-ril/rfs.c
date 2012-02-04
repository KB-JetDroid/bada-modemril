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

#define LOG_TAG "RIL-RFS"
#include <utils/Log.h>

#include "samsung-ril.h"
#include "util.h"

void ipc_rfs_nv_read_item(struct ipc_message_info *info)
{
	struct ipc_rfs_io *rfs_io = (struct ipc_rfs_io *) info->data;
	struct ipc_rfs_io_confirm *rfs_io_conf;
	struct ipc_client *ipc_client;
	void *rfs_data;
	int rc;

	ipc_client = ((struct ipc_client_object *) ipc_rfs_client->object)->ipc_client;

	if(rfs_io == NULL) {
		LOGE("Error: NULL rfs_io");
		return;
	}

	rfs_io_conf = malloc(rfs_io->length + sizeof(struct ipc_rfs_io_confirm));
	memset(rfs_io_conf, 0, rfs_io->length + sizeof(struct ipc_rfs_io_confirm));
	rfs_data = rfs_io_conf + sizeof(struct ipc_rfs_io_confirm);

	LOGD("Asked to read 0x%x bytes at offset 0x%x", rfs_io->length, rfs_io->offset);
	rc = nv_data_read(ipc_client, rfs_io->offset, rfs_io->length, rfs_data);

	LOGD("Read rfs_data dump:");
	hex_dump(rfs_data, rfs_io->length);

	LOGD("Sending RFS IO Confirm message (rc is %d)", rc);
	rfs_io_conf->confirm = rc < 0 ? 0 : 1;
	rfs_io_conf->offset = rfs_io->offset;
	rfs_io_conf->length = rfs_io->length;

	ipc_rfs_send(IPC_RFS_NV_READ_ITEM, rfs_io_conf, rfs_io->length + sizeof(struct ipc_rfs_io_confirm), info->aseq);

	free(rfs_io_conf);
}

void ipc_rfs_nv_write_item(struct ipc_message_info *info)
{
	struct ipc_rfs_io *rfs_io = (struct ipc_rfs_io *) info->data;
	struct ipc_rfs_io_confirm rfs_io_conf;
	struct ipc_client *ipc_client;
	void *rfs_data;
	int rc;

	ipc_client = ((struct ipc_client_object *) ipc_rfs_client->object)->ipc_client;

	if(rfs_io == NULL) {
		LOGE("Error: NULL rfs_io");
		return;
	}

	rfs_data = info->data + sizeof(struct ipc_rfs_io);

	LOGD("Write rfs_data dump:");
	hex_dump(rfs_data, rfs_io->length);

	LOGD("Asked to write 0x%x bytes at offset 0x%x", rfs_io->length, rfs_io->offset);
	rc = nv_data_write(ipc_client, rfs_io->offset, rfs_io->length, rfs_data);

	LOGD("Sending RFS IO Confirm message (rc is %d)", rc);
	rfs_io_conf.confirm = rc < 0 ? 0 : 1;
	rfs_io_conf.offset = rfs_io->offset;
	rfs_io_conf.length = rfs_io->length;

	ipc_rfs_send(IPC_RFS_NV_WRITE_ITEM, &rfs_io_conf, sizeof(struct ipc_rfs_io_confirm), info->aseq);
}

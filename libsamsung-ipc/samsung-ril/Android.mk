# This file is part of samsung-ril.
#
# Copyright (C) 2010-2011 Joerie de Gram <j.de.gram@gmail.com>
# Copyright (C) 2011 Paul Kocialkowski <contact@oaulk.fr>
#
# samsung-ril is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# samsung-ril is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with samsung-ril.  If not, see <http://www.gnu.org/licenses/>.
#

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	samsung-ril.c \
	client.c \
	ipc.c \
	srs.c \
	util.c \
	gen.c \
	pwr.c \
	disp.c \
	misc.c \
	sat.c \
	sim.c \
	net.c \
	sms.c \
	call.c \
	snd.c \
	gprs.c \
	rfs.c

LOCAL_SHARED_LIBRARIES := \
	libcutils libutils libril

LOCAL_STATIC_LIBRARIES := libsamsung-ipc

# for asprinf
LOCAL_CFLAGS := -D_GNU_SOURCE

ifeq ($(TARGET_DEVICE),crespo)
	LOCAL_CFLAGS += -DDEVICE_CRESPO
endif
ifeq ($(TARGET_DEVICE),h1)
	LOCAL_CFLAGS += -DDEVICE_H1
endif

LOCAL_C_INCLUDES := external/libsamsung-ipc/include
LOCAL_C_INCLUDES += hardware/ril/libsamsung-ipc/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include

LOCAL_MODULE_TAGS := optional

LOCAL_PRELINK_MODULE := false

ifeq (foo,foo)
	# build shared library
	LOCAL_SHARED_LIBRARIES += \
		libcutils libnetutils libutils
	LOCAL_LDLIBS += -lpthread
	LOCAL_CFLAGS += -DRIL_SHLIB
	LOCAL_MODULE:= libsamsung-ril
	include $(BUILD_SHARED_LIBRARY)
else
	# build executable
	LOCAL_SHARED_LIBRARIES += \
		libril
	LOCAL_MODULE:= samsung-ril
	include $(BUILD_EXECUTABLE)
endif

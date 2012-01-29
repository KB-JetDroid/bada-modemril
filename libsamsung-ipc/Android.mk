# TARGET_DEVICE hardcoded since Jet's platform build system is not set for cyanogen gingerbread
TARGET_DEVICE:= jet

BUILD_IPC-MODEMCTRL := true
DEBUG := true

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libsamsung-ipc
LOCAL_MODULE_TAGS := optional

samsung-ipc_files := \
	samsung-ipc/ipc.c \
	samsung-ipc/fm_packet.c \
	samsung-ipc/proto_packet.c \
	samsung-ipc/sim_packet.c \
	samsung-ipc/ipc_packet.c \
	samsung-ipc/tapi_packet.c \
	samsung-ipc/tapi_call.c \
	samsung-ipc/tapi_nettext.c \
	samsung-ipc/tapi_network.c \
	samsung-ipc/tapi_ss.c \
	samsung-ipc/tapi_at.c \
	samsung-ipc/tapi_dmh.c \
	samsung-ipc/tapi_config.c \
	samsung-ipc/device/$(TARGET_DEVICE)/$(TARGET_DEVICE)_ipc.c

ifeq ($(TARGET_DEVICE),crespo)
	device_files := samsung-ipc/device/$(TARGET_DEVICE)/$(TARGET_DEVICE)_nv_data.c
	LOCAL_CFLAGS += -Iexternal/openssl/include
	LOCAL_LDFLAGS += -lcrypto
	LOCAL_CFLAGS += -DDEVICE_CRESPO
endif

ifeq ($(TARGET_DEVICE),wave)
	device_files := samsung-ipc/device/$(TARGET_DEVICE)/$(TARGET_DEVICE)_nv_data.c
	LOCAL_CFLAGS += -Iexternal/openssl/include
	LOCAL_LDFLAGS += -lcrypto
	LOCAL_CFLAGS += -DDEVICE_CRESPO
endif

ifeq ($(TARGET_DEVICE),h1)
	LOCAL_CFLAGS += -DDEVICE_H1
endif
ifeq ($(TARGET_DEVICE),jet)
	LOCAL_CFLAGS += -DDEVICE_JET
endif
ifeq ($(TARGET_DEVICE),wave)
	LOCAL_CFLAGS += -DDEVICE_WAVE
endif

ifeq ($(DEBUG),true)
	LOCAL_CFLAGS += -DDEBUG
	LOCAL_CFLAGS += -DDEBUG_INFO
	LOCAL_CFLAGS += -DLOG_STDOUT
endif

LOCAL_SRC_FILES := $(samsung-ipc_files) $(device_files)

LOCAL_SHARED_LIBRARIES := libutils

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include \
	$(LOCAL_PATH)/samsung-ipc

include $(BUILD_STATIC_LIBRARY)

ifeq ($(BUILD_IPC-MODEMCTRL),true)

include $(CLEAR_VARS)

LOCAL_MODULE := ipc-modemctrl
LOCAL_MODULE_TAGS := optional

modemctrl_files := tools/modemctrl.c


ifeq ($(TARGET_DEVICE),crespo)
	LOCAL_CFLAGS += -DDEVICE_CRESPO
endif
ifeq ($(TARGET_DEVICE),h1)
	LOCAL_CFLAGS += -DDEVICE_H1
endif
ifeq ($(TARGET_DEVICE),jet)
	LOCAL_CFLAGS += -DDEVICE_JET
endif
ifeq ($(TARGET_DEVICE),wave)
	LOCAL_CFLAGS += -DDEVICE_WAVE
endif
ifeq ($(DEBUG),true)
	LOCAL_CFLAGS += -DDEBUG
	LOCAL_CFLAGS += -DDEBUG_INFO
	LOCAL_CFLAGS += -DLOG_STDOUT
endif

LOCAL_SRC_FILES := $(modemctrl_files)

LOCAL_STATIC_LIBRARIES := libsamsung-ipc
#LOCAL_SHARED_LIBRARIES := libutils

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils

ifeq ($(TARGET_ARCH),arm)
LOCAL_SHARED_LIBRARIES += libdl
endif # arm

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

include $(BUILD_EXECUTABLE)

endif

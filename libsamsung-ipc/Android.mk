
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

BUILD_IPC-MODEMCTRL := true
DEBUG := true

LOCAL_MODULE := libsamsung-ipc
LOCAL_MODULE_TAGS := optional debug

samsung-ipc_files := \
	samsung-ipc/ipc.c \
	samsung-ipc/ipc_dispatch.c \
	samsung-ipc/misc.c \
	samsung-ipc/util.c \
	samsung-ipc/fm.c \
	samsung-ipc/lbs.c \
	samsung-ipc/proto.c \
	samsung-ipc/sim.c \
	samsung-ipc/drv.c \
	samsung-ipc/tm.c \
	samsung-ipc/syssec.c \
	samsung-ipc/tapi.c \
	samsung-ipc/tapi_call.c \
	samsung-ipc/tapi_nettext.c \
	samsung-ipc/tapi_network.c \
	samsung-ipc/tapi_ss.c \
	samsung-ipc/tapi_at.c \
	samsung-ipc/tapi_dmh.c \
	samsung-ipc/tapi_config.c \
	samsung-ipc/device/$(TARGET_DEVICE)/$(TARGET_DEVICE)_ipc.c


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
LOCAL_MODULE_TAGS := optional debug

modemctrl_files := tools/modemctrl.c

ifeq ($(TARGET_DEVICE),jet)
	LOCAL_CFLAGS += -DDEVICE_JET
endif
ifeq ($(TARGET_DEVICE),wave)
	LOCAL_CFLAGS += -DDEVICE_WAVE
endif

DEBUG := true

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
	liblog \
	libutils

ifeq ($(TARGET_ARCH),arm)
LOCAL_SHARED_LIBRARIES += libdl
endif # arm

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

include $(BUILD_EXECUTABLE)

endif

include $(CLEAR_VARS)

DEBUG := true

ifeq ($(DEBUG),true)
	LOCAL_CFLAGS += -DDEBUG
	LOCAL_CFLAGS += -DDEBUG_INFO
	LOCAL_CFLAGS += -DLOG_STDOUT
endif

samsung-ril_files := \
	samsung-ril/samsung-ril.c \
	samsung-ril/client.c \
	samsung-ril/ipc.c \
	samsung-ril/srs.c \
	samsung-ril/pwr.c \
	samsung-ril/call.c \
	samsung-ril/misc.c \
	samsung-ril/util.c

LOCAL_SHARED_LIBRARIES := \
	libcutils libutils libril

LOCAL_STATIC_LIBRARIES := libsamsung-ipc

# for asprinf
LOCAL_CFLAGS := -D_GNU_SOURCE

ifeq ($(TARGET_DEVICE),jet)
	LOCAL_CFLAGS += -DDEVICE_JET
endif
ifeq ($(TARGET_DEVICE),wave)
	LOCAL_CFLAGS += -DDEVICE_WAVE
endif

LOCAL_C_INCLUDES := external/bada-modemril/libsamsung-ipc/include
LOCAL_C_INCLUDES += hardware/ril/libsamsung-ipc/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/samsung-ipc
LOCAL_MODULE_TAGS := optional debug

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := $(samsung-ipc_files) $(samsung-ril_files)

ifeq (foo,foo)
	# build shared library
	LOCAL_SHARED_LIBRARIES += \
		libcutils libnetutils libutils liblog
	LOCAL_LDLIBS += -lpthread
	LOCAL_CFLAGS += -DRIL_SHLIB
	LOCAL_MODULE := libsamsung-ril
	include $(BUILD_SHARED_LIBRARY)
else
	# build executable
	LOCAL_SHARED_LIBRARIES += \
		liblog \
		libril
	LOCAL_MODULE := samsung-ril
	include $(BUILD_EXECUTABLE)
endif

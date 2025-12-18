# ====== toolchain ======
CROSS ?= arm-linux-gnueabi-
CC_ARM := $(CROSS)gcc
CC_HOST := gcc

# ====== dirs ======
ROOT := .
SRC  := $(ROOT)/src
BUILD:= $(ROOT)/build

CFLAGS := -O2 -Wall -std=gnu99

ARM_BINS := filesrv status.cgi files.cgi ctl.cgi download.cgi delete.cgi
HOST_BINS := filecli

ARM_OUT := $(addprefix $(BUILD)/,$(ARM_BINS))
HOST_OUT:= $(addprefix $(BUILD)/,$(HOST_BINS))

all: arm host

arm: $(ARM_OUT)
host: $(HOST_OUT)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/filesrv: $(SRC)/svc/filesrv.c $(SRC)/common/common.c | $(BUILD)
	$(CC_ARM) $(CFLAGS) -o $@ $^

$(BUILD)/status.cgi: $(SRC)/cgi/status.c $(SRC)/common/common.c | $(BUILD)
	$(CC_ARM) $(CFLAGS) -o $@ $^

$(BUILD)/files.cgi: $(SRC)/cgi/files.c | $(BUILD)
	$(CC_ARM) $(CFLAGS) -o $@ $^

$(BUILD)/ctl.cgi: $(SRC)/cgi/ctl.c $(SRC)/common/common.c | $(BUILD)
	$(CC_ARM) $(CFLAGS) -o $@ $^

$(BUILD)/filecli: $(SRC)/svc/filecli.c | $(BUILD)
	$(CC_HOST) $(CFLAGS) -o $@ $^

$(BUILD)/download.cgi: $(SRC)/cgi/download.c $(SRC)/common/common.c | $(BUILD)
	$(CC_ARM) $(CFLAGS) -o $@ $^

$(BUILD)/delete.cgi: $(SRC)/cgi/delete.c $(SRC)/common/common.c | $(BUILD)
	$(CC_ARM) $(CFLAGS) -o $@ $^

clean:
	rm -f $(ARM_OUT) $(HOST_OUT)

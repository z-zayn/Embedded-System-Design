# ====== toolchain ======
CROSS ?= arm-linux-
CC_ARM := $(CROSS)gcc
CC_HOST := gcc

# ====== dirs ======
ROOT := .
SRC  := $(ROOT)/src
BUILD:= $(ROOT)/build

CFLAGS := -O2 -Wall -std=gnu99

ARM_BINS := status.cgi files.cgi upload.cgi download.cgi delete.cgi

ARM_OUT := $(addprefix $(BUILD)/,$(ARM_BINS))

all: arm

arm: $(ARM_OUT)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/status.cgi: $(SRC)/cgi/status.c $(SRC)/common/common.c | $(BUILD)
	$(CC_ARM) $(CFLAGS) -o $@ $^

$(BUILD)/files.cgi: $(SRC)/cgi/files.c | $(BUILD)
	$(CC_ARM) $(CFLAGS) -o $@ $^

$(BUILD)/download.cgi: $(SRC)/cgi/download.c $(SRC)/common/common.c | $(BUILD)
	$(CC_ARM) $(CFLAGS) -o $@ $^

$(BUILD)/delete.cgi: $(SRC)/cgi/delete.c $(SRC)/common/common.c | $(BUILD)
	$(CC_ARM) $(CFLAGS) -o $@ $^

$(BUILD)/upload.cgi: $(SRC)/cgi/upload.c $(SRC)/common/common.c | $(BUILD)
	$(CC_ARM) $(CFLAGS) -o $@ $^

clean:
	rm -f $(ARM_OUT)

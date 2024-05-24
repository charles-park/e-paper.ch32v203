# /*---------------------------------------------------------------------------*/
# /**
#  * @file Makefile
#  * @author charles-park (charles.park@hardkernel.com)
#  * @brief e-paper project
#  * @version 0.1
#  * @date 2024-05-08
#  *
#  * @copyright Copyright (c) 2022
# **/
# /*---------------------------------------------------------------------------*/
TARGET_NAME ?= $(notdir $(shell pwd))
TARGET_ELF  ?= $(TARGET_NAME).elf
TARGET_HEX  ?= $(TARGET_NAME).hex
TARGET_BIN  ?= $(TARGET_NAME).bin

# Release F/W Version string
DEF_FLAGS += -D_FW_VERSION_STR_=\"V091\"

# Serial debug port(UART2) define. (PA2-Tx, PA3-Rx)
# DEF_FLAGS += -D_DEBUG_UART_PORT_=2 -D_DEBUG_UART_BAUD_=115200

# Serial debug port(UART1) define. (PA9-Tx, PA10-Rx)
DEF_FLAGS += -D_DEBUG_UART_PORT_=1 -D_DEBUG_UART_BAUD_=115200

# Hangul Font select (Hangul font 12K used. only 1 selected)
# DEF_FLAGS += -D_FONT_HANGUL_
# DEF_FLAGS += -D_FONT_HANSOFT_
# DEF_FLAGS += -D_FONT_HANBOOT_
DEF_FLAGS += -D_FONT_HANGODIC_
# DEF_FLAGS += -D_FONT_HANPIL_

# /*---------------------------------------------------------------------------*/
#	RISC-V Toolchain
#	http://www.mounriver.com/
#	http://www.mounriver.com/download
#	gcc version 12.2.0 (xPack GNU RISC-V Embedded GCC x86_64)
# /*---------------------------------------------------------------------------*/
AS := riscv-none-elf-gcc
CC := riscv-none-elf-gcc
CXX := riscv-none-elf-g++
OBJCOPY := riscv-none-elf-objcopy

BUILD_DIR ?= ./build
SRC_DIRS ?= ./Arduino ./Core ./Debug ./Peripheral ./Startup ./USBLIB ./BoardConfig ./lib_fb ./lib_epd ./User

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.S)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

FLAGS ?= -march=rv32imac -mabi=ilp32 -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wunused -Wuninitialized  -g
ASFLAGS ?= $(FLAGS) -x assembler $(INC_FLAGS) -MMD -MP
CPPFLAGS ?=  $(FLAGS) $(INC_FLAGS) -std=gnu99 -MMD -MP
LDFLAGS ?= $(FLAGS) -T ./Ld/Link.ld -nostartfiles -Xlinker --gc-sections -Wl,-Map,"$(BUILD_DIR)/CH32V203.map" --specs=nano.specs --specs=nosys.specs

$(BUILD_DIR)/$(TARGET_ELF): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	$(OBJCOPY) -Oihex   $@ $(BUILD_DIR)/$(TARGET_HEX)
	$(OBJCOPY) -Obinary $@ $(BUILD_DIR)/$(TARGET_BIN)
	cp $(BUILD_DIR)/$(TARGET_BIN) .

# /*---------------------------------------------------------------------------*/
#	Download Tool
#	https://github.com/ch32-rs/wchisp
#	wchisp flash build/app.bin
# /*---------------------------------------------------------------------------*/
# assembly
$(BUILD_DIR)/%.S.o: %.S
	$(MKDIR_P) $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEF_FLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)
	$(RM) $(TARGET_BIN)

-include $(DEPS)

MKDIR_P ?= mkdir -p

# /*---------------------------------------------------------------------------*/

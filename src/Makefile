# STM32 Makefile for GNU toolchain and openocd
#
# This Makefile fetches the Cube firmware package from ST's' website.
# This includes: CMSIS, STM32 HAL, BSPs, USB drivers and examples.
#
# Usage:
#	make cube		Download and unzip Cube firmware
#	make flash		Flash the board with OpenOCD
#	make openocd		Start OpenOCD
#	make debug		Start GDB and attach to OpenOCD
#	make dirs		Create subdirs like obj, dep, ..
#	make template		Prepare a simple example project in this dir
#
# Copyright	2015 Steffen Vogel
# License	http://www.gnu.org/licenses/gpl.txt GNU Public License
# Author	Steffen Vogel <post@steffenvogel.de>
# Link		http://www.steffenvogel.de
#

# A name common to all output files (elf, map, hex, bin, lst)
TARGET     = demo

# Take a look into $(CUBE_DIR)/Drivers/BSP for available BSPs
BOARD      = STM32F4-Discovery
BSP_BASE   = stm32f4_discovery

#OCDFLAGS   = -f board/stm32f4discovery.cfg -c "stm32f4x.cpu configure -rtos auto;"
OCDFLAGS   = -f openocd.cfg -c "stm32f4x.cpu configure -rtos auto;"
JLINKFLAGS = -device stm32f417vg -si swd -speed 4000
GDBFLAGS   =

#EXAMPLE   = Templates
EXAMPLE    = Examples/GPIO/GPIO_EXTI

# MCU family and type in various capitalizations o_O
MCU_FAMILY = stm32f4xx
MCU_LC     = stm32f407xx
MCU_MC     = STM32F407xx
MCU_UC     = STM32F407VG

# Your C files from the /src directory
SRCS       = main.c syscalls.c util.c
SRCS      += system_$(MCU_FAMILY).c
SRCS      += stm32f4xx_it.c

# usb stuff in /src
SRCS      += usbd_desc.c usbd_conf.c usbd_cdc_interface.c

# Basic HAL libraries
SRCS      += stm32f4xx_hal_rcc.c stm32f4xx_hal_rcc_ex.c stm32f4xx_hal.c stm32f4xx_hal_cortex.c stm32f4xx_hal_gpio.c stm32f4xx_hal_tim.c stm32f4xx_hal_tim_ex.c stm32f4xx_ll_usb.c stm32f4xx_hal_pcd.c stm32f4xx_hal_pcd_ex.c

# FreeRTOS
SRCS      += cmsis_os.c port.c croutine.c event_groups.c list.c queue.c tasks.c timers.c

# FreeRTOS memory manager (there are 5 to choose from)
SRCS      += heap_1.c

# OpenOCD needs some help with FreeRTOS v8+
SRCS      += FreeRTOS-openocd.c

# ST's USB Device core middleware
SRCS      += usbd_core.c usbd_ioreq.c usbd_ctlreq.c
SRCS      += usbd_cdc.c

# Directories
OCD_DIR    = /usr/share/openocd/scripts

CUBE_DIR   = cube

BSP_DIR    = $(CUBE_DIR)/Drivers/BSP/$(BOARD)
HAL_DIR    = $(CUBE_DIR)/Drivers/STM32F4xx_HAL_Driver
CMSIS_DIR  = $(CUBE_DIR)/Drivers/CMSIS
RTOS_DIR   = $(CUBE_DIR)/Middlewares/Third_Party/FreeRTOS
USB_DIR    = $(CUBE_DIR)/Middlewares/ST/STM32_USB_Device_Library/

DEV_DIR    = $(CMSIS_DIR)/Device/ST/STM32F4xx

CUBE_URL   = http://www.st.com/st-web-ui/static/active/en/st_prod_software_internet/resource/technical/software/firmware/stm32cubef4.zip

# that's it, no need to change anything below this line!

###############################################################################
# Toolchain

PREFIX     = arm-none-eabi
CC         = $(PREFIX)-gcc
AR         = $(PREFIX)-ar
OBJCOPY    = $(PREFIX)-objcopy
OBJDUMP    = $(PREFIX)-objdump
SIZE       = $(PREFIX)-size
GDB        = $(PREFIX)-gdb

OCD        = openocd
JLINK      = JLinkExe

###############################################################################
# Options

# Defines
DEFS       = -D$(MCU_MC) -DUSE_HAL_DRIVER

# Debug specific definitions for semihosting
DEFS       += -DUSE_DBPRINTF

# Include search paths (-I)
INCS       = -Isrc
INCS      += -I$(BSP_DIR)
INCS      += -I$(CMSIS_DIR)/Include
INCS      += -I$(DEV_DIR)/Include
INCS      += -I$(HAL_DIR)/Inc
INCS      += -I$(RTOS_DIR)/Source/include
INCS      += -I$(RTOS_DIR)/Source/CMSIS_RTOS
INCS      += -I$(RTOS_DIR)/Source/portable/GCC/ARM_CM4F
INCS      += -I$(USB_DIR)/Core/Inc
INCS      += -I$(USB_DIR)/Class/CDC/Inc

# Library search paths
LIBS       = -L$(CMSIS_DIR)/Lib

# Compiler flags
CFLAGS     = -Wall -g -std=c99 -O0
CFLAGS    += -Wno-unused-variable -Wno-unused-function
CFLAGS    += -mlittle-endian -mcpu=cortex-m4 -march=armv7e-m -mthumb
CFLAGS    += -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CFLAGS    += -ffunction-sections -fdata-sections
CFLAGS    += $(INCS) $(DEFS)

# Linker flags
LDFLAGS    = -Wl,--gc-sections -Wl,-Map=$(TARGET).map $(LIBS) -T$(MCU_LC).ld

# Enable Semihosting
# LDFLAGS   += --specs=rdimon.specs -lc -lrdimon

# This is *required* to get openocd/gdb to see the FreeRTOS v8+ threads (along with the FreeRTOS-openocd.c helper file)
LDFLAGS   += -Wl,--undefined=uxTopUsedPriority

# Source search paths
VPATH      = ./src
VPATH     += $(BSP_DIR)
VPATH     += $(HAL_DIR)/Src
VPATH     += $(DEV_DIR)/Source
VPATH     += $(RTOS_DIR)/Source
VPATH     += $(RTOS_DIR)/Source/CMSIS_RTOS
VPATH     += $(RTOS_DIR)/Source/portable/GCC/ARM_CM4F
VPATH     += $(RTOS_DIR)/Source/portable/MemMang
VPATH     += $(USB_DIR)/Core/Src
VPATH     += $(USB_DIR)/Class/CDC/Src


OBJS       = $(addprefix obj/,$(SRCS:.c=.o))
DEPS       = $(addprefix dep/,$(SRCS:.c=.d))

# Prettify output
V = 0
ifeq ($V, 0)
	Q = @
	P = > /dev/null
endif

###################################################

.PHONY: all dirs flash debug template clean

all: $(TARGET).hex

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $(TARGET).elf $(TARGET).hex

-include $(DEPS)

dirs: dep obj cube
dep obj src:
	@echo "[MKDIR]   $@"
	$Qmkdir -p $@

obj/%.o : %.c | dirs
	@echo "[CC]      $(notdir $<)"
	$Q$(CC) $(CFLAGS) -c -o $@ $< -MMD -MF dep/$(*F).d

$(TARGET).elf: $(OBJS)
	@echo "[LD]      $(TARGET).elf"
	$Q$(CC) $(CFLAGS) $(LDFLAGS) src/startup_$(MCU_LC).s $^ -o $@
	@echo "[OBJDUMP] $(TARGET).lst"
	$Q$(OBJDUMP) -St $(TARGET).elf >$(TARGET).lst
	@echo "[SIZE]    $(TARGET).elf"
	$(SIZE) $(TARGET).elf

openocd:
	$(OCD) -s $(OCD_DIR) $(OCDFLAGS)

flash: all
	#$(JLINK) $(JLINKFLAGS) flash.jlink
	$(OCD) -s $(OCD_DIR) $(OCDFLAGS) -c "program $(TARGET).elf verify reset"

debug:
	@if ! nc -z localhost 3333; then \
		echo "\n\t[Error] OpenOCD is not running! Start it with: 'make openocd'\n"; exit 1; \
	else \
		$(GDB)  -ex "target extended localhost:3333" \
			-ex "monitor arm semihosting enable" \
			-ex "monitor reset halt" \
			-ex "load" \
			-ex "monitor reset init" \
			$(GDBFLAGS) $(TARGET).elf; \
	fi

cube:
	rm -fr $(CUBE_DIR)
	wget -O /tmp/cube.zip $(CUBE_URL)
	unzip /tmp/cube.zip
	mv STM32Cube* $(CUBE_DIR)
	chmod -R u+w $(CUBE_DIR)
	rm -f /tmp/cube.zip

template: cube src
	cp -ri $(CUBE_DIR)/Projects/$(BOARD)/$(EXAMPLE)/Src/* src
	cp -ri $(CUBE_DIR)/Projects/$(BOARD)/$(EXAMPLE)/Inc/* src
	cp -i $(DEV_DIR)/Source/Templates/gcc/startup_$(MCU_LC).s src
	cp -i $(CUBE_DIR)/Projects/$(BOARD)/$(EXAMPLE)/TrueStudio/$(BOARD)/$(MCU_UC)_FLASH.ld $(MCU_LC).ld

clean:
	@echo "[RM]      $(TARGET).hex"; rm -f $(TARGET).hex
	@echo "[RM]      $(TARGET).elf"; rm -f $(TARGET).elf
	@echo "[RM]      $(TARGET).map"; rm -f $(TARGET).map
	@echo "[RM]      $(TARGET).lst"; rm -f $(TARGET).lst
	@echo "[RMDIR]   dep"          ; rm -fr dep
	@echo "[RMDIR]   obj"          ; rm -fr obj


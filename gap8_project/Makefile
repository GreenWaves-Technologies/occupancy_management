# Copyright (C) 2020 GreenWaves Technologies
# All rights reserved.

# This software may be modified and distributed under the terms
# of the BSD license.  See the LICENSE file for details.

#                                  #
# Application configuration flags: #
#                                  #

##  if you want to test the application with an input file from host PC use this flag:
##  Note that in this case the shutterless filtering is desactivated:
GAP_FLAGS += -DINPUT_FILE
##  To save output results to host PC
##  This is usefull for application debug
SAVE_TO_PC=1
##  In full demo mode ble should be activated and silent as well, since printfs block the execution if set over jtag
##  To disable all printf (needed to flash application to boot from flash):
#GAP_FLAGS += -DSILENT=1
##  To enable Ble communication (with the demo android app):
#GAP_FLAGS += -DUSE_BLE
##  To take an image as reference offset at every boot you can use the following flags:
#GAP_FLAGS += -DOFFSET_IMAGE_EVERY_BOOT=1 

#######################################

ifndef GAP_SDK_HOME
	$(error Source sourceme in gap_sdk first)
endif

MODEL_PREFIX=lynred
ifndef MNIST_BITS
	MNIST_BITS=16
endif

$(info Building GAP8 mode with $(MNIST_BITS) bit quantization)

# For debugging don't load an image
# Run the network with zeros
#NO_IMAGE=1

# The training of the model is slightly different depending on
# the quantization. This is because in 8 bit mode we used signed
# 8 bit so the input to the model needs to be shifted 1 bit
ifeq ($(MNIST_BITS),8)
	GAP_FLAGS += -DMNIST_8BIT
	NNTOOL_SCRIPT=model/nntool_script8
	MODEL_SUFFIX = _8BIT
else
	ifeq ($(MNIST_BITS),16)
		GAP_FLAGS += -DMNIST_16BIT
		NNTOOL_SCRIPT=model/nntool_script16
		MODEL_SUFFIX = _16BIT
	else
		$(error Don't know how to build with this bit width)
	endif
endif

include model_decl.mk

# Here we set the memory allocation for the generated kernels
# REMEMBER THAT THE L1 MEMORY ALLOCATION MUST INCLUDE SPACE
# FOR ALLOCATED STACKS!
MODEL_L1_MEMORY=40000
MODEL_L2_MEMORY=50000
MODEL_L3_MEMORY=8000000
# hram - HyperBus RAM
# qspiram - Quad SPI RAM
MODEL_L3_EXEC=hram
# hflash - HyperBus Flash
# qpsiflash - Quad SPI Flash
MODEL_L3_CONST=hflash

pulpChip = GAP
APP = PeopleDetection

APP_SRCS += main.c ImgIO.c SSDParams.c $(MODEL_SRCS) SSDBasicKernels.c $(SSD_MODEL_GEN_C) ImageDraw.c BleNotifier.c

GAP_FLAGS += -w -O2 -s -mno-memcpy -fno-tree-loop-distribute-patterns 
GAP_FLAGS += -I. -I./helpers -I$(TILER_EMU_INC) -I$(TILER_INC) -I$(GEN_PATH) -I$(MODEL_BUILD)
APP_LDFLAGS += -L$(PWD)/shutterless/ -lshutterless -lm
APP_INC += 

BOARD_NAME = gapoc_b
PMSIS_OS  ?= pulpos

FLASH_TYPE ?= HYPER_FLASH

#RUNNER_CONFIG = $(CURDIR)/config-$(TARGET_NAME).ini

ifeq ($(platform),gvsoc)
  $(info Platform is GVSOC)
  READFS_FILES=$(realpath $(MODEL_TENSORS))
  PLPBRIDGE_FLAGS = -f
  else
  READFS_FILES+=$(realpath $(MODEL_TENSORS)) $(realpath Calibration.bin)
  PLPBRIDGE_FLAGS+= -f
endif

export GAP_USE_OPENOCD=1
io=host

ifeq ($(SAVE_TO_PC),1)
	GAP_FLAGS+=-DSAVE_TO_PC=1
prepare_debug:
	mkdir -p dump_out_imgs
	rm -rf dump_out_imgs/*
else
prepare_debug:
	true
endif

#####Here we add cutom kernels that are not yet integrated into AT libraries

SSD_MODEL_GEN = SSDKernels
SSD_MODEL_GEN_C = $(addsuffix .c, $(SSD_MODEL_GEN))
SSD_MODEL_GEN_CLEAN = $(SSD_MODEL_GEN_C) $(addsuffix .h, $(SSD_MODEL_GEN))

GenSSDTile: SSDModel.c
	gcc -g -o GenSSDTile -I"$(TILER_INC)" SSDModel.c $(TILER_LIB)

$(SSD_MODEL_GEN_C): GenSSDTile
	./GenSSDTile

SSD_model: $(SSD_MODEL_GEN_C)
	cd SSDParamsGenerator && $(MAKE) all run

# all depends on the model
all:: SSD_model model prepare_debug

clean_all: clean_model clean
	rm -rf BUILD*
	cd SSDParamsGenerator && $(MAKE) clean

.PHONY: clean_all 

include model_rules.mk
include $(RULES_DIR)/pmsis_rules.mk


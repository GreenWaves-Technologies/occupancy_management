# Copyright (C) 2017 GreenWaves Technologies
# All rights reserved.

# This software may be modified and distributed under the terms
# of the BSD license.  See the LICENSE file for details.

GEN_PATH = $(TILER_GENERATOR_PATH)/CNN

ifndef MODEL_SUFFIX
  MODEL_SUFFIX=
endif

ifndef MODEL_PREFIX
  MODEL_PREFIX=GapFlow
endif

MODEL_PYTHON=python

# Increase this to improve accuracy
MODEL_BUILD = BUILD_MODEL$(MODEL_SUFFIX)
MODEL_H5 = $(MODEL_TRAIN_BUILD)/$(MODEL_PREFIX).h5
MODEL_NAME = $(MODEL_PREFIX).tflite
MODEL_TFLITE = model/$(MODEL_NAME)

TENSORS_DIR = $(MODEL_BUILD)/tensors
MODEL_TENSORS = $(MODEL_BUILD)/$(MODEL_PREFIX)_L3_Flash_Const.dat

MODEL_STATE = $(MODEL_BUILD)/$(MODEL_PREFIX).json
MODEL_SRC = $(MODEL_PREFIX)Model.c
MODEL_GEN = $(MODEL_BUILD)/$(MODEL_PREFIX)Kernels
MODEL_GEN_C = $(addsuffix .c, $(MODEL_GEN))
MODEL_GEN_CLEAN = $(MODEL_GEN_C) $(addsuffix .h, $(MODEL_GEN))
MODEL_GEN_EXE = ./GenTile
ifndef NNTOOL_SCRIPT
  NNTOOL_SCRIPT=model/nntool_script
endif
IMAGES = images
RM=rm -f

NNTOOL=nntool

# Here we set the memory allocation for the generated kernels
# REMEMBER THAT THE L1 MEMORY ALLOCATION MUST INCLUDE SPACE
# FOR ALLOCATED STACKS!
MODEL_L1_MEMORY=48000
MODEL_L2_MEMORY=307200
MODEL_L3_MEMORY=8388608
# hram - HyperBus RAM
# qspiram - Quad SPI RAM
MODEL_L3_EXEC=hram
# hflash - HyperBus Flash
# qpsiflash - Quad SPI Flash
MODEL_L3_CONST=hflash

MODEL_SRCS += $(MODEL_GEN_C)
MODEL_SRCS += $(GEN_PATH)/CNN_BiasReLULinear_BasicKernels.c
MODEL_SRCS += $(GEN_PATH)/CNN_Conv_BasicKernels.c
MODEL_SRCS += $(GEN_PATH)/CNN_Conv_DP_BasicKernels.c
MODEL_SRCS += $(GEN_PATH)/CNN_Conv_DW_BasicKernels.c
MODEL_SRCS += $(GEN_PATH)/CNN_Conv_DW_DP_BasicKernels.c
MODEL_SRCS += $(GEN_PATH)/CNN_Pooling_BasicKernels.c
MODEL_SRCS += $(GEN_PATH)/CNN_MatAlgebra.c
MODEL_SRCS += $(GEN_PATH)/CNN_SoftMax.c

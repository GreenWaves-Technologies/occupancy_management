### Copyright (C) 2020 GreenWaves Technologies
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.

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

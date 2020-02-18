# User Test
#------------------------------------

APP              = test
APP_SRCS        += thermal_eye.c 
APP_INC	        += .
APP_CFLAGS      +=

PMSIS_OS?=pulpos
BOARD_NAME=gapoc_b

GAP_FLAGS += -I$(TILER_INC) 

export GAP_USE_OPENOCD=1
io=host


prepare_debug:
	mkdir -p offset_img
	rm -rf offset_img/*
	mkdir -p dump_out_imgs
	rm -rf dump_out_imgs/*


all:: prepare_debug

include $(RULES_DIR)/pmsis_rules.mk

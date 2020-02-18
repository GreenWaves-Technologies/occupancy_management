# User Test
#------------------------------------

APP              = test
APP_SRCS        += test_thermal_eye.c BleNotifier.c
APP_INC	        += .
APP_CFLAGS      +=

PMSIS_OS?=pulpos
BOARD_NAME=gapoc_b

GAP_FLAGS += -I$(TILER_INC) 

export GAP_USE_OPENOCD=1
io=host

include $(RULES_DIR)/pmsis_rules.mk

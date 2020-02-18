#
# Copyright (C) 2017 GreenWaves Technologies
# All rights reserved.
#
# This software may be modified and distributed under the terms
# of the BSD license.  See the LICENSE file for details.
#

CC = gcc
src = $(wildcard *.c)
obj = $(src:.c=.o)

CFLAGS  = -O2 --std=c99 -Ishutterless_lib/
LDFLAGS = -L. -lshutterless -lm

EXEC=ImgsFilter


$(EXEC): $(obj)
	$(CC) $(CFLAGS) -g3 -o $@ $^ $(LDFLAGS)

all: $(EXEC)

run:
	mkdir -p filtered_raw
	mkdir -p filtered_pgm
	rm -rf filtered_raw/*
	rm -rf filtered_pgm/*
	./$(EXEC) ../gap8_capture_raw_images/offset_img/Calibration.bin ../gap8_capture_raw_images/dump_out_imgs/

clean:
	rm -f $(obj) $(EXEC) 

.DEFAULT_GOAL := all
.PHONY: all run clean

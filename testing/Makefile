#
# libhaut, Lightweight HTML tokenizer
#
# https://github.com/mickymuis/haut-html
#
# Micky Faas <micky@edukitty.org>
# Copyright 2017-2018
# Leiden Institute of Advanced Computer Science, The Netherlands.
#
# for Haut parser
HAUT_DIR = ../
.PHONY: haut

# General flags
CC = gcc
CFLAGS = -Wall -std=c99 -O2 -g -I$(HAUT_DIR)/include
LDFLAGS = -lhaut -lm -L$(HAUT_DIR)/lib

OBJS = build/runtest.o build/test.o
HEADERS = test.h
SOURCES = runtest.c test.c

all:		build/runtest tests

build/runtest:	$(OBJS)
		$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

build/%.o:	%.c $(HEADERS)
		mkdir -p build
		$(CC) $(CFLAGS) -c $< -o $@

tests:		build/runtest
		build/runtest tests/*
.PHONY: tests

clean:
		rm -rf build



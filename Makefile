#
# libhaut, Lightweight HTML tokenizer
#
# https://github.com/mickymuis/haut-html
#
# Micky Faas <micky@edukitty.org>
# Copyright 2017-2018
# Leiden Institute of Advanced Computer Science, The Netherlands.
#

CC = gcc
CFLAGS = -Wall -std=c99 -O2 -g
AR = ar rcs
LDFLAGS =

OBJS = build/haut.o build/string_util.o build/state_machine.o
HEADERS = include/haut/haut.h include/haut/string_util.h include/haut/state_machine.h 
SOURCES = src/parser_transitions.h src/lexer_transitions.h src/tag_transitions.h

all:		lib/libhaut.a

lib/libhaut.a:	$(OBJS)
		$(AR) $@ $^

build/%.o:	src/%.c $(HEADERS) $(HEADERS_INT)
		mkdir -p build
		$(CC) $(CFLAGS) -c $< -o $@

clean:
		rm -rf build
		rm -f lib/libhaut.a



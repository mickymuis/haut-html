#
# libhaut, Lightweight HTML tokenizer
#
# https://github.com/mickymuis/haut-html
#
# Micky Faas <micky@edukitty.org>
# Copyright 2017-2018
# Leiden Institute of Advanced Computer Science, The Netherlands.


# for Haut parser
HAUT_DIR = ../
.PHONY: haut

# General flags
CC = gcc
CFLAGS = -Wall -std=c99 -O2 -g -I$(HAUT_DIR)/include
LDFLAGS = -lhaut -lm -L$(HAUT_DIR)/lib

EXAMPLES = getlinks getlinks_stream html2text

all:		$(EXAMPLES)

%:		%.c $(HEADERS)
		$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:		
		rm -f $(EXAMPLES)

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
CFLAGS = -Wall -std=c99 -O2
FSM = build/fsm2array

all:		build/fsm2array example lexer parser tag entities

build/fsm2array:	fsm2array.c
			$(CC) $(CFLAGS) fsm2array.c -o $(FSM)

build/gen_tags:	gen_tags.c
		$(CC) $(CFLAGS) gen_tags.c -o build/gen_tags

build/gen_entities:	gen_entities.c
		$(CC) $(CFLAGS) gen_entities.c json.c -o build/gen_entities -lm

.PHONY: example
example:	build/fsm2array example_fsm.h
		$(CC) -E -P example_fsm.h -o build/example_fsm.in
		cat build/example_fsm.in | $(FSM) > build/example.h

.PHONY: lexer
lexer:		build/fsm2array lexer_transitions_fsm.h
		$(CC) -E -P lexer_transitions_fsm.h -o build/lexer_transitions_fsm.in
		cat build/lexer_transitions_fsm.in | $(FSM) > ../src/lexer_transitions.h

.PHONY: tag
tag:		build/fsm2array build/gen_tags html5_tags.in
		(cd build && ./gen_tags ../html5_tags.in ../../include/haut/tag.h tag_fsm.h)
		$(CC) -E -P build/tag_fsm.h -o build/tag_fsm.in
		cat build/tag_fsm.in | $(FSM) -i -b 47 > ../src/tag_transitions.h

.PHONY: parser
parser:		build/fsm2array parser_transitions_fsm.h
		$(CC) -E -P parser_transitions_fsm.h -o build/parser_transitions_fsm.in
		cat build/parser_transitions_fsm.in | $(FSM) > ../src/parser_transitions.h

.PHONY: entities
entities:	build/fsm2array build/gen_entities entities.json
		build/gen_entities entities.json ../src/entity.h build/entity_transitions_fsm.in
		cat build/entity_transitions_fsm.in | $(FSM) -i -b 59 > ../src/entity_transitions.h

clean:
		rm -f build/*




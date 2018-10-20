/*
 * Haut - a lightweight html tokenizer
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017: Leiden University
 */

#include "../include/haut/state_machine.h"
#include "../include/haut/tag.h"
#include "state.h"
#include "entity.h"

#include <inttypes.h>
#include <stdio.h>

#pragma pack(1)

#define INPUT_BITS 8 // for now

static const char* _lexer_transition[L_N_STATES][1<<INPUT_BITS] = {
    /* Here the output of the FSM-generator is inserted
     * Given a combination of current state and input character, 
     * this array gives the next state of the tokenizer */
#include "lexer_transitions.h"
};

#if 0
static const char* _lexer_script_transition[L_SCRIPT_N_STATES][1<<INPUT_BITS] = {
    /* Here the output of the FSM-generator is inserted
     * Given a combination of current state and input character, 
     * this array gives the next state of the tokenizer, in script-mode */
#include "lexer_script.h"
};

    /* Index of both lexer FSM's */
static const char*(*_lexer[])[1<<INPUT_BITS] = {
    _lexer_transition,
    _lexer_script_transition
};

#endif

static const char* _parser_transition[L_N_STATES][L_N_STATES] = {
    /* Here the output of the FSM-generator is inserted
     * Given a current and a new state,
     * this array gives the number of the event that must be emitted by the parser
     * as defined in events.h */
    #include "parser_transitions.h"
};

static uint16_t _tag_transition[][TAG__N_INPUTS] = {
    #include "tag_transitions.h"
};

static uint32_t _entity_transitions[][ENTITY__N_INPUTS] = {
    #include "entity_transitions.h"
};


int 
lexer_next_state( int lexer_state, char c ) {
    return _lexer_transition[lexer_state][(unsigned char)c][0];
}

const char* 
parser_next_state( int lexer_state, int next_lexer_state ) {
    return _parser_transition[lexer_state][next_lexer_state];
}

int 
decode_tag( const char* str, size_t len ) {

    int state =TAG_NONE;

    //printf( "decode_tag: `%.*s'\n", (int)len, str );

    for( size_t i =0; i < len; i++ ) {
        // This is the dangerous part: we expect str[i] to be 
        // in the range [TAG__FIRST_CHAR, TAG__LAST_CHAR]
        state = _tag_transition[state][str[i]-TAG__FIRST_CHAR];
    }
    
    state = _tag_transition[state][TAG__EOF];
    if( state < TAG__N )
        return state; 

    return TAG_UNKNOWN;
}

char32_t 
decode_entity( const char* str, size_t len ) {

    int state =ENTITY_NONE;

    for( size_t i =0; i < len; i++ ) {
        // This is the dangerous part: we expect str[i] to be 
        // in the range [ENTITY__FIRST_CHAR, ENTITY__LAST_CHAR]
        state = _entity_transitions[state][str[i]-ENTITY__FIRST_CHAR];
        if( state & ENTITY_SUCCESS_BIT ) {
            if( i == len -1 )
                return state &~ENTITY_SUCCESS_BIT;
            else
                break;
        }
    }
    return ENTITY_UNKNOWN;
}

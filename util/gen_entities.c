/*
 * gen_entities - Generate Finite State Machine for parsing of HTML5 entities
 *                given the original HTML5 specification on character references
 *                in JSON format
 *
 * Micky E. Faas, University of Leiden (C) 2017
 * https://github.com/mickymuis/haut-html
 *
 * This file is based on the example for the json-parser 
 * Copyright (C) 2015 Mirko Pasqualetti  All rights reserved.
 * https://github.com/udp/json-parser
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <ctype.h>

#include "json.h"

/*
 * Generate an FSM and C-header from a collection of HTML-entity definitions
 * in JSON format.
 *
 * Compile with
 *         gcc -o gen_entities gen_entities.c json.c -lm
 *
 * USAGE: ./gen_entities <input json file> <output header> <output fsm file>
 */

// The range of input ASCII characters, 
// minimal set for HTML character references
#define FIRST_CHAR 59 // ';'
#define LAST_CHAR 122 // 'z'
#define NUM_STATES 2 // Only `none' and `unknown', rest is filled later
#define SUCCESS_BIT 1UL << 31 // Most significant bit is used to signal 'decode success'

#define ONLY_ONE_CODEPOINT
#define ENUM_PREFIX "ENTITY_"

void
str_to_sym( char* str ) {
    while( *str != 0 ) {
        if( *str == '-' || *str == '.' || *str == '/' )
            *str = '_';
        else
            *str =toupper(*str);
        str++;
    }
}

typedef struct {
    FILE* fsm_file;
    FILE* header_file;
    uint32_t mask;
    int count;
} gen_entities_state_t;

static int
process_codepoints( gen_entities_state_t* state, json_value* value ) {
    if (value == NULL) {
        return 1;
    }
    if( value->type != json_array ) {
        fprintf( stderr, "ERROR: expected array `codepoints' " );
        return 1;
    }
    fprintf( state->fsm_file, "{ " );


    int length, x;
    length = value->u.object.length;

    for (x = 0; x < length; x++) {
        if( value->u.array.values[x]->type != json_integer ) {
            fprintf( stderr, "ERROR: expected integer in array `codepoints' " );
            return 1;
        }
        fprintf( state->fsm_file, "%" PRId64 "", value->u.array.values[x]->u.integer | SUCCESS_BIT );

        state->mask |=(value->u.array.values[x]->u.integer);
        state->count ++;

#ifdef ONLY_ONE_CODEPOINT
        break;
#endif
        if( x != length-1 )
            fprintf( state->fsm_file, ", ");
    }
    fprintf( state->fsm_file, " }\n" );
    return 0;

}

static int
process_entity_object( gen_entities_state_t* state, json_value* value ) {
    if (value == NULL) {
        return 1;
    }
    if( value->type != json_object ) {
        fprintf( stderr, "ERROR: expected object " );
        return 1;
    }
    int length, x;
    length = value->u.object.length;
    for (x = 0; x < length; x++) {
        /* Each object contains two members: "codepoints" and "characters" */
        if( strcmp( value->u.object.values[x].name, "codepoints" ) == 0 ) {
            if( process_codepoints(state, value->u.object.values[x].value) != 0 )
                return 1;
            return 0;
        }
    }
    fprintf( stderr, "ERROR: no such field `codepoints' " );
    return 1;
}

static int
generateFSM( gen_entities_state_t* state, json_value* value ) {
    if (value == NULL || state == NULL ) {
        return 1;
    }
    if( value->type != json_object ) {
        fprintf( stderr, "ERROR: expected root object\n" );
        return 1;
    }
    int length, x;
    length = value->u.object.length;

    /* Generate the header for the FSM file */
    fprintf( state->fsm_file, "%%! %d %d\n\n", NUM_STATES, LAST_CHAR - FIRST_CHAR ); 
    fprintf( state->fsm_file, "**, ** => { 1 }\n" );

    /* Process each JSON object member to produce an FSM transition from entity string to codepoint */
    for (x = 0; x < length; x++) {
        const char *name =value->u.object.values[x].name;
        if( name[strlen(name)-1] != ';' ) {
            continue;
        }
        fprintf( state->fsm_file, "0, \"%.*s\" => ", (int)strlen(name)-2, name+1);
        if( process_entity_object( state, value->u.object.values[x].value ) != 0 ) {
            fprintf( stderr, "in `%s'\n", name);
            return 1;
        }
    }
    return 0;
}

int 
main(int argc, char** argv)
{
    enum FILES {
        JSON_FILE,
        HEADER_FILE,
        FSM_FILE
    };
    
    char* filename[3];
    FILE *files[3];
    struct stat filestatus;
    int file_size;
    char* file_contents;
    json_char* json;
    json_value* value;

    if (argc != 4) {
        fprintf(stderr, "%s <json html entities file> <output header-file> <output fsm-file>\n", argv[0]);
        return 1;
    }

    filename[JSON_FILE] = argv[1];
    filename[HEADER_FILE] = argv[2];
    filename[FSM_FILE] = argv[3];

    /* Check the existence fo the input file and determine its size */
    if ( stat(filename[JSON_FILE], &filestatus) != 0) {
        fprintf(stderr, "File %s not found\n", filename[JSON_FILE]);
        return 1;
    }
    file_size = filestatus.st_size;
    /* Allocate a buffer large enough to hold the input file */
    file_contents = (char*)malloc(filestatus.st_size);
    if ( file_contents == NULL) {
        fprintf(stderr, "ERROR: unable to allocate %d bytes\n", file_size);
        return 1;
    }

    /* Open all three files */
    for( int i =0; i < 3; i++ ) {
        files[i] =fopen( filename[i], i==0 ? "rt" : "w" );
        if( !files[i]) {
            fprintf( stderr, "ERROR: Could not open `%s'\n", filename[i] );
            free(file_contents);
            return 1;
        }
    }
    fprintf( files[HEADER_FILE], "\
/* This file was automatically generated by gen_entities.c from `%s' \n\
 * Please do not edit this file in any way \n\
 */\n\n", filename[JSON_FILE] );

    /* Read the input JSON file */
    if ( fread(file_contents, file_size, 1, files[JSON_FILE]) != 1 ) {
        fprintf(stderr, "ERROR: Unable to read content of %s\n", filename[JSON_FILE]);
        fclose(files[JSON_FILE]);
        free(file_contents);
        return 1;
    }
    fclose(files[JSON_FILE]);

    json = (json_char*)file_contents;

    /* Run the JSON parser */
    value = json_parse(json,file_size);
    if (value == NULL) {
        fprintf(stderr, "ERROR: Unable to parse data\n");
        free(file_contents);
        exit(1);
    }

    /* Extract the HTML entities and produce the FSM output */
    gen_entities_state_t state;
    state.fsm_file = files[FSM_FILE];
    state.header_file = files[HEADER_FILE];
    state.count = state.mask =0;
    generateFSM(&state, value);
    
    /* Generate a C header to define some usefull constants.
     * These constants are to be used later by the program that includes the FSM */
    str_to_sym( filename[HEADER_FILE] );
    fprintf( files[HEADER_FILE], "#ifndef %s\n", filename[HEADER_FILE] );
    fprintf( files[HEADER_FILE], "#define %s\n\n", filename[HEADER_FILE] );
    fprintf( files[HEADER_FILE], "#define %sNONE %d\n", ENUM_PREFIX, 0 );
    fprintf( files[HEADER_FILE], "#define %sUNKNOWN %d\n\n", ENUM_PREFIX, 1 );
    fprintf( files[HEADER_FILE], "#define %sSUCCESS_BIT 0x%lx\n\n", ENUM_PREFIX, SUCCESS_BIT );
    fprintf( files[HEADER_FILE], "#define %s_N %d\n", ENUM_PREFIX, ++(state.count) );
    fprintf( files[HEADER_FILE], "#define %s_N_INPUTS %d\n", ENUM_PREFIX, LAST_CHAR - FIRST_CHAR );
    fprintf( files[HEADER_FILE], "#define %s_EOF %d\n", ENUM_PREFIX, 0 );
    fprintf( files[HEADER_FILE], "#define %s_FIRST_CHAR %d\n", ENUM_PREFIX, FIRST_CHAR );
    fprintf( files[HEADER_FILE], "#define %s_LAST_CHAR %d\n/* */\n", ENUM_PREFIX, LAST_CHAR );
    fprintf( files[HEADER_FILE], "#endif\n" );

    /* Clean up and close the remaining files */
    json_value_free(value);
    free(file_contents);
    fclose( files[HEADER_FILE] );
    fclose( files[FSM_FILE] );

    fprintf( stderr, "Used bits: 0x%x\n", state.mask );
    return 0;
}

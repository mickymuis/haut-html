/*
 * Haut - a lightweight html tokenizer
 *
 * https://github.com/mickymuis/haut-html
 *
 * Micky Faas <micky@edukitty.org>
 * Copyright 2017-2018
 * Leiden Institute of Advanced Computer Science, The Netherlands
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <haut/haut.h>

// Fill a chunk with random characters
void
makeChunk( char* buf, int size ) {
    for( int i =0; i < size; i++ ) {
        buf[i] = rand() % 256;
    }
}

int
main( int argc, char** argv ) {

    const int chunkMax =32;
    const int n =10000000;

    char* chunk;

    haut_t parser;
    haut_init( &parser );

    for( int i =0; i < n; i++ ) {
        
        // Create a chunk of a random length
        int len = (rand() % chunkMax) + 1;

        // Deliberately send a new pointer everytime!
        chunk = malloc( len * sizeof(char) );

        makeChunk( chunk, len );

        haut_parseChunk( &parser, chunk, len );

        free( chunk );

/*        if( i % 1000 ) {
            parser.state->lexer_state = rand() % 65;
        }*/

    }

    haut_destroy( &parser );

    return 0;
};

/*
 * Haut - a lightweight html tokenizer
 *
 * https://github.com/mickymuis/haut-html
 *
 * This file is part of the Haut test-suite.
 * The purpose of this test is to run large strings of random characters
 *  through the parser in order to test its robustness against malformed input.
 * In order to test a variety of conditions, each test set is prefixed
 *  with a (partial) string of valid HTML.
 *
 * Micky Faas <micky@edukitty.org>
 * Copyright 2017-2019
 * Leiden Institute of Advanced Computer Science, The Netherlands
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <haut/haut.h>

const char* PREFIX[] = {
    "<",
    "<ht",
    "<html",
    "< ",
    "<p ",
    "<p attr",
    "<p attr=",
    "<p attr=value",
    "<p attr='",
    "<p attr=\"",
    "<p>",
    "<br/",
    "<!",
    "<!-",
    "<!--",
    "<script>",
    "<!",
    "<![",
    "<![CDATA",
    "<![CDATA[",
    NULL
};

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
    const int n =10000;

    char* chunk;

    haut_t parser;
    haut_init( &parser );

    const char **prefix = PREFIX;

    while( *prefix != NULL ) {

        // Test each prefix several times
        for( int j =0; j < 10; j++ ) {
            haut_init( &parser );
            //printf( "%s", *prefix );
            haut_parseChunk( &parser, *prefix, strlen( *prefix ) );

            // Generate n chunks of random garbage
            for( int i =0; i < n; i++ ) {
                
                // Create a chunk of a random length
                int len = (rand() % chunkMax) + 1;

                // Deliberately send a new pointer every time!
                chunk = malloc( len * sizeof(char) );

                makeChunk( chunk, len );
                
                haut_parseChunk( &parser, chunk, len );

                free( chunk );

            }
            haut_destroy( &parser );
        }
        prefix++;
    }


    return 0;
};

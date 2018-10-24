/*
 * Haut - a lightweight html tokenizer
 *
 * https://github.com/mickymuis/haut-html
 *
 * Micky Faas <micky@edukitty.org>
 * Copyright 2017-2018
 * Leiden Institute of Advanced Computer Science, The Netherlands
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "test.h"

#define RESULT_STRING( r ) ((r) ? "[PASSED]" : "[FAILED]") 

#define DEFAULT_FLAGS true, false, false

bool
processFile( const char* filename, flags_t flags ) {

    /* Open the given file */
    FILE *file =fopen( filename, "r" );
    if( !file ) {
        fprintf( stderr, "ERROR: Could not open `%s'\n", filename );
        return false;
    }

    test_t t;
    memset( &t, 0, sizeof(test_t) );
    char*line =NULL;
    size_t read =0, len =0;
    char **dest =&(t.input_buf);
    size_t *dest_size =&(t.input_size);

    // We read the file in two parts seperated by the EXPECT token
    while( (read = getline( &line, &len, file ) ) != -1 ) {
        if( strncmp( "EXPECT\n", line, 7 ) == 0 ) {
            dest =&(t.expect_buf);
            dest_size =&(t.expect_size);
            if( flags.generate ) break;
            continue;
        }
        if( !*dest ) {
            *dest = malloc( sizeof(char) * read );
        } else {
            *dest = realloc( *dest, sizeof(char) * (*dest_size+read) );
        }
        memcpy( *dest+*dest_size, line, sizeof(char) * read );
        *dest_size += read;
    }  
    free( line );
//    printf( "First part:\n%.*s\nsecond part:\n%.*s\n", (int)html_size, html, (int)expect_size, expect );

    bool passed =beginTest( &t, flags );

    free( t.input_buf ); free( t.expect_buf );
    return passed;
}

int
main( int argc, char** argv ) {
    flags_t flags = { DEFAULT_FLAGS };
    int error =0;
    
    for( int i =1; i < argc; i++ ) {
        // Some very basic arguments parsing
        if( strcmp( argv[i], "-f" ) == 0 )
            flags.stop_on_error =false;
        else if( strcmp( argv[i], "-g" ) == 0 )
            flags.generate =true;
        else if( strcmp( argv[i], "-s" ) == 0 )
            flags.stream =true;
        else {
        // Everything else is treated as a filename
            bool result =processFile( argv[i], flags );
            printf( "%s %s\n", RESULT_STRING( result ), argv[i] );
            if( !result ) {
                error =1;
                if( flags.stop_on_error )
                    break;
            }
        }
    }
    
    return error;
}

#include <stdio.h>
#include <string.h>
#include "entities.h"
#include <sys/types.h>

int decode_entity( const char* str, size_t len );
int decode_tag( const char* str, size_t len );

int
main( int argc, char** argv ) {
    if( argc < 2 ) {
        fprintf( stderr, "Usage: %s <string>\n", argv[0] );
        return -1;
    }

    const char* str =argv[1];
    int e =decode_tag( str, strlen( str ) );

    printf( "Decoded `%s' to %d\n", str, e );

    return 0;
}

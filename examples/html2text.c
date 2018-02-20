/*
 * html2text - Example program using Haut
 *
 * https://github.com/mickymuis/haut-html
 *
 * Micky Faas <micky@edukitty.org>
 * Copyright 2017-2018
 * Leiden Institute of Advanced Computer Science, The Netherlands
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <haut/haut.h>
#include <haut/tag.h>
#include <haut/string_util.h>

/* Usage:
 * html2text <html-file>
 *
 * Parses an HTML-document and outputs the contents of all textnodes 
 * inside the <BODY> element. This is just a simple example of extracting
 * human-readable text from HTML, which could be greatly expanded by 
 * adding support for nested lists, indentation, tables etc.
 * The demo takes a filename from the command-line or reads from stdin.
 */

#define BUFFER_SIZE 1024

/* We define this type so we can store additional information during parsing.
 * An instance of this type is passed around whenever one of the event-functions 
 * are called. */

typedef struct {
    bool inBody;
    bool insideLine;
} user_data_t;

/* This function is called everytime the parser has processed an opening element.
 * Arguments are a pointer @p to the parser object, its @tag and the raw contents @name.
 * @tag contains is an enumerated type whose values are defined in haut/tag.h and
 * can be any of the 150 HTML5 standard tag names.
 */
void
myElementOpen( haut_t* p, haut_tag_t tag, strfragment_t* name ) {

    /* Obtain a pointer to our custom data-type stored inside the parser */
    user_data_t* user_data =(user_data_t*)p->userdata;
    
    /* Although we are mainly interested in textnodes, the context provided by
     * the enclosing HTML-element gives us additional information about the text
     * we are extracting (e.g.: is it inside the BODY, is it a table, etc.)
     */
    switch( tag ) {
        case TAG_BODY:
            user_data->inBody =true;
            break;
        /* Beginning these elements starts a new line */
        case TAG_UL:
        case TAG_OL:
        case TAG_BR:
            printf( "\n" );
            user_data->insideLine =false;
            break;
        case TAG_LI:
            printf( "\t- " );
            break;
        case TAG_A:
            if( user_data->insideLine ) 
                printf( " " );
            printf( "[" );
            user_data->insideLine =false;
            break;
        /* Now add your own states for processing tables, nested lists etc. */
        default:
            break;
    }
}

/* This function is called everytime an element closes.
 * Arguments are a pointer @p to the parser object, its @tag and the raw contents @name.
 */
void
myElementClose( haut_t* p, haut_tag_t tag, strfragment_t* name ) {
    user_data_t* user_data =(user_data_t*)p->userdata;
    
    switch( tag ) {
        case TAG_BODY:
            user_data->inBody =false;
            break;
        /* Closing these elements ends a paragraph */
        case TAG_LI:
        case TAG_P:
            printf( "\n" );
            user_data->insideLine =false;
            break;
        case TAG_A:
            printf( "]" );
            break;
        default:
            break;
    }
}

/* This function is called for every piece of 'inner-text' (non-HTML inside a textnode).
 * Arguments are a pointer to the parser @p and the resulting @text.
 * @text is always at most one token which is ended when any whitespace is encountered.
 * This means all words of a sentence are received separately.
 */
void 
myTextnode( haut_t* p, strfragment_t* text ) {
    user_data_t* user_data =(user_data_t*)p->userdata;
   
    /* Only extract text from the <BODY> element */
    if( !user_data->inBody ) return;
    /* Although Haut does not emit the contents of <script> and <style> elements, 
     * we filter them here anyway just to be sure */
    if( haut_currentElementTag( p ) == TAG_STYLE || haut_currentElementTag( p ) == TAG_SCRIPT ) return;
    /* We receive the seperate words from each text node, which means we must add
     * whitespace when necessary. */
    if( user_data->insideLine ) { 
        printf( " " );
    } else {    
        user_data->insideLine =true;
    }

    printf( "%.*s", (int)text->size, text->data );
}

int
main( int argc, char** argv ) {
    
    FILE* input_file = stdin;
    
    /* First the dull part; read the filename from the command-line and read its contents */
    if( argc >= 2 ) {
        const char* filename = argv[1];

        /* Open the given file and write it to our buffer */
        input_file =fopen( filename, "r" );
        if( !input_file ) {
            fprintf( stderr, "ERROR: Could not open `%s'\n", filename );
            return -1;
        }
    }

    /* Now the actual parsing begins,
     * Construct parser object and pass an instance of our user-data type.
     */
    haut_t p;
    haut_init( &p );

    user_data_t user_data;
    memset( &user_data, 0, sizeof( user_data_t ) );
    p.userdata = (void*)&user_data;

    /* Setup our event handler.
     * Haut uses a SAX-like interface, which means it will not generate a DOM-tree for us.
     * Instead, we supply the parser with callbacks of events that we are interested in.
     * If the parser encounters one of these events it will let us know by using the callback.
     */
    p.events.element_open =myElementOpen;       // function pointer of type element_open_event
    p.events.element_close =myElementClose;     // function pointer of type element_close_event
    p.events.innertext =myTextnode;             // function pointer of type innertext_event

    /* Begin the parsing */
    char buffer[BUFFER_SIZE];

    while( !feof( input_file ) ) {
        /* Here we read chunks of the file and feed it to the parser piece by piece */
        size_t length;
        if ( (length = fread( buffer, 1, BUFFER_SIZE, input_file)) == 0 ) {
            fprintf( stderr, "ERROR: read error\n" );
            return 1;
        }
        haut_parseChunk( &p, buffer, length );
    }
    printf( "\n" );

    /* Clean up */
    haut_destroy( &p );
    if( input_file != stdin )
        fclose( input_file );

    return 0;
}

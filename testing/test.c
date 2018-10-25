/*
 * Haut - a lightweight html tokenizer
 *
 * https://github.com/mickymuis/haut-html
 *
 * Micky Faas <micky@edukitty.org>
 * Copyright 2017-2018
 * Leiden Institute of Advanced Computer Science, The Netherlands
 */

#include "test.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <haut/haut.h>
#include <haut/tag.h>

/* Tests if the current generated output buffer matches the buffer of
 * expectations at the current offset. The current offset is advanced afterwards. */
void
expect( haut_t* p, const char* fmt, ... ) {
    va_list ap;
    va_start( ap, fmt );
    test_t* t =(test_t*)p->userdata;

    if( t->flags.generate ) {
        /* Only generate expectations; simply print the output from the parser */
        vprintf( fmt, ap );
        va_end( ap );
        return;
    }

    /* First we compute the number of bytes we have to reserve for the output,
     * it can be at most as many bytes as the expectation */
    const char* expect =t->expect_buf + t->expect_ptr;
    char* offset = strchr( expect, '\n' );
    size_t len;
    if( offset ) 
        len = (size_t)(offset - expect);
    else
        len = t->expect_size - t->expect_ptr;

    /* Fill the output buffer with the variable arguments send by the callee */
    strbuffer_reserve( &t->output_buf, len+1 );
    char* output =t->output_buf.data;
    vsnprintf( output, len+1, fmt, ap );
    va_end( ap );
    
    // If we mismatch the expectation and the output, we need to throw an error.
    if( strncmp( expect, output, len ) != 0 ) {

        fprintf( stderr, "Error\n-----\n Expected: %.*s\n Got:      %s\n On line %d, column %d\n",
                 (int)len, expect, output, p->position.row, p->position.col );
        // We need to escape the parser's mainloop using a long jump
        longjmp( t->return_on_mismatch, 1 );
    }
    // Advance the pointer to point at the next expectation
    t->expect_ptr += len+1;
}

/* Callback functions for the haut event handler below.
 * Each function generates a string based on the event type and data.
 * These strings are tested to match the expectations from the test file. 
 */

void    
test_document_begin_event     ( struct haut* p ){
//    fprintf( stderr, "Debug: document begin\n" );
}
void            
test_document_end_event       ( struct haut* p ){
//   fprintf( stderr, "Debug: document end\n" );
}

void            
test_element_open_close    ( struct haut* p, haut_tag_t tag, strfragment_t* name, bool close ){
    static const char* format_tag[2] =  {
        "ELEMENT OPEN TAG %d\n",
        "ELEMENT CLOSE TAG %d\n" };
    static const char* format_name[2]=  {
        "ELEMENT OPEN NAME %.*s\n",
        "ELEMENT CLOSE NAME %.*s\n" };

    if( tag == TAG_UNKNOWN ) {
        expect( p, format_name[(int)close], (int)name->size, name->data );
    } else {
        expect( p, format_tag[(int)close], (int)tag );
    }
}

void            
test_element_open_event    ( struct haut* p, haut_tag_t tag, strfragment_t* name ){
    test_element_open_close( p, tag, name, false );
}

void            
test_element_close_event   ( struct haut* p, haut_tag_t tag, strfragment_t* name ){
    test_element_open_close( p, tag, name, true );
}

void            
test_attribute_event       ( struct haut* p, strfragment_t* key, strfragment_t* value ){
    static const char* format = "ARGUMENT %.*s \"%.*s\"\n";
    static const char* format_void = "ARGUMENT %.*s VOID\n";

    if( value == NULL ) {
        expect( p, format_void, (int)key->size, key->data );
    } else {
        expect( p, format, (int)key->size, key->data, (int)value->size, value->data );
    }
}

void            
test_comment_event         ( struct haut* p, strfragment_t* text ){
    static const char* format = "COMMENT %.*s\n";

    expect( p, format, (int)text->size, text->data );
}

void            
test_innertext_event       ( struct haut* p, strfragment_t* text ){
    static const char* format = "INNERTEXT %.*s\n";

    expect( p, format, (int)text->size, text->data );
}

void            
test_cdata_event           ( struct haut* p, strfragment_t* text ){
    static const char* format = "CDATA %.*s\n";

    expect( p, format, (int)text->size, text->data );
}
void            
test_doctype_event         ( struct haut* p, strfragment_t* text ){
    static const char* format = "DOCTYPE %.*s\n";

    expect( p, format, (int)text->size, text->data );
}
void            
test_script_event          ( struct haut* p, strfragment_t* text ){
    static const char* format = "SCRIPT %.*s\n";

    expect( p, format, (int)text->size, text->data );
}

void            
test_error_event           ( struct haut* p, haut_error_t err ){
/*    printf( "Debug: Syntax error (%d) on line %d, column %d:\n", (int)err, 
            p->position.row, p->position.col );

    const int max_width = 20;
    int before = p->position.offset;
    before = before >= max_width ? max_width : before;
    int after = p->length - p->position.offset;
    after = after < 0 ? 0 : after;
    after = after >= max_width ? max_width : after;


    printf( "\t`%.*s%.*s'\n", before, (p->input + p->position.offset) - before, after, (p->input + p->position.offset));
    printf( "\t" );
    for( int i =0; i < before; i++ ) printf( " " );
    printf( "^\n" );*/
}

/* Describes the event handler structure. We need to catch all events emitted. */
static const haut_event_handler_t TEST_EVENT_HANDLER = {
    .document_begin=test_document_begin_event,
    .document_end  =test_document_end_event,
    .element_open  =test_element_open_event,
    .element_close =test_element_close_event,
    .attribute     =test_attribute_event,
    .comment       =test_comment_event,
    .innertext     =test_innertext_event,
    .doctype       =test_doctype_event,
    .script        =test_script_event,
    .cdata         =test_cdata_event,
    .error         =test_error_event,
};

/* Starts parsing the input_buf from @t and tests 
 * the parser's output against expect_buf.
 * Returns true if all expectations are met.*/
bool
beginTest( test_t* t ) {
    bool pass =true;

    haut_t p;
    haut_init( &p );
    p.userdata =(void*)t;
    p.events =TEST_EVENT_HANDLER;

    strbuffer_init( &t->output_buf );

    /* Set the buffer */
    haut_setInput( &p, (char*)t->input_buf, t->input_size );

    if( setjmp( t->return_on_mismatch ) != 0 ) {
        /* An expectation was not met when parsing */
        pass =false;

    } else {
        /* Begin the parsing */
        haut_parse( &p );
    }

    /* Clean up */
    haut_destroy( &p );
    strbuffer_free( &t->output_buf );
    return pass;
}

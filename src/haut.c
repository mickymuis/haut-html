/*
 * Haut - a lightweight html tokenizer
 *
 * https://github.com/mickymuis/haut-html
 *
 * Micky Faas <micky@edukitty.org>
 * Copyright 2017-2018
 * Leiden Institute of Advanced Computer Science, The Netherlands
 */

#include "../include/haut/haut.h"
#include "../include/haut/state_machine.h"
#include "../include/haut/tag.h"
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include "state.h"
#include "entity.h"

/* This struct contains the internal state of the parser
 * and is opaque to the user of the API */
struct haut_state {
    haut_tag_t last_tag;
    haut_error_t last_error;
    
    strfragment_t attr_key_ptr;
    strbuffer_t attr_key_buffer;
    
    // Fragment that points to the current token, 
    // can point to either token_buffer or token_chunk_ptr
    strfragment_t token_ptr;
    // Local copy of the current token (if applicable)
    strbuffer_t token_buffer;
    // Fragment that points to the current token in the current chunk
    strfragment_t token_chunk_ptr;
    // Offset used when we are parsing an entity (character reference)
    int entity_token_offset;
    // Whether we are collecting a token at all (true)
    // false if token_ptr points to a meaningfull token
    bool in_token;
    
    // Current lexer state and its 'one-entry stack'
    int lexer_state;
    int lexer_saved_state;
};

//#define DEBUG_PRINT

#if defined(_MSC_VER)
#define inline __inline
#endif

void    
default_document_begin_event     ( struct haut* p ){
#ifdef DEBUG_PRINT
    fprintf( stderr, "Debug: document begin\n" );
#endif
}
void            
default_document_end_event       ( struct haut* p ){
#ifdef DEBUG_PRINT
    fprintf( stderr, "Debug: document end\n" );
#endif
}

void            
default_element_open_event    ( struct haut* p, haut_tag_t tag, strfragment_t* name ){
#ifdef DEBUG_PRINT
    printf( "Debug: element open: `%.*s (%d)'\n", (int)name->size, name->data, tag );
#endif
}
void            
default_element_close_event   ( struct haut* p, haut_tag_t tag, strfragment_t* name ){
#ifdef DEBUG_PRINT
    printf( "Debug: element close: `%.*s (%d)'\n", (int)name->size, name->data, tag );
#endif
}

void            
default_attribute_event       ( struct haut* p, strfragment_t* key, strfragment_t* value ){
#ifdef DEBUG_PRINT
    printf( "Debug: attribute: %.*s=\"%.*s\"\n", (int)key->size, key->data, 
            (value != NULL ? (int)value->size : 0 ),
            (value != NULL ? value->data : "" ) );
#endif
}

void            
default_comment_event         ( struct haut* p, strfragment_t* text ){
#ifdef DEBUG_PRINT
    printf( "Debug: comment: `%.*s'\n", (int)text->size, text->data );
#endif
}
void            
default_innertext_event       ( struct haut* p, strfragment_t* text ){
#ifdef DEBUG_PRINT
    printf( "Debug: innertext: `%.*s'\n", (int)text->size, text->data );
#endif
}
void            
default_cdata_event           ( struct haut* p, strfragment_t* text ){
#ifdef DEBUG_PRINT
    printf( "Debug: CDATA: `%.*s'\n", (int)text->size, text->data );
#endif
}
void            
default_doctype_event         ( struct haut* p, strfragment_t* text ){
#ifdef DEBUG_PRINT
    printf( "Debug: DOCTYPE: `%.*s'\n", (int)text->size, text->data );
#endif
}
void            
default_script_event          ( struct haut* p, strfragment_t* text ){
#ifdef DEBUG_PRINT
    printf( "Debug: script: `%.*s'\n", (int)text->size, text->data );
#endif
}

void            
default_error_event           ( struct haut* p, haut_error_t err ){
#ifdef DEBUG_PRINT
    printf( "Debug: Syntax error (%d) on line %d, column %d:\n", (int)err, 
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
    printf( "^\n" );
#endif
}

static void*
default_allocator( void* userdata, size_t size ) {
    return malloc( size );
}

static void
default_deallocator( void* userdata, void* ptr ) {
    free( ptr );
}

const haut_event_handler_t DEFAULT_EVENT_HANDLER = {
    .document_begin=default_document_begin_event,
    .document_end  =default_document_end_event,
    .element_open  =default_element_open_event,
    .element_close =default_element_close_event,
    .attribute     =default_attribute_event,
    .comment       =default_comment_event,
    .innertext     =default_innertext_event,
    .doctype       =default_doctype_event,
    .script        =default_script_event,
    .cdata         =default_cdata_event,
    .error         =default_error_event,
};

const haut_opts_t DEFAULT_PARSER_OPTS = {
    .allocator      =default_allocator,
    .deallocator    =default_deallocator,
    .flags          =FLAG_NONE
};

const haut_position_t POSITION_BEGIN ={
    .row           =1,
    .col           =1,
    .offset        =0
};

/** Returns the char corresponding to the current offset in @p */
static inline char
current_char( haut_t* p ) {
    return *( p->input + p->position.offset );
}

/** Returns true if the offset in @p is past the end of the buffer */
static inline int
at_end( haut_t* p ) {
    return (p->length <= p->position.offset);
}

/** Call the error event in @p's event handler, if any */
static inline void
emit_error( haut_t* p, int error ) {
    p->state->last_error =error;
    if( p->events.error != NULL )
        p->events.error( p, error );
}

/** Set the token chunk pointer to the current position in @p + @offs
 *  The chunk pointer deals with the input buffer only, not with any locally stored data. */
static inline void
set_token_chunk_begin( haut_t* p, int offs ) {
    p->state->token_chunk_ptr.data = p->input + p->position.offset + offs;
    p->state->token_chunk_ptr.size =0;
    p->state->in_token =true;
}

/** Complete the current token by setting the end of the chunk to the current position in @p + offs
 */
static inline void
set_token_chunk_end( haut_t* p, int offs ) {
    p->state->token_chunk_ptr.size = ((p->input + p->position.offset) - p->state->token_chunk_ptr.data) + offs;
    p->state->in_token =false;
}

static inline void store_current_token( haut_t* p );
static inline void clear_current_token( haut_t* p );

/** Returns true whether @p has a token that is stored locally, as opposed to inside the input buffer */ 
static inline bool
has_stored_token( haut_t* p ) {
    return (p->state->token_ptr.data == p->state->token_buffer.data);
}

/** Begin a new token by setting the token chunck pointer to the current position in @p + offs
 *  This function is called whenever a new token is encountered in the input. */
static inline void
begin_token( haut_t* p, int offs ) {
    set_token_chunk_begin( p, offs );
    p->state->token_ptr = p->state->token_chunk_ptr;
}

/** Ends the current token by either pointing to its position in the input buffer,
 *  or storing it locally. This function is called whenever a token ends in the input. */
static inline void
end_token( haut_t* p, int offs ) {
    set_token_chunk_end( p, offs );
    
    if( has_stored_token( p ) ) {
        // We have a (partial) token stored locally
        store_current_token( p );
    } else {
        // The entire token is inside this chunk
        p->state->token_ptr = p->state->token_chunk_ptr;
    }
}

/** Append the token pointed to by the token chunk pointer, to the local token buffer.
 *  After calling this function, the current token is stored locally.
 *  Before and after a call to this function, the token chunk pointer should be adjusted
 *  by calling set_token_chuck_*() */
static inline void
store_current_token( haut_t* p ) {
    strbuffer_append( 
            &p->state->token_buffer,
            p->state->token_chunk_ptr.data,
            p->state->token_chunk_ptr.size );
    p->state->token_ptr.data = p->state->token_buffer.data;
    p->state->token_ptr.size = p->state->token_buffer.size;
}

/** Make a local copy of the data pointed to by attr_key_buffer in @p 
 *  This function is called when parsing an attribute and the key that has been parsed,
 *  needs to be saved - either if the chunk ends or an entity is encountered. */
static inline void
store_attr_key( haut_t* p ) { 
    strbuffer_clear( &p->state->attr_key_buffer );
    strbuffer_copyFragment( &p->state->attr_key_buffer, 0, &p->state->attr_key_ptr );
    if( p->state->attr_key_ptr.data == p->state->token_buffer.data )
        clear_current_token( p );
    p->state->attr_key_ptr.data = p->state->attr_key_buffer.data;
    p->state->attr_key_ptr.size = p->state->attr_key_buffer.size;
}

/** Clear both the token chunk pointer and the locally stored token, if any */ 
static inline void
clear_current_token( haut_t* p ) {
    strbuffer_clear( &p->state->token_buffer );
    p->state->token_chunk_ptr.size =0;
    p->state->token_ptr = p->state->token_chunk_ptr;
    p->state->in_token =false;
}

/** Given the new state of the parser, performs the action corresponding to the semantics of that state,
 *  Additionally, the lexer's next state may be modified (for example, if it was stored previously). */
static inline bool
dispatch_parser_action( haut_t* p, int state, int* next_lexer_state ) {
    switch( state ) {
        /* Public events */
        default:
        case P_NONE:
            break;
        case P_DOCUMENT_BEGIN:
            p->events.document_begin( p );
            break;
        case P_DOCUMENT_END:
            p->events.document_end( p );
            break;

        case P_ELEMENT_OPEN:
            end_token( p, 0 );
            p->state->last_tag =decode_tag( p->state->token_ptr.data, p->state->token_ptr.size );

            p->events.element_open( p, p->state->last_tag, &p->state->token_ptr );
            clear_current_token( p );
            break;

        case P_ELEMENT_CLOSE:
            end_token( p, 0 );
            p->state->last_tag =decode_tag( p->state->token_ptr.data, p->state->token_ptr.size );
            
            p->events.element_close( p, p->state->last_tag, &p->state->token_ptr );
            clear_current_token( p );
            break;

        case P_ATTRIBUTE:
            end_token( p, 0 );
            p->events.attribute( p, &p->state->attr_key_ptr, &p->state->token_ptr );
            p->state->attr_key_ptr.data = NULL;
            clear_current_token( p );
            break;
        
        case P_ATTRIBUTE_VOID:
            end_token( p, 0 );
            // The attribute key may already have been stored
            if( p->state->attr_key_ptr.data != NULL )
                p->events.attribute( p, &p->state->attr_key_ptr, NULL );
            // Otherwise we use the current token
            else
                p->events.attribute( p, &p->state->token_ptr, NULL );
            p->state->attr_key_ptr.data = NULL;
            clear_current_token( p );
            break;

        case P_INNERTEXT:
            end_token( p, 0 );
            p->events.innertext( p, &p->state->token_ptr );
            clear_current_token( p );
            break;

        case P_TEXT:
            break;

        case P_COMMENT:
            end_token( p, -2 ); // Exclude the trailing --
            p->state->token_ptr.data++; // Exclude the leading -
            // Double check the new token size
            if( --p->state->token_ptr.size )
                p->events.comment( p, &p->state->token_ptr );
            clear_current_token( p );
            break;
        
        case P_CDATA:
            p->events.cdata( p, &p->state->token_ptr );
            clear_current_token( p );
            break;

        case P_DOCTYPE:
            end_token( p, 0 );
            p->events.doctype( p, &p->state->token_ptr );
            clear_current_token( p );
            break;

        case P_INNERTEXT_ENTITY_BEGIN:
                p->state->lexer_saved_state = L_INNERTEXT;
        case P_ENTITY_BEGIN:
            if( !p->state->in_token ) {
                begin_token( p, 0 );
                p->state->entity_token_offset =0;
            } else {
                set_token_chunk_end( p, 0 );
                store_current_token( p );       
                p->state->entity_token_offset = p->state->token_buffer.size;
                set_token_chunk_begin( p, 0 );
            }
            if( p->state->lexer_state == L_ATTR_EQUALS )
                p->state->lexer_saved_state = L_ATTR_VALUE;
            else if( state != P_INNERTEXT_ENTITY_BEGIN )
                p->state->lexer_saved_state = p->state->lexer_state;
            break;

        case P_ENTITY:
            // Return the lexer to the token we were parsing before the entity was encountered
            *next_lexer_state = p->state->lexer_saved_state;
            end_token( p, 0 );
/*            fprintf( stderr, "DEBUG: entity token `%.*s' (%d)\n", 
                    p->state->token_ptr.size - p->state->entity_token_offset,
                    p->state->token_ptr.data + p->state->entity_token_offset,
                    p->state->token_ptr.size - p->state->entity_token_offset );*/
            char32_t entity = decode_entity( 
                    p->state->token_ptr.data + p->state->entity_token_offset + 1, 
                    p->state->token_ptr.size - p->state->entity_token_offset - 1 );
            if( entity != ENTITY_UNKNOWN ) { 
                strbuffer_t tmp;
                u32toUTF8( &tmp, entity );
                // Append the decoded entity to whatever token we were parsing
                if( has_stored_token( p ) )
                    p->state->token_buffer.size =p->state->entity_token_offset;
                strbuffer_append( &p->state->token_buffer, tmp.data, tmp.size );
                strbuffer_free( &tmp );
                p->state->token_ptr = strbuffer_to_fragment( p->state->token_buffer );
                set_token_chunk_begin( p, 1 );
            } else {
                emit_error( p, ERROR_UNKNOWN_ENTITY );
                strbuffer_t tmp;
                tmp.data =p->state->token_ptr.data + p->state->entity_token_offset;
                tmp.size =p->state->token_ptr.size - p->state->entity_token_offset;

                // Append the invalid entity-string to whatever token we were parsing
                if( has_stored_token( p ) )
                    p->state->token_buffer.size =p->state->entity_token_offset;
                strbuffer_append( &p->state->token_buffer, tmp.data, tmp.size );
                p->state->token_ptr = strbuffer_to_fragment( p->state->token_buffer );
                set_token_chunk_begin( p, 1 );
            }

            // Appearantly, due to errors in the HTML,
            // we have consumed one too many characters from the input stream
            if( entity == ENTITY_UNKNOWN || p->state->lexer_state == L_ENTITY_END_DIRTY ) {
                set_token_chunk_begin( p, 0 );
                p->state->lexer_state =*next_lexer_state;
                // Jump back to the parser main loop, 
                // causing the current character to be parsed again
                return false;
            }

            break;

        case P_ERROR:
            p->events.error( p, ERROR_SYNTAX_ERROR );
            // For some syntax errors, we actively return the lexer to its previous state,
            // in an attempt to continue the current token dispite the wrong syntax.
            switch( p->state->lexer_state ) {
                case L_ELEM:
                case L_ELEM_WS:
                case L_ATTR_KEY:
                case L_ATTR_WS:
                case L_ATTR_EQUALS:
                case L_ATTR_VALUE:
                    *next_lexer_state = p->state->lexer_state; // Ignore the current character
                    break;
                case L_SPECIAL_ELEM:
                    *next_lexer_state = L_ELEM; // Treat as a regular element
                    break;
                default:
                    break;
            }
            break;

        case P_TOKEN_BEGIN:
            if( !p->state->in_token )
                begin_token( p, 0 );
            break;
        case P_TOKEN_END:
            if( p->state->in_token )
                end_token( p, 0 );
            break;
        case P_ATTRIBUTE_KEY:
            end_token( p, 0 );
            //p->state->attr_key_ptr = p->state->token_ptr;
            p->state->attr_key_ptr.data = p->state->token_ptr.data;
            p->state->attr_key_ptr.size = p->state->token_ptr.size;
            if( p->state->token_ptr.data == p->state->token_buffer.data ) {
                store_attr_key( p );
            }
            break;
        case P_ELEMENT_END:
            if( p->state->last_tag == TAG_SCRIPT ) {
                begin_token( p, 1 );
                *next_lexer_state =L_SCRIPT;
            }
            break;
        case P_VOID_ELEMENT_END:
            break;
        case P_SCRIPT_END:
            p->events.script( p, &p->state->token_ptr );
            clear_current_token( p );
            break;
        case P_RESET_LEXER:
            return false; 
            break;
        // These three are currently unused and reserved for future use.
        case P_SAVE_TOKEN:
            set_token_chunk_end( p, -1 );
            store_current_token( p );
            break;
        case P_SAVE_LEXER_STATE:
            p->state->lexer_saved_state = p->state->lexer_state;
            break;
        case P_RESTORE_LEXER_STATE:
            *next_lexer_state = p->state->lexer_saved_state;
            break;
    }
    return true;
}

/* */

void
haut_init(  haut_t* p ) {
    memset( p, 0, sizeof( haut_t ) );
    p->opts =DEFAULT_PARSER_OPTS;
    p->events =DEFAULT_EVENT_HANDLER;
    p->position =POSITION_BEGIN;
    p->state = (struct haut_state*)malloc( sizeof( struct haut_state ) );
    memset( p->state, 0, sizeof( struct haut_state ) );
    strbuffer_init( &p->state->token_buffer );
    strbuffer_init( &p->state->attr_key_buffer );
    p->state->lexer_state = L_BEGIN;
}

void
haut_destroy( haut_t* p ) {
    strbuffer_free( &p->state->token_buffer );
    strbuffer_free( &p->state->attr_key_buffer );
    free( p->state );
}

void
haut_setInput( haut_t* p, const char* buffer, size_t len ) {
    p->input =(char*)buffer;
    p->length =len;
    p->position.offset =0;
}

void
haut_parse( haut_t* p ) {
    char c;
    int next_lexer_state;
    const char *parser_state;
    
    while( !at_end( p ) ) {

REREAD:
        c =current_char( p );
        /* Insert the character into the lexer's FSM */
        next_lexer_state =lexer_next_state( p->state->lexer_state, c );

        /* We now have two lexer states: the current and the next.
         * The parser's FSM responds on this transition by generating (a)
         * state(s) that semantically describes this transition */
        parser_state =parser_next_state( p->state->lexer_state, next_lexer_state );

        /* The parser FSM gives either zero, one or two new states that define serialized actions */
        for( int k =0; k < 2; k++ ) {
            if( !dispatch_parser_action( p, parser_state[k], &next_lexer_state ) )
                goto REREAD;
        }
        
        /* Lastly, make the lexer's next state current and advance the counters */
        p->state->lexer_state =next_lexer_state;
        p->position.offset++;
        if( c == '\n' ) {
            p->position.row++;
            p->position.col = 1;
        } else if( c != '\r' )
            p->position.col++;
    }
}

void
haut_parseChunk( haut_t* p, const char* buffer, size_t len ) {
    /* In this function, we parse a part of the input (chunk)
     * and save the state for future calls */
    
    haut_setInput( p, buffer, len );

    /* We were inside a token the last time, continue it by correcting the pointer
     */
    if( p->state->in_token )
        set_token_chunk_begin( p, 0 );
    haut_parse( p );
    /* Attributes consist of two tokens (key, value). The key needs to be saved separately
     */
    if( p->state->attr_key_ptr.data && p->state->attr_key_ptr.data != p->state->attr_key_buffer.data ) {
        store_attr_key( p );
    }
    /* Save the current token, if any */
    if( p->state->in_token ) {
        // Partial token
        set_token_chunk_end( p, 0 );
        store_current_token( p );
        set_token_chunk_begin( p, 0 );
    }
    else if( !has_stored_token( p ) && p->state->token_ptr.size ) {
        // Completed token
        clear_current_token( p );
        store_current_token( p );
    }
}

haut_tag_t
haut_currentElementTag( haut_t* p ) {
    return p->state->last_tag;
}

void
haut_setOpts( haut_t* p, haut_opts_t opts ) {
    p->opts =opts;
}

void
haut_setEventHandler( haut_t* p, haut_event_handler_t e ) {
    p->events =e;
}

void
haut_enable( haut_t* p, haut_flag_t flag ) {
    p->opts.flags |= flag;
}

void
haut_disable( haut_t* p, haut_flag_t flag ) {
    p->opts.flags &= ~flag;
}


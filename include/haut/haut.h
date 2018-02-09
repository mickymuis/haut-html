/*
 * Haut - a lightweight html tokenizer
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, Leiden University
 */

#ifndef LMTH_H
#define LMTH_H

#include "string_util.h"

/** Enumeration of all possible errors thrown by the error event */
enum haut_error {
    ERROR_NONE          =0,
    ERROR_SYNTAX_ERROR,
    ERROR_UNKNOWN_TAG,
    ERROR_UNKNOWN_ENTITY
};

typedef enum haut_error haut_error_t;

/* List of possible standard HTML5 tags is defined in haut/tag.h */
typedef int haut_tag_t;
struct haut;

typedef void*           (*allocatorfunc)         ( void* userdata, size_t size );
typedef void            (*deallocatorfunc)       ( void* userdata, void* ptr );

/* Function definitions for event handlers */
typedef void            (*document_begin_event)  ( struct haut* );
typedef void            (*document_end_event)    ( struct haut* );

typedef void            (*element_open_event)    ( struct haut*, haut_tag_t tag, strfragment_t* name );
typedef void            (*element_close_event)   ( struct haut*, haut_tag_t tag, strfragment_t* name );

typedef void            (*attribute_event)       ( struct haut*, strfragment_t* key, strfragment_t* value );

typedef void            (*comment_event)         ( struct haut*, strfragment_t* text );
typedef void            (*innertext_event)       ( struct haut*, strfragment_t* text );
typedef void            (*cdata_event)           ( struct haut*, strfragment_t* text );
typedef void            (*doctype_event)         ( struct haut*, strfragment_t* text );
typedef void            (*script_event)          ( struct haut*, strfragment_t* text );

typedef void            (*error_event)           ( struct haut*, haut_error_t err );


typedef struct {
    document_begin_event document_begin;
    document_end_event  document_end;
    element_open_event  element_open;
    element_close_event element_close;
    attribute_event     attribute;
    comment_event       comment;
    innertext_event     innertext;
    cdata_event         cdata;
    doctype_event       doctype;
    script_event        script;
    error_event         error;
} haut_event_handler_t;

extern const haut_event_handler_t DEFAULT_EVENT_HANDLER;

/** This structure is used to pass optional options to the parser.
 *  It is reserved for future use */
typedef struct {
    allocatorfunc       allocator;
    deallocatorfunc     deallocator;
    int                 flags;
} haut_opts_t;

extern const haut_opts_t DEFAULT_PARSER_OPTS;

/** List of optional flags to the parser */
typedef enum {
    FLAG_NONE                   = 0,
    FLAG_ACCUMULATE_INNERTEXT   = 1 // Reserved for future use
} haut_flag_t;

typedef struct {
    unsigned int row;
    unsigned int col;
    size_t offset;
} haut_position_t;

extern const haut_position_t POSITION_BEGIN;

struct haut_state;
/** haut_t defines the principle parser object that holds all state information.
 *  Initialize this structure used haut_init() */
struct haut {
	haut_event_handler_t events;
	haut_opts_t opts;

        void* userdata;

        struct haut_state* state;

        char* input;
        size_t length;

        haut_position_t position;
};
typedef struct haut haut_t;

void
haut_init(  haut_t* p );

void
haut_destroy( haut_t* p );

void
haut_setInput( haut_t* p, const char* buffer, size_t len );

void
haut_parse( haut_t* p );

void
haut_parseChunk( haut_t* p, const char* buffer, size_t len );

haut_tag_t
haut_currentElementTag( haut_t* p );

void
haut_setOpts( haut_t* p, haut_opts_t opts );

void
haut_setEventHandler( haut_t* p, haut_event_handler_t e );


// These functions are reserved for future use

void
haut_enable( haut_t* p, haut_flag_t flag );

void
haut_disable( haut_t* p, haut_flag_t flag );


#endif

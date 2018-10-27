// This file is a finite state machine in the `fsm2array' format
// In order to build it, it has to be pre-processed using `gcc -E -P'
// The preprocessed file can then be fed to fsm2array
// The resulting transition table can be included inside a C-style array notation
#include "../src/state.h"
// Define the number of states and the number of inputs
%! L_N_STATES L_N_STATES


// This FSM defines the parser transitions that are emitted by the HTML lexer
// The input tuples consist of the current state and the new state of the lexer
// The output gives a tuple of the new parser state, optional recovery state and recovery character

//
// Generalized transitions
//
^L_ERROR, L_ERROR               => { P_ERROR }
**, **                          => { P_NONE, P_NONE, P_NONE }

//
// Transitions that begin a token
//

L_BEGIN, ^L_BEGIN               => { P_DOCUMENT_BEGIN }

// Elements
^L_ELEM, L_ELEM                 => { P_TOKEN_BEGIN }
^L_CLOSE_ELEM, L_CLOSE_ELEM     => { P_TOKEN_BEGIN }

// Attribute key
^L_ATTR_KEY, L_ATTR_KEY         => { P_TOKEN_BEGIN }

// Attribute values
^L_ATTR_VALUE, L_ATTR_VALUE                             => { P_TOKEN_BEGIN }
^L_ATTR_SINGLE_QUOTE_VALUE, L_ATTR_SINGLE_QUOTE_VALUE   => { P_TOKEN_BEGIN }
^L_ATTR_DOUBLE_QUOTE_VALUE, L_ATTR_DOUBLE_QUOTE_VALUE   => { P_TOKEN_BEGIN }

// Inner text
^L_INNERTEXT, L_INNERTEXT      => { P_TOKEN_BEGIN }

// Comments
L_COMMENT_BEGIN, L_COMMENT       => { P_TOKEN_BEGIN }

// DOCTYPE
L_DOCTYPE_E, L_DOCTYPE_DECLARATION => { P_TOKEN_BEGIN }

// CDATA
L_CDATA_LBRACKET2, L_CDATA      => { P_TOKEN_BEGIN }
L_CDATA, L_CDATA_RBRACKET1      => { P_TOKEN_END }

// Entities
^L_ENTITY, L_ENTITY             => { P_ENTITY_BEGIN }
L_WHITESPACE, L_ENTITY          => { P_INNERTEXT_ENTITY_BEGIN }
L_INNERTEXT, L_ENTITY           => { P_INNERTEXT_ENTITY_BEGIN }
L_ELEM_END, L_ENTITY            => { P_INNERTEXT_ENTITY_BEGIN }

//
// Transitions that result in a complete token
//

// Attribute key
L_ATTR_KEY, L_ATTR_WS           => { P_ATTRIBUTE_KEY }
L_ATTR_KEY, L_ATTR_EQUALS       => { P_ATTRIBUTE_KEY }

// Attribute key without value
L_ATTR_WS, L_ATTR_KEY           => { P_ATTRIBUTE_VOID, P_TOKEN_BEGIN }
L_ATTR_WS, L_ELEM_END           => { P_ATTRIBUTE_VOID, P_ELEMENT_END }
L_ATTR_WS, L_CLOSE_ELEM_SELF    => { P_ATTRIBUTE_VOID }
L_ATTR_KEY, L_ELEM_END          => { P_ATTRIBUTE_VOID, P_ELEMENT_END }
L_ATTR_KEY, L_CLOSE_ELEM_SELF   => { P_ATTRIBUTE_VOID }
L_ATTR_EQUALS, L_ELEM_END       => { P_ATTRIBUTE_VOID, P_ELEMENT_END }
L_ATTR_EQUALS, L_CLOSE_ELEM_SELF => { P_ATTRIBUTE_VOID }

// Unquoted attribute value
L_ATTR_VALUE, L_ELEM_WS         => { P_ATTRIBUTE }
L_ATTR_VALUE, L_ELEM_END        => { P_ATTRIBUTE, P_ELEMENT_END }
L_ATTR_VALUE, L_CLOSE_ELEM_SELF => { P_ATTRIBUTE }

// Quoted attribute values
L_ATTR_SINGLE_QUOTE_VALUE,L_ELEM_WS => { P_ATTRIBUTE }
L_ATTR_DOUBLE_QUOTE_VALUE,L_ELEM_WS => { P_ATTRIBUTE }
// Empty attribute values
L_ATTR_SINGLE_QUOTE_OPEN,L_ELEM_WS => { P_TOKEN_BEGIN, P_ATTRIBUTE }
L_ATTR_DOUBLE_QUOTE_OPEN,L_ELEM_WS => { P_TOKEN_BEGIN, P_ATTRIBUTE }

// Opening tag
L_ELEM, L_ELEM_WS               => { P_ELEMENT_OPEN }
L_ELEM, L_ELEM_END              => { P_ELEMENT_OPEN, P_ELEMENT_END }
L_ELEM, L_CLOSE_ELEM_SELF       => { P_ELEMENT_OPEN }
^L_CLOSE_ELEM_SELF, L_ELEM_END  => { P_ELEMENT_END }
L_CLOSE_ELEM_SELF, L_ELEM_END   => { P_VOID_ELEMENT_END }

// Closing tag
L_CLOSE_ELEM, L_CLOSE_ELEM_END   => { P_ELEMENT_CLOSE }
L_CLOSE_ELEM, L_CLOSE_ELEM_SKIP  => { P_ELEMENT_CLOSE }

// Inner text
L_INNERTEXT, **                  => { P_INNERTEXT, P_TEXT }
L_INNERTEXT, L_INNERTEXT         => { P_NONE }
L_INNERTEXT, L_WHITESPACE        => { P_INNERTEXT }

// Comment
L_COMMENT_END_DASH2, L_ELEM_END  => { P_COMMENT }

// DOCTYPE
L_DOCTYPE_DECLARATION, L_ELEM_END => { P_DOCTYPE }

// CDATA
L_CDATA_RBRACKET2, L_ELEM_END   => { P_CDATA }

// Entity
L_ENTITY, L_ENTITY_END          => { P_ENTITY }
L_ENTITY, L_ENTITY_END_DIRTY    => { P_ENTITY }

//
// Internal parser events
//

L_SCRIPT, L_SCRIPT_LT           => { P_TOKEN_END }
L_SCRIPT_T, L_ELEM_END        => { P_SCRIPT_END }
//L_SCRIPT_STYLE_E, L_SCRIPT_END  => { P_SCRIPT_END }



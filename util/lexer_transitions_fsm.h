// This file is a finite state machine in the `fsm2array' format
// In order to build it, it has to be pre-processed using `gcc -E -P'
// The preprocessed file can then be fed to fsm2array
// The resulting transition table can be included inside a C-style array notation
#include "../src/state.h"
// Define the number of states and the number of inputs (byte)
%! L_N_STATES 256


// This FSM defines the transitions for the HTML lexer
// The input tuples consist of the current state and a single input character
// The output gives the new state

//
// Section 1 - General states
//


// Begin document
L_BEGIN, **     => { L_BEGIN } // Ignore rubbish before first element
L_BEGIN, *s     => { L_WHITESPACE }
L_BEGIN, '<'      => { L_ELEM_BEGIN }

// End document
L_END, **       => { L_END } // Trap state

// Whitespace
L_WHITESPACE, **        => { L_INNERTEXT }
L_WHITESPACE, *s        => { L_WHITESPACE }
L_WHITESPACE, '<'       => { L_ELEM_BEGIN }
L_WHITESPACE, '&'       => { L_ENTITY }

// Error recovery
L_ERROR, **     => { L_ERROR }
L_ERROR, '<'    => { L_ELEM_BEGIN }
L_ERROR, '>'    => { L_WHITESPACE }

// 
// Section 2 - General elements
//

// Element begin (we've just seen <)
// Because many pages contain malformed tagid's, we've decided to ignore the specs a little here
L_ELEM_BEGIN, **        => { L_ERROR }
L_ELEM_BEGIN, *aAd      => { L_ELEM }
L_ELEM_BEGIN, '?'       => { L_ELEM }
L_ELEM_BEGIN, ':'       => { L_ELEM }
//L_ELEM_BEGIN, **        => { L_ELEM }
L_ELEM_BEGIN, '!'       => { L_SPECIAL_ELEM } // Either doctype or <!--
L_ELEM_BEGIN, '/'       => { L_CLOSE_ELEM_BEGIN }
L_ELEM_BEGIN, *s        => { L_ELEM_BEGIN }

// Inside tag id
// Because many pages contain malformed tagid's, we've decided to ignore the specs a little here
L_ELEM, **              => { L_ERROR }
L_ELEM, *aAd            => { L_ELEM }
L_ELEM, '?'             => { L_ELEM }
L_ELEM, ':'             => { L_ELEM }
//L_ELEM, **            => { L_ELEM }
L_ELEM, *s              => { L_ELEM_WS }
L_ELEM, '/'             => { L_CLOSE_ELEM_SELF }
L_ELEM, '>'             => { L_ELEM_END }

// Whitespace after tag id, or after previous attribute declaration
L_ELEM_WS, **           => { L_ATTR_KEY }
L_ELEM_WS, *s           => { L_ELEM_WS }
L_ELEM_WS, '/'          => { L_CLOSE_ELEM_SELF }
L_ELEM_WS, '>'          => { L_ELEM_END }
L_ELEM_WS, '='          => { L_ERROR }
L_ELEM_WS, '\''         => { L_ERROR }
L_ELEM_WS, '"'          => { L_ERROR }

// After an opening tag
L_ELEM_END, **          => { L_INNERTEXT }
L_ELEM_END, *s          => { L_WHITESPACE }
L_ELEM_END, '<'         => { L_ELEM_BEGIN }
L_ELEM_END, '&'         => { L_ENTITY }

// Close tag, we have just seen </
// Because many pages contain malformed tagid's, we've decided to ignore the specs a little here
L_CLOSE_ELEM_BEGIN, **  => { L_ERROR }
L_CLOSE_ELEM_BEGIN, *aAd => { L_CLOSE_ELEM }
L_CLOSE_ELEM_BEGIN, ':' => { L_CLOSE_ELEM }
L_CLOSE_ELEM_BEGIN, '?' => { L_CLOSE_ELEM }
//L_CLOSE_ELEM_BEGIN, **  => { L_CLOSE_ELEM }
L_CLOSE_ELEM_BEGIN, *s  => { L_CLOSE_ELEM_BEGIN }

// Inside a closing tag
// Because many pages contain malformed tagid's, we've decided to ignore the specs a little here
L_CLOSE_ELEM, **        => { L_ERROR }
L_CLOSE_ELEM, *aAd      => { L_CLOSE_ELEM }
L_CLOSE_ELEM, ':'       => { L_CLOSE_ELEM }
L_CLOSE_ELEM, '?'       => { L_CLOSE_ELEM }
//L_CLOSE_ELEM, **        => { L_CLOSE_ELEM }
L_CLOSE_ELEM, *s        => { L_CLOSE_ELEM_SKIP }
L_CLOSE_ELEM, '>'       => { L_CLOSE_ELEM_END }

// Inside a closing tag after the tag id
// The HTML specification allowes but ignores any attributes in the closing tag
L_CLOSE_ELEM_SKIP, **   => { L_CLOSE_ELEM_SKIP }
L_CLOSE_ELEM_SKIP, '>'  => { L_CLOSE_ELEM_END }

// After a closing tag
L_CLOSE_ELEM_END, **    => { L_INNERTEXT }
L_CLOSE_ELEM_END, *s    => { L_WHITESPACE }
L_CLOSE_ELEM_END, '<'   => { L_ELEM_BEGIN }

// Self closing element (void element)
L_CLOSE_ELEM_SELF, **   => { L_ERROR }
L_CLOSE_ELEM_SELF, *s   => { L_CLOSE_ELEM_SELF }
L_CLOSE_ELEM_SELF, '>'  => { L_ELEM_END }

// Special elements beginning with <!
L_SPECIAL_ELEM, **      => { L_ERROR }
L_SPECIAL_ELEM, 'D'     => { L_DOCTYPE_D }
L_SPECIAL_ELEM, 'd'     => { L_DOCTYPE_D }
L_SPECIAL_ELEM, '['     => { L_CDATA_LBRACKET1 }
L_SPECIAL_ELEM, '-'     => { L_COMMENT_BEGIN }

//
// Section 3 - Attributes
//

// Attribute key
L_ATTR_KEY, **                  => { L_ATTR_KEY }
L_ATTR_KEY, *s                  => { L_ATTR_WS }
L_ATTR_KEY, '='                 => { L_ATTR_EQUALS }
L_ATTR_KEY, '>'                 => { L_ELEM_END }
L_ATTR_KEY, '/'                 => { L_CLOSE_ELEM_SELF }
L_ATTR_KEY, '<'                 => { L_ERROR }
L_ATTR_KEY, '\''                => { L_ERROR }
L_ATTR_KEY, '"'                 => { L_ERROR }

// Attribute whitespace
L_ATTR_WS, '='                  => { L_ATTR_EQUALS }
L_ATTR_WS, **                   => { L_ATTR_KEY }
L_ATTR_WS, *s                   => { L_ATTR_WS }
L_ATTR_WS, '/'                  => { L_CLOSE_ELEM_SELF }
L_ATTR_WS, '>'                  => { L_ELEM_END }
L_ATTR_WS, '<'                  => { L_ERROR }
L_ATTR_WS, '\''                 => { L_ERROR }
L_ATTR_WS, '"'                  => { L_ERROR }

// Attribute equal sign
L_ATTR_EQUALS, **               => { L_ATTR_VALUE }
L_ATTR_EQUALS, *s               => { L_ATTR_EQUALS }
L_ATTR_EQUALS, '"'              => { L_ATTR_DOUBLE_QUOTE_OPEN }
L_ATTR_EQUALS, '\''             => { L_ATTR_SINGLE_QUOTE_OPEN }
L_ATTR_EQUALS, '>'              => { L_ELEM_END }
L_ATTR_EQUALS, '`'              => { L_ERROR }
L_ATTR_EQUALS, '<'              => { L_ERROR }
L_ATTR_EQUALS, '&'              => { L_ENTITY }

// Unquoted attribute value
L_ATTR_VALUE, **                => { L_ATTR_VALUE }
L_ATTR_VALUE, *s                => { L_ELEM_WS }
L_ATTR_VALUE, '>'               => { L_ELEM_END }
L_ATTR_VALUE, '"'               => { L_ERROR }
L_ATTR_VALUE, '`'               => { L_ERROR }
L_ATTR_VALUE, '<'               => { L_ERROR }
L_ATTR_VALUE, '&'               => { L_ENTITY }
// This mistake is made so often, that we chose to allow it.
//L_ATTR_VALUE, '='               => { L_ERROR }

// Single-quoted attribute
L_ATTR_SINGLE_QUOTE_OPEN, **    => { L_ATTR_SINGLE_QUOTE_VALUE }
L_ATTR_SINGLE_QUOTE_OPEN, '\''  => { L_ELEM_WS }
L_ATTR_SINGLE_QUOTE_OPEN, '&'  => { L_ENTITY }
//L_ATTR_SINGLE_QUOTE_OPEN, '>'   => { L_ERROR }
//L_ATTR_SINGLE_QUOTE_OPEN, '/'   => { L_ERROR }

L_ATTR_SINGLE_QUOTE_VALUE, **   => { L_ATTR_SINGLE_QUOTE_VALUE }
L_ATTR_SINGLE_QUOTE_VALUE, '&'  => { L_ENTITY }
L_ATTR_SINGLE_QUOTE_VALUE, '\'' => { L_ELEM_WS }

// Double-quoted attribute
L_ATTR_DOUBLE_QUOTE_OPEN, **    => { L_ATTR_DOUBLE_QUOTE_VALUE }
L_ATTR_DOUBLE_QUOTE_OPEN, '&'  => { L_ENTITY }
L_ATTR_DOUBLE_QUOTE_OPEN, '"'  => { L_ELEM_WS }

L_ATTR_DOUBLE_QUOTE_VALUE, **   => { L_ATTR_DOUBLE_QUOTE_VALUE }
L_ATTR_DOUBLE_QUOTE_VALUE, '&'  => { L_ENTITY }
L_ATTR_DOUBLE_QUOTE_VALUE, '"' => { L_ELEM_WS }

//
// Section 4 - Text/character nodes
//

L_INNERTEXT, **         => { L_INNERTEXT }
L_INNERTEXT, *s         => { L_WHITESPACE }
L_INNERTEXT, '<'        => { L_ELEM_BEGIN }
L_INNERTEXT, '&'        => { L_ENTITY }

// 
// Section 5 - Doctype declaration
//

L_DOCTYPE_D, **         => { L_ERROR }
L_DOCTYPE_D, 'o'        => { L_DOCTYPE_O }
L_DOCTYPE_D, 'O'        => { L_DOCTYPE_O }

L_DOCTYPE_O, **         => { L_ERROR }
L_DOCTYPE_O, 'c'        => { L_DOCTYPE_C }
L_DOCTYPE_O, 'C'        => { L_DOCTYPE_C }

L_DOCTYPE_C, **         => { L_ERROR }
L_DOCTYPE_C, 't'        => { L_DOCTYPE_T }
L_DOCTYPE_C, 'T'        => { L_DOCTYPE_T }

L_DOCTYPE_T, **         => { L_ERROR }
L_DOCTYPE_T, 'y'        => { L_DOCTYPE_Y }
L_DOCTYPE_T, 'Y'        => { L_DOCTYPE_Y }

L_DOCTYPE_Y, **         => { L_ERROR }
L_DOCTYPE_Y, 'p'        => { L_DOCTYPE_P }
L_DOCTYPE_Y, 'P'        => { L_DOCTYPE_P }

L_DOCTYPE_P, **         => { L_ERROR }
L_DOCTYPE_P, 'e'        => { L_DOCTYPE_E }
L_DOCTYPE_P, 'E'        => { L_DOCTYPE_E }

L_DOCTYPE_E, **         => { L_ERROR }
L_DOCTYPE_E, *s         => { L_DOCTYPE_DECLARATION }

// The parsing of the DOCTYPE declaration should be handled externally
L_DOCTYPE_DECLARATION, ** => { L_DOCTYPE_DECLARATION }
L_DOCTYPE_DECLARATION, '>' => { L_ELEM_END }

//
// Section 6 - Comments
//

L_COMMENT_BEGIN, **             => { L_ERROR }
L_COMMENT_BEGIN, '-'            => { L_COMMENT }

L_COMMENT, **                   => { L_COMMENT }
L_COMMENT, '-'                  => { L_COMMENT_END_DASH1 }

L_COMMENT_END_DASH1, **         => { L_COMMENT }
L_COMMENT_END_DASH1, '-'        => { L_COMMENT_END_DASH2 }

L_COMMENT_END_DASH2, **         => { L_COMMENT }
L_COMMENT_END_DASH2, '>'        => { L_ELEM_END }
L_COMMENT_END_DASH2, '-'        => { L_COMMENT_END_DASH2 }

//
// Section 7 - CDATA
//

L_CDATA_LBRACKET1, **           => { L_ERROR }
L_CDATA_LBRACKET1, 'C'          => { L_CDATA_C }

L_CDATA_C, **                   => { L_ERROR }
L_CDATA_C, 'D'                  => { L_CDATA_D }
L_CDATA_D, **                   => { L_ERROR }
L_CDATA_D, 'A'                  => { L_CDATA_A }
L_CDATA_A, **                   => { L_ERROR }
L_CDATA_A, 'T'                  => { L_CDATA_T }
L_CDATA_T, **                   => { L_ERROR }
L_CDATA_T, 'A'                  => { L_CDATA_A2 }
L_CDATA_A2, **                  => { L_ERROR }
L_CDATA_A2, '['                 => { L_CDATA_LBRACKET2 }

L_CDATA_LBRACKET2, **           => { L_CDATA }
L_CDATA_LBRACKET2, ']'          => { L_CDATA_RBRACKET1 }

L_CDATA, **                     => { L_CDATA }
L_CDATA, ']'                    => { L_CDATA_RBRACKET1 }

L_CDATA_RBRACKET1, **           => { L_CDATA }
L_CDATA_RBRACKET1, ']'          => { L_CDATA_RBRACKET2 }

L_CDATA_RBRACKET2, **           => { L_CDATA }
L_CDATA_RBRACKET2, '>'          => { L_ELEM_END }

// 
// Section 8 - Entities
//

// Characters references in text nodes 
L_ENTITY, **                    => { L_ENTITY_END_DIRTY }
L_ENTITY, *aA                   => { L_ENTITY }
L_ENTITY, ';'                   => { L_ENTITY_END }
// We add this one to detect entities that are missing the trailing semi-colon
L_ENTITY, *s                    => { L_ENTITY_END_DIRTY }
L_ENTITY, '<'                   => { L_ENTITY_END_DIRTY }
L_ENTITY_END, **                => { L_ENTITY_END_DIRTY }

L_ENTITY_END_DIRTY, **          => { L_ENTITY_END_DIRTY }

//
// Section 9 - Inside <script> and <style> elements

// The fact that there can be different types of scripts in a HTML-page
// and the ambiguity of how the 'end' of a script is specified,
// makes this a hard problem to write an FSM for.
// The following rules are by no means comprehensive.

L_SCRIPT, **          => { L_SCRIPT }

// Define single-quoted and double-quoted strings and the beginning of the closing tag
L_SCRIPT, '\''  => { L_SCRIPT_SINGLE_QUOTE_STRING }
L_SCRIPT, '"'   => { L_SCRIPT_DOUBLE_QUOTE_STRING }
L_SCRIPT, '<'   => { L_SCRIPT_LT }
L_SCRIPT, '/'   => { L_SCRIPT_COMMENT_BEGIN }

// Single-quoted string with escaping by the \ character
L_SCRIPT_SINGLE_QUOTE_STRING, **        => { L_SCRIPT_SINGLE_QUOTE_STRING }
L_SCRIPT_SINGLE_QUOTE_STRING, '\\'      => { L_SCRIPT_SINGLE_QUOTE_STRING_ESCAPE }
L_SCRIPT_SINGLE_QUOTE_STRING, '\''      => { L_SCRIPT }

L_SCRIPT_SINGLE_QUOTE_STRING_ESCAPE, ** => { L_SCRIPT_SINGLE_QUOTE_STRING }

// Double-quoted string with escaping by the \ character
L_SCRIPT_DOUBLE_QUOTE_STRING, **        => { L_SCRIPT_DOUBLE_QUOTE_STRING }
L_SCRIPT_DOUBLE_QUOTE_STRING, '\\'      => { L_SCRIPT_DOUBLE_QUOTE_STRING_ESCAPE }
L_SCRIPT_DOUBLE_QUOTE_STRING, '"'      => { L_SCRIPT }

L_SCRIPT_DOUBLE_QUOTE_STRING_ESCAPE, ** => { L_SCRIPT_DOUBLE_QUOTE_STRING }

// Javascript comments
L_SCRIPT_COMMENT_BEGIN,       **        => { L_SCRIPT }
L_SCRIPT_COMMENT_BEGIN,       '/'       => { L_SCRIPT_SINGLE_COMMENT }
L_SCRIPT_COMMENT_BEGIN,       '*'       => { L_SCRIPT_MULTI_COMMENT }

L_SCRIPT_SINGLE_COMMENT,      **        => { L_SCRIPT_SINGLE_COMMENT }
L_SCRIPT_SINGLE_COMMENT,      '\n'      => { L_SCRIPT }
// The following is a questionable decision, however, it is encountered in wild html
L_SCRIPT_SINGLE_COMMENT,      '<'       => { L_SCRIPT_LT }

L_SCRIPT_MULTI_COMMENT,       **        => { L_SCRIPT_MULTI_COMMENT }
L_SCRIPT_MULTI_COMMENT,       '*'       => { L_SCRIPT_MULTI_COMMENT_END }

L_SCRIPT_MULTI_COMMENT_END,   **        => { L_SCRIPT_MULTI_COMMENT }
L_SCRIPT_MULTI_COMMENT_END,   '/'       => { L_SCRIPT }


// Closing sequence for </script>

L_SCRIPT_LT, **         => { L_SCRIPT }
L_SCRIPT_LT, *s         => { L_SCRIPT_LT }
L_SCRIPT_LT, '/'        => { L_SCRIPT_SOLIDUS }

L_SCRIPT_SOLIDUS, **    => { L_SCRIPT }
L_SCRIPT_SOLIDUS, *s    => { L_SCRIPT_SOLIDUS }
L_SCRIPT_SOLIDUS, 's'   => { L_SCRIPT_S }
L_SCRIPT_SOLIDUS, 'S'   => { L_SCRIPT_S }

L_SCRIPT_S, **          => { L_SCRIPT }
L_SCRIPT_S, 'c'         => { L_SCRIPT_C }
L_SCRIPT_S, 'C'         => { L_SCRIPT_C }

L_SCRIPT_C, **          => { L_SCRIPT }
L_SCRIPT_C, 'r'         => { L_SCRIPT_R }
L_SCRIPT_C, 'R'         => { L_SCRIPT_R }

L_SCRIPT_R, **          => { L_SCRIPT }
L_SCRIPT_R, 'i'         => { L_SCRIPT_I }
L_SCRIPT_R, 'I'         => { L_SCRIPT_I }

L_SCRIPT_I, **          => { L_SCRIPT }
L_SCRIPT_I, 'p'         => { L_SCRIPT_P }
L_SCRIPT_I, 'P'         => { L_SCRIPT_P }

L_SCRIPT_P, **          => { L_SCRIPT }
L_SCRIPT_P, 't'         => { L_SCRIPT_T }
L_SCRIPT_P, 'T'         => { L_SCRIPT_T }

L_SCRIPT_T, **          => { L_SCRIPT }
L_SCRIPT_T, *s          => { L_SCRIPT_T }
L_SCRIPT_T, '>'         => { L_ELEM_END }



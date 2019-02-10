/*
 * Haut - a lightweight html tokenizer
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, Leiden University
 *
 * This file is to be included is finite-state-machine generator scripts.
 * Therefore, only preprocessor directives and comments may be used.
 */

#ifndef STATE_H
#define STATE_H

/* HTML reserved characters */

#define HTML_EXCL       '!'
#define HTML_LT         '<'
#define HTML_GT         '>'
#define HTML_EQUALS     '='
#define HTML_SOLIDUS    '/'
#define HTML_AMP        '&'
#define HTML_SEMI       ';'
#define HTML_APOS       '\''
#define HTML_QUOTE      '"'
#define HTML_AGRAVE     '`'
#define HTML_LBRACKET   '['
#define HTML_RBRACKET   ']'

/* Lexer states */

#define L_BEGIN                         0
#define L_END                           1
#define L_WHITESPACE                    2
#define L_ERROR                         3
    
#define L_ELEM_BEGIN                    4
#define L_ELEM                          5
#define L_ELEM_END                      6
#define L_ELEM_WS                       7

#define L_CLOSE_ELEM_BEGIN              8
#define L_CLOSE_ELEM                    9
#define L_CLOSE_ELEM_SKIP               10
#define L_CLOSE_ELEM_END                11
#define L_CLOSE_ELEM_SELF               12

#define L_SPECIAL_ELEM                  13

#define L_ATTR_KEY                      14
#define L_ATTR_WS                       15
#define L_ATTR_EQUALS                   16
#define L_ATTR_SINGLE_QUOTE_OPEN        17
#define L_ATTR_SINGLE_QUOTE_VALUE       18
#define L_ATTR_DOUBLE_QUOTE_OPEN        19
#define L_ATTR_DOUBLE_QUOTE_VALUE       20
#define L_ATTR_VALUE                    21

#define L_INNERTEXT                     22

#define L_DOCTYPE_D                     23
#define L_DOCTYPE_O                     24
#define L_DOCTYPE_C                     25
#define L_DOCTYPE_T                     26
#define L_DOCTYPE_Y                     27
#define L_DOCTYPE_P                     28
#define L_DOCTYPE_E                     29
#define L_DOCTYPE_DECLARATION           30

#define L_COMMENT_BEGIN                 31
#define L_COMMENT                       32
#define L_COMMENT_END_DASH1             33
#define L_COMMENT_END_DASH2             34

#define L_CDATA_LBRACKET1               35
#define L_CDATA_C                       36
#define L_CDATA_D                       37
#define L_CDATA_A                       38
#define L_CDATA_T                       39
#define L_CDATA_A2                      40
#define L_CDATA_LBRACKET2               41
#define L_CDATA                         42
#define L_CDATA_RBRACKET1               43
#define L_CDATA_RBRACKET2               44

#define L_ENTITY                        45
#define L_ENTITY_END                    46
#define L_ENTITY_END_DIRTY              47

/* Lexer states inside <script> elements */

#define L_SCRIPT                                48

#define L_SCRIPT_SINGLE_QUOTE_STRING            49
#define L_SCRIPT_SINGLE_QUOTE_STRING_ESCAPE     50

#define L_SCRIPT_DOUBLE_QUOTE_STRING            51
#define L_SCRIPT_DOUBLE_QUOTE_STRING_ESCAPE     52

#define L_SCRIPT_COMMENT_BEGIN                  53
#define L_SCRIPT_SINGLE_COMMENT                 54
#define L_SCRIPT_MULTI_COMMENT                  55
#define L_SCRIPT_MULTI_COMMENT_END              56

#define L_SCRIPT_LT                             57
#define L_SCRIPT_SOLIDUS                        58

#define L_SCRIPT_S                              59
#define L_SCRIPT_C                              60
#define L_SCRIPT_R                              61
#define L_SCRIPT_I                              62
#define L_SCRIPT_P                              63
#define L_SCRIPT_T                              64

#define L_N_STATES                      65

/* Parser states */

#define P_NONE                  0       /* No action from the parser is required */
#define P_DOCUMENT_BEGIN        1       /* Document has begun */
#define P_DOCUMENT_END          2       /* Document has ended */

#define P_ELEMENT_OPEN          3       /* We're parsing an opening tag and its ID is now known */
#define P_ELEMENT_CLOSE         4       /* We're parsing a closing tag and its ID is now known */

#define P_ATTRIBUTE             5
#define P_ATTRIBUTE_VOID        6
#define P_ATTRIBUTE_KEY         7

#define P_INNERTEXT             8
#define P_TEXT                  9
#define P_COMMENT               10
#define P_CDATA                 11

#define P_DOCTYPE               12
#define P_ENTITY_BEGIN          13
#define P_INNERTEXT_ENTITY_BEGIN 14
#define P_ENTITY                15

#define P_ERROR                 16      /* Syntax error */

#define P_TOKEN_BEGIN           17      /* We begun parsing some token */
#define P_TOKEN_END             18      /* We have completed parsing some token */

#define P_ELEMENT_END           19      /* The definition of the current element is completed (including attributes) */
#define P_VOID_ELEMENT_END      20      /* The definition of the current void element is completed (including attributes) */

#define P_SCRIPT_END            21      /* We were lexing through a <script> and have reached its end */

// For future use
#define P_SAVE_LEXER_STATE      22      /* On a rare occasion we need to store the lexer's state */
#define P_RESTORE_LEXER_STATE   23
#define P_RESET_LEXER           24      /* Set the lexer state by looking at the current character (again) */
#define P_SAVE_TOKEN            25      /* Save the current token to a separate buffer - we do this already in streaming mode */

#endif

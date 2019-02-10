/*
 * fsm2array - convert simple finite state machine syntax to C array notation
 *
 * (C) 2017, Micky Faas <micky@edukitty.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <limits.h>
#include <ctype.h>
#include <assert.h>
#include <getopt.h> // for getopt()

// Options and flags
typedef enum {
    TOKEN_ERROR                 = 1,
    TOKEN_STRING                = 1 << 1,
    NEGATE                      = 1 << 2,
    WILDCARD_ALL                = 1 << 3,
    WILDCARD_LOWER_ALPHA        = 1 << 4,
    WILDCARD_UPPER_ALPHA        = 1 << 5,
    WILDCARD_DIGIT              = 1 << 6,
    WILDCARD_WHITESPACE         = 1 << 7,
    IS_WILDCARD                 = WILDCARD_ALL | WILDCARD_LOWER_ALPHA | WILDCARD_UPPER_ALPHA | WILDCARD_DIGIT | WILDCARD_WHITESPACE,
    CASE_INSENSITIVE            = 1 << 8
} token_opts_t;
    
typedef enum {
    ARRAY_INT,      // Type { 1, 2, 3 }
    ARRAY_CHAR      // Type "\x1\x2\x3"
} array_type_t;

typedef struct {
    // The action(s) of a transition can be writting as both int arrays and char arrays
    array_type_t array_type;
    // If the values of the input do not begin at zero, an offset can be substracted
    int input_base;
    // Number of action arrays written per line of output
    int line_width;
} options_t;

#define DEFAULT_OPTS { .array_type =ARRAY_CHAR, .input_base =0, .line_width =6 }
static const char* HELP_TEXT = "fsm2array - Generate C-style array from finite-state-machine transitions\n\
\n\
Options:\n\
\t-c\tSet output mode to character array (e.g. \"\\x1\\x2\\x3\") (Default)\n\
\t-i\tSet output mode to integer array (e.g. {1,2,3})\n\
\t-b <n>\tIf the values of the input do not begin at zero, an offset can be substracted. Default: 0\n\
\t-w <n>\tNumber of array elements per line of output\n\
\t-h\tThis help.\n\
Input is read from stdin and output is written to stdout, by default.\n";

int
parse_cmd_opts( options_t* opts, int argc, char* const* argv ) {
    int c;

    while ((c = getopt (argc, argv, "icb:w:h")) != -1)  {
        switch( c ) {
            case 'i':
                opts->array_type =ARRAY_INT;
                break;
            case 'c':
                opts->array_type =ARRAY_CHAR;
                break;
            case 'b':
                opts->input_base =atoi( optarg );
                break;
            case 'w':
                opts->line_width =atoi( optarg );
                break;
            case 'h':
                fprintf( stderr, "%s\n", HELP_TEXT );
                return -1;
            case '?':
                if( optopt == 'f' || optopt == 'w' )
                    fprintf( stderr, "ERROR: Option -%c required an argument. Try -h (help)\n", optopt );
                else
                    fprintf( stderr, "ERROR: Option -%c invalid. Try -h (help)\n", optopt );
                return -1;

            default:
                return -1;
        }
    }
    return 0;
}

#define LIST_APPEND( type, list, node ) { type* tmp =list; if( !list ) { list = node; } while( tmp != NULL ) { if( tmp->next == NULL ) { tmp->next = node; break; } tmp =tmp->next; } node->next =NULL; }

struct state_table;

struct state_table {
    struct state_table* next;
    int state;
    token_opts_t state_opts;
    int input;
    token_opts_t input_opts;
    char* input_string;
    int* action;
    size_t action_length;
};

typedef struct state_table state_table_t;

struct generated_state {
    struct generated_state* next;
    int state;
    char* substr;
    size_t substr_len;
};

typedef struct generated_state generated_state_t;

static void
print_array( int* A, size_t length, size_t max_length, options_t* opts ) {
    if( opts->array_type ==ARRAY_INT ) {
        if( max_length != 1 )
            printf( "{" );
    } else
        printf( "\"" );
    int i=0;
    for( ; i < max_length; i++ ) {
        int val;
        if( i < length )
            val =A[i];
        else
            val =0;
        if( opts->array_type ==ARRAY_INT ) {
            if( i == max_length-1 )
                printf( "0x%x", val );
            else
                printf( "0x%x,", val );
        }
        else
            printf( "\\x%x", val );

    }

    if( opts->array_type ==ARRAY_INT ) {
        if( max_length != 1 )
            printf( "}" );
    } else
        printf( "\"" );
}

static state_table_t*
find_transition( state_table_t* begin, int state, int input, options_t* opts ) {
    state_table_t *t =begin;

    state_table_t *match =NULL;
    state_table_t *input_wildcard =NULL;
    state_table_t *state_wildcard =NULL;
    state_table_t *catch_all =NULL;

    while( t ) {
        int state_matches = (t->state == state);
        if( t->state_opts & NEGATE ) state_matches =! state_matches;

        int input_matches; 
        if( t->input_opts & CASE_INSENSITIVE )
            input_matches= ((tolower(t->input) - opts->input_base) == tolower(input+opts->input_base) - opts->input_base );
        else
            input_matches= ((t->input - opts->input_base) == input);
        if( t->input_opts & NEGATE ) input_matches =! input_matches;
        
        int state_matches_wildcard = ( t->state_opts & WILDCARD_ALL && !(t->state_opts & NEGATE) ); 
        
        int input_matches_wildcard = 
                ( t->input_opts & WILDCARD_LOWER_ALPHA && input >= 'a' && input <= 'z' ) ||
                ( t->input_opts & WILDCARD_UPPER_ALPHA && input >= 'A' && input <= 'Z' ) ||
                ( t->input_opts & WILDCARD_DIGIT && input >= '0' && input <= '9' ) ||
                ( t->input_opts & WILDCARD_WHITESPACE && isspace( input ) ) ||
                ( t->input_opts == WILDCARD_ALL );
        if( t->input_opts & IS_WILDCARD && t->input_opts & NEGATE ) input_matches_wildcard =! input_matches_wildcard;

        if( state_matches && input_matches ) {
            match =t;
            // Exact match gets highest precedence
            if( !(t->state_opts & NEGATE) && !(t->input_opts & NEGATE ) )
                break;
        } else if( state_matches_wildcard && input_matches ) {
            state_wildcard =t;
        } else if( state_matches && input_matches_wildcard ) {
            // More specific wildcards take precedence
            if( !input_wildcard || !(t->input_opts & WILDCARD_ALL) )
                input_wildcard =t;
        } else if( state_matches_wildcard && input_matches_wildcard ) {
            // More specific wildcards take precedence
            if( !catch_all || !(t->input_opts & WILDCARD_ALL) )
                catch_all =t;
        }

        t =t->next;
    }
    // Precendence of rules:
    // - Exact match            (highest)
    // - State wildcard <**, i>
    // - Input wildcard
    // - Input catch all <s, **>
    // - Catch all              (lowest)
    
    if( !match ) match = state_wildcard;
    if( !match ) match = input_wildcard;
    if( !match ) match = catch_all;
    

    return match;
}

static token_opts_t
tokenize( const char* input, int* numeric, char** string ) {
//    fprintf( stderr, "Token: %s\n", input );
    token_opts_t opts =0;
    int o =0;
    // Some expression allow ^ as negation operator
    if( input[0] == '^' ) {
        opts = NEGATE;
        o =1; // offset
    }
    // Numeric expression
    if( isdigit( input[o] ) ) {
        *numeric = atoi( input+o );
        return opts;
    // Wildcard expression
    } else if( strlen( input ) > 1 && (input[0] == '*' || input[0] == '^') ) {
        for( int i =1; i < strlen( input ); i++ ) {
            switch( input[i] ) {
                case 'a':
                    opts |= WILDCARD_LOWER_ALPHA;
                    break;
                case 'A':
                    opts |= WILDCARD_UPPER_ALPHA;
                    break;
                case 'd':
                    opts |= WILDCARD_DIGIT;
                    break;
                case 's':
                    opts |= WILDCARD_WHITESPACE;
                    break;
                case '*':
                    opts = WILDCARD_ALL;
                    goto r;
                default:
                    opts = TOKEN_ERROR;
                    goto r;
            }
        }
r:
        return opts;
    // Single character expression
    } else if ( strlen(input+o) == 3 && input[o] == '\'' && input[o+2] == '\'' ) {
        *numeric = input[o+1];
        return opts;
    }
    // Single escaped character expression
    else if ( strlen(input+0) == 4 && input[o+0] == '\'' && input[o+1] == '\\' && input[o+3] == '\'' ) {
        switch( input[o+2] ) {
            case 'n':
                *numeric = '\n';
                break;
            case 'r':
                *numeric = '\r';
                break;
            case 't':
                *numeric = '\t';
                break;
            default:
                *numeric = input[2];
        }
        return opts;
    }
    // String expression
    else if( input[0] == '"' ) {
        char *str =malloc( 1 );
        size_t str_size =0;
        str [0] =0;
        int escape =0, done =0, i =1;

        for( ; i < strlen( input ); i++ ){
            if( input[i] == '\\' && !escape ) {
                escape =1;
                continue;
            }
            else if( input[i] == '"' && !escape && !done ) {
                done =1;
            }
            else if( input[i] == 'i' && done ) {
                opts |= CASE_INSENSITIVE;
            }

            if( !done ) {
                str[str_size] =input[i];
                str =realloc( str, ++str_size+1 );
                str[str_size] =0;
            }
            escape =0;
        }

        if( str_size ) {
            *string =str;
            *numeric =0;
            opts |= TOKEN_STRING;
            return opts;
        }
    }
   
    // TODO: add lookup of defined labels here, so we could do without the external preprocessing
    return TOKEN_ERROR;
}

generated_state_t*
find_state( generated_state_t* list, const char* substr, size_t substr_len ) {
    generated_state_t* s =list;
    while( s ) {
        if( substr_len == s->substr_len &&
                strncmp( substr, s->substr, substr_len ) == 0 )
            return s;
        s = s->next;
    }

    return NULL;
}

void
expand_input_strings( state_table_t* transition_list, state_table_t* string_transition_list, int* numstates ) {
    
    generated_state_t* gen_states =NULL;
    int substr_len =1;

    while( 1 ) {
        int new_ts =0;

        state_table_t* t =string_transition_list;
        while( t ) {
            if( strlen( t->input_string ) < substr_len ) goto cont;

            // Check if a state with this substring exists, create it otherwise
            // Also, if we've seen the entire string, don't generate another state

            generated_state_t* s =NULL;
            
            if( strlen( t->input_string ) != substr_len ) {
                s =find_state( gen_states, t->input_string, substr_len );
                if( !s ) {
                    s =malloc( sizeof( generated_state_t ) );
                    s->state =(*numstates)++;
                    s->substr =t->input_string;
                    s->substr_len =substr_len;
                    LIST_APPEND( generated_state_t, gen_states, s );
                }
            }

            // Add the transition for this substring and state
            state_table_t* trans =malloc( sizeof( state_table_t ) );
            memset( trans, 0, sizeof( state_table_t ) );

            // Take the current input character from the substring
            trans->input =t->input_string[substr_len-1];
            trans->input_opts =t->input_opts & ~TOKEN_STRING; // Transfer any options, but not the STRING attribute

            if( substr_len == 1 ) {  // Initial transition from the user state
                 trans->state      =t->state;
                 trans->state_opts =t->state_opts;
            } else {
                // Otherwise find the previous state, again
                generated_state_t* s_prev =find_state( gen_states, t->input_string, substr_len -1);
                assert( s_prev );
                trans->state       =s_prev->state;
                trans->state_opts  =0; // For now
            }

            // Set the actions, depending on whether this is the final state or not
            if( s ) {
                trans->action        =malloc( sizeof(int) );
                trans->action_length =1;
                trans->action[0]     =s->state;
            } else {
                trans->action        =malloc( sizeof(int) * t->action_length );
                trans->action_length =t->action_length;
                memcpy( trans->action, t->action, sizeof(int) * t->action_length ); 
            }

            //fprintf( stderr, "Adding: %d, %c => { %d }\n", trans->state, (char)trans->input, trans->action[0] );
            LIST_APPEND( state_table_t, transition_list, trans );
            new_ts++;
cont:
            t =t->next;
        }

        if( !new_ts )
            break;

        substr_len++;
    }

    // cleanup
    generated_state_t* s =gen_states;
    while(s) { generated_state_t* tmp =s->next; free(s); s =tmp; }
}

int
main( int argc, char * const*argv ) {
    // Check the commandline arguments
    options_t opts = DEFAULT_OPTS;
    if( parse_cmd_opts( &opts, argc, argv ) != 0 )
        return -1;

    // Read the number of states and number of inputs from the first line
    // Expected syntax: `<states> <inputs>'
    int numstates =0, numinputs =0;
    size_t max_actions =0; // The largest number of actions encoutered

    /*char buf[2];
    fread( buf, 1, 2, stdin );
    if( buf[0] == '/' && ( buf[1] == '/' || buf[1] == '&' ) ) {
        scanf( "%d%d", &numstates, &numinputs );
    }*/

    scanf( "%%! %d%d", &numstates, &numinputs );
    if( !numstates || !numinputs ) {
        fprintf( stderr, "ERROR: Specify number of states and number of input values on the first line\n" );
        return -1;
    }


    // Read each state transition from the standard input
    // Expected syntax: `<state>, <input> => { <new state>, <option 1>, ... <option n> }'
    
    state_table_t* transition_list =NULL;
    state_table_t* string_transition_list =NULL;

    while( !feof( stdin ) ) {
        // Create an empty transition
        state_table_t* n =(state_table_t*)malloc( sizeof( state_table_t ) );
        memset( n, 0, sizeof( state_table_t ) );

        // Split the input line into the part before the => and the part after
        int ret;
        char* state, *input;
        if( ( (ret = scanf( " %m[0-9*^] , %ms => { ", &state, &input ) ) != 2 )
                || !state
                || !input ) {
            if( ret == EOF ) break;
            fprintf( stderr, "ERROR: Syntax error\n" );
            return -1;
        }

        // After the =>, we read the list of actions
        int *action =malloc( sizeof(int) * 1 );
        action[0] =0;
        size_t action_length =0;

        while( !feof( stdin ) ) {
            int value =0;
            if( scanf( "%d , ", &value ) != 1 ) {
                scanf( "}" );
                break;
            }

            if( value > 255 && opts.array_type == ARRAY_CHAR ) {
                fprintf( stderr, "ERROR: Specified action `%d' does not fit specified type (char) try `-i'\n", value );
                return -1;
            }
            
            action =realloc( action, (++action_length+1) * sizeof(int) );
            action[action_length-1] =value;
            action[action_length] =0;
        }

        n->action =action;
        n->action_length =action_length;
        max_actions =(action_length>max_actions) ? action_length : max_actions;

        // Tokenize the state and input buffers
        //
        char* dummy =NULL;
        n->state_opts =tokenize ( state, &n->state, &dummy);
        n->input_opts =tokenize( input, &n->input, &n->input_string );

        if( !(n->state_opts & IS_WILDCARD) && n->state >= numstates ) { 
            fprintf( stderr, "ERROR: State out-of-range in `%s %s => ...'\n", state, input );
            return -1;
        }
        if( !(n->input_opts & (IS_WILDCARD | TOKEN_STRING) ) &&
                ( ((n->input - opts.input_base) >= numinputs )
                  || ((n->input - opts.input_base) < 0 ) ) ) {
            fprintf( stderr, "ERROR: Input out-of-range in `%s %s => ...' (check the -b flag)\n", state, input );
            return -1;
        }
        if( n->state_opts == TOKEN_ERROR || n->input_opts == TOKEN_ERROR ) {
            fprintf( stderr, "ERROR: Unrecognized token in `%s %s => ...'\n", state, input );
            return -1;
        }
        if( dummy ) {
            fprintf( stderr, "ERROR: String not allowed as state literal\n" );
            return -1;
        }

/*        fprintf( stderr, "Parsing (%s, %s) into [%c%d, %c%d]\n",
                state, input,
                ((n->state_opts & NEGATE) ? '~' : ' '), n->state,
                ((n->input_opts & NEGATE) ? '~' : ' '), n->input );*/
        
        free( state );
        free( input );

        
        // Insert the transition into the correct list
        if( n->input_opts & TOKEN_STRING ) {
            LIST_APPEND( state_table_t, string_transition_list, n );
        } else {
            LIST_APPEND( state_table_t, transition_list, n );
        }

    }

    // Transitions with input specified as string must be expanded first
    expand_input_strings( transition_list, string_transition_list, &numstates );

    printf( "// states = %d, inputs = %d\n", numstates, numinputs );
    printf( "// Resulting array is indexed by [state][input+(%d)]\n", -opts.input_base );
    printf( "// This file was automatically generated by fsm2array\n// Please do not edit this file directly\n// Rules to generate this file can be found in the util/ directory\n" );

    // Cycle through all state-input combinations and generate C-array syntax
    for( int i =0; i < numstates; i++ ) {
        printf( "// Transitions for state %d\n", i );
        printf( "{" );
        for( int j =0; j < numinputs; j++ ) {
            state_table_t *t = find_transition( transition_list, i, j, &opts );
            if( !t ) {
                fprintf( stderr, "ERROR: No transition defined for (%d, %d)\n", i,j );
                return -1;
            }
            print_array( t->action, t->action_length, max_actions, &opts );
            if( j != numinputs-1 )
                printf( "," );
            if( !((j+1) % opts.line_width ) )
                printf( "\n" );
        }
        if( i != numstates-1 )
            printf( "},\n" );
        else
            printf( "}\n" );
    }

    // Cleanup
    state_table_t* n = transition_list;
    while( n ) {
        state_table_t *tmp =n;
        n = n->next;
        free( tmp->action );
        free( tmp );
    }


    return 0;
}

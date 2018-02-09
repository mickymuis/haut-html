// This is an example file for fsm2array
%! 2 34
// The above defines two different states (0 and 1) and 34 different inputs (0...33)

// To improve readability, we use the C-preprocessor to substitute numbers for named states
// Run this file through `gcc -E -P' before handing it to fsm2array
#define STATE1 0
#define STATE2 1

// A transition is specified as <state>, <input> => { actions... }
STATE1, 31 => { 1, 1 }
// Actions can be any number of comma-separated byte-values
// The result will be a charater string composed of all action-values
STATE1, 32 => { 1, 2 }
// We can match `all other inputs' by a double asterisk
// Other wildcards are also available: 
//      *a matches all lower case letters
//      *A matches all upper case letters
//      *d matches all digits
//      *s matches all whitespace
//      They can be combined, e.g. *aA
STATE1, ** => { 0, 0 }
// The transition with the largest amount of actions 
// defines the length of each string in the output array
STATE2, ** => { 0, 0, 1 }
// We can also use an ASCII character as input ('!' has value 33)
STATE2, '!' => { 1, 4 }

/*
 * Haut - a lightweight html tokenizer
 *
 * https://github.com/mickymuis/haut-html
 *
 * Micky Faas <micky@edukitty.org>
 * Copyright 2017-2018
 * Leiden Institute of Advanced Computer Science, The Netherlands
 */

#ifndef TEST_H
#define TEST_H

#include <stdbool.h>
#include <setjmp.h>
#include <haut/string_util.h>

typedef struct {
    bool stop_on_error;
    bool generate;
    bool stream;
} flags_t;

typedef struct {
    /* These variables are expected to be initialized by the caller */
    char* input_buf;
    size_t input_size, input_ptr;
    char* expect_buf;
    size_t expect_size, expect_ptr;

    flags_t flags;

    /* These variables are used internally by the test functions */
    jmp_buf return_on_mismatch;
    strbuffer_t output_buf;

} test_t;

/* Starts parsing the input_buf from @t and tests 
 * the parser's output against expect_buf.
 * Returns true if all expectations are met.*/
bool
beginTest( test_t* t );

#endif

/*
 * Haut - a lightweight html tokenizer
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, Leiden University
 */

#include "../include/haut/string_util.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#if defined(_WIN32) || defined(_WIN64) 
#else
#include <strings.h>
#endif

#define MIN(a,b) (((a)<(b))?(a):(b))

bool 
strfragment_cmp(strfragment_t* str1, const char* str2) {
	return ( strncmp( str1->data, str2, str1->size ) == 0 );
}

bool 
strfragment_ncmp(strfragment_t* str1, const char* str2, size_t len) {
	return ( strncmp( str1->data, str2, MIN( len, str1->size ) ) == 0);
}

bool 
strfragment_icmp(strfragment_t* str1, const char* str2) {
#if defined(_WIN32) || defined(_WIN64) 
	return ( _strnicmp( str1->data, str2, str1->size ) == 0 );
#else
	return ( strncasecmp( str1->data, str2, str1->size ) == 0 );
#endif
}

bool 
strfragment_nicmp(strfragment_t* str1, const char* str2, size_t len) {
#if defined(_WIN32) || defined(_WIN64) 
	return (_strnicmp(str1->data, str2, MIN(len, str1->size)) == 0);
#else
	return (strncasecmp(str1->data, str2, MIN( len, str1->size )) == 0);
#endif
}


#define BLOCK_SIZE 64

void
strbuffer_init( strbuffer_t* d ) {
    d->data =malloc( BLOCK_SIZE );
    d->data[0] =0;
    d->size =0;
    d->capacity =BLOCK_SIZE;
}

void
strbuffer_free( strbuffer_t* d ) {
    free( d->data );
    d->size =0;
}

void 
strbuffer_clear( strbuffer_t* d ) {
    if( d->capacity > BLOCK_SIZE ) {
        d->data =realloc( d->data, BLOCK_SIZE );
        assert( d->data != NULL );
        d->capacity =BLOCK_SIZE;
    }
    d->size =0;
    d->data[0] =0;
}

size_t
strbuffer_grow( strbuffer_t* d, size_t add ) {
    return strbuffer_reserve( d, d->size + add );
}

size_t
strbuffer_reserve( strbuffer_t* d, size_t total ) {
    if( total + 1 > d->capacity ) {
        size_t newcap =((total + 1) / BLOCK_SIZE + 1) * BLOCK_SIZE;
        d->data =realloc( d->data, newcap );
        assert( d->data != NULL );
        d->data[d->size] =0;
        d->capacity =newcap;
    }
    return d->capacity;
}

void 
strbuffer_copy( strbuffer_t* d, const strbuffer_t* src ) {
    strbuffer_ncopy( d, 0, src, 0, src->size );
}

void 
strbuffer_ncopy( strbuffer_t* d, size_t dest_offset, const strbuffer_t* src, size_t src_offset, size_t length ) {
    if( src->capacity < src_offset + length )
        return;
    if( d->size <= dest_offset + length + 1 )
        strbuffer_grow( d, (dest_offset + length + 1) - d->size );
    memcpy( d->data+dest_offset, src->data+src_offset, length );
}

void 
strbuffer_copyFragment( strbuffer_t* d, size_t offset, strfragment_t* str ) {
    if( d->size <= offset + str->size + 1 ) {
        strbuffer_grow( d, (offset + str->size + 1) - d->size );
        d->size = offset + str->size;
    }
    memcpy( d->data+offset, str->data, str->size );
}

void 
strbuffer_append( strbuffer_t* d, const char* str, size_t len ) {

    strbuffer_grow( d, len );
    strncpy( d->data + d->size, str, len );
    d->size +=len;
    d->data[d->size] =0;
}

void 
strbuffer_swap( strbuffer_t* str1, strbuffer_t* str2 ) {
    strbuffer_t tmp =*str1;
    *str1 =*str2;
    *str2 =tmp;
}

strfragment_t 
strbuffer_to_fragment( strbuffer_t str ) {
    strfragment_t frag;
    frag.data =str.data;
    frag.size =str.size;
    return frag;
}

bool
u32toUTF8( strbuffer_t* d, char32_t c ) {
    // Unicode codepoints can either expand to 1, 2, 3 or 4 UTF-8 values
    int length =0;
    if( c < 0x80 ) {
        length =1;
    } else if( c < 0x800 ) { 
        length =2;
    } else if( c < 0x10000 ) {
        length =3;
    } else if( c <= 0x10FFFF ) {
        length =4;
    } else
        return false; // Not Unicode

    // Allocate the correct number of bytes
    d->data = (char*)malloc( sizeof(char) * length+1 );
    d->size =length;
    d->capacity =length+1;

    return u32toUTF8_at( d, 0, c );
}

bool
u32toUTF8_at( strbuffer_t* d, int offset, char32_t c ) {
    char* ptr = d->data;

    // Multi-byte sequences a made up of six-bit groups in the codepoint,
    // OR'ed with 0x80 they make a byte.
    // The remaining bits are to be put into the first byte
    if( c < 0x80 ) {
        ptr[0] = (unsigned char)c;
        ptr[1] = 0;
    } else if ( c < 0x800 ) {
        // 0xC0 denotes a two-byte sequence
        ptr[0] = 0xC0 | ( c >> 6 );
        // Second byte (group of six bits)
        ptr[1] = 0x80 | ( c & 0x3F );
        ptr[2] = 0;
    } else if( c < 0x10000 ) {
        // 0xE0 denotes a three-byte sequence
        ptr[0] = 0xE0 | ( c >> 12 );
        ptr[1] = 0x80 | ( ( c >> 6 ) & 0x3F );
        ptr[2] = 0x80 | ( c & 0x3F );
        ptr[3] = 0;
    } else if( c <= 0x10FFFF ) {
        // 0xF0 denotes a four-byte sequence
        ptr[0] = 0xF0 | ( c >> 18 );
        ptr[1] = 0x80 | ( ( c >> 12 ) & 0x3F );
        ptr[2] = 0x80 | ( ( c >> 6 ) & 0x3F );
        ptr[3] = 0x80 | ( c & 0x3F );
        ptr[4] = 0;
    } else
        return false;
    return true;
}

/*
base64encode.c - modified by David Lazar

Originally:
cencoder.c - c source to a base64 encoding algorithm implementation

This is part of the libb64 project, and has been placed in the public domain.
For details, see http://sourceforge.net/projects/libb64
*/

#include <stddef.h>
#include <stdint.h>
#include "base64encode.h"

void base64_encode_init(base64_encodestate *S) {
    S->step = step_A;
    S->result = 0;
}

char base64_encode_value(uint8_t value) {
    static const char* encoding = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    if (value > 63) return '=';
    return encoding[value];
}

ptrdiff_t base64_encode_update(base64_encodestate *S, const uint8_t *data, uint64_t datalen, char *encoded) {
    char *encoded_begin = encoded;
    const uint8_t *currbyte = data;
    const uint8_t *data_end = data + datalen;
    uint8_t result;
    uint8_t fragment;

    result = S->result;

    switch (S->step) {
        while (1) {
    case step_A:
            if (currbyte == data_end) {
                S->result = result;
                S->step = step_A;
                return encoded - encoded_begin;
            }
            fragment = *currbyte++;
            result = (fragment & 0x0fc) >> 2;
            *encoded++ = base64_encode_value(result);
            result = (fragment & 0x003) << 4;
    case step_B:
            if (currbyte == data_end) {
                S->result = result;
                S->step = step_B;
                return encoded - encoded_begin;
            }
            fragment = *currbyte++;
            result |= (fragment & 0x0f0) >> 4;
            *encoded++ = base64_encode_value(result);
            result = (fragment & 0x00f) << 2;
    case step_C:
            if (currbyte == data_end) {
                S->result = result;
                S->step = step_C;
                return encoded - encoded_begin;
            }
            fragment = *currbyte++;
            result |= (fragment & 0x0c0) >> 6;
            *encoded++ = base64_encode_value(result);
            result = (fragment & 0x03f) >> 0;
            *encoded++ = base64_encode_value(result);
        }
    }

    // control flow shouldn't reach here
    return encoded - encoded_begin;
}

ptrdiff_t base64_encode_final(base64_encodestate *S, char *encoded) {
    char *encoded_begin = encoded;

    switch (S->step) {
    case step_B:
        *encoded++ = base64_encode_value(S->result);
        *encoded++ = '=';
        *encoded++ = '=';
        break;
    case step_C:
        *encoded++ = base64_encode_value(S->result);
        *encoded++ = '=';
        break;
    case step_A:
        break;
    }

    return encoded - encoded_begin;
}

ptrdiff_t base64_encode(const uint8_t *data, uint64_t datalen, char *encoded) {
    ptrdiff_t c = 0;

    base64_encodestate S;
    base64_encode_init(&S);
    c += base64_encode_update(&S, data, datalen, encoded);
    c += base64_encode_final(&S, encoded + c);

    return c;
}

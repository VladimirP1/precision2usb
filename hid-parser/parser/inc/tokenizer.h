#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t const* data;
    size_t length;
    size_t curPos;
} tokenizer_state;

enum {
    TYPE_MAIN = 0,
    TYPE_GLOBAL,
    TYPE_LOCAL,
    TYPE_RESERVED,
    TYPE_INVALID,
};

enum {
    TAG_MAIN_INPUT = 0x8,
    TAG_MAIN_OUTPUT = 0x9,
    TAG_MAIN_FEATURE = 0xb,
    TAG_MAIN_COLLECTION = 0xa,
    TAG_MAIN_END_COLLECTION = 0xc,
};

enum {
    TAG_GLOBAL_USAGE_PAGE = 0x0,
    TAG_GLOBAL_LOGICAL_MINIMUM = 0x1,
    TAG_GLOBAL_LOGICAL_MAXIMUM = 0x2,
    TAG_GLOBAL_PHYSICAL_MINIMUM = 0x3,
    TAG_GLOBAL_PHYSICAL_MAXIMUM = 0x4,
    TAG_GLOBAL_UNIT_EXPONENT = 0x5,
    TAG_GLOBAL_UNIT = 0x6,
    TAG_GLOBAL_REPORT_SIZE = 0x7,
    TAG_GLOBAL_REPORT_ID = 0x8,
    TAG_GLOBAL_REPORT_COUNT = 0x9,
    TAG_GLOBAL_PUSH = 0xa,
    TAG_GLOBAL_POP = 0xb,
};

enum {
    TAG_LOCAL_USAGE = 0x0,
    TAG_LOCAL_USAGE_MINIMUM = 0x1,
    TAG_LOCAL_USAGE_MAXIMUM = 0x2,
    TAG_LOCAL_DESIGNATOR_INDEX = 0x3,
    TAG_LOCAL_DESIGNATOR_MINIMUM = 0x4,
    TAG_LOCAL_DESIGNATOR_MAXIMUM = 0x5,
    TAG_LOCAL_STRING_INDEX = 0x7,
    TAG_LOCAL_STRING_MINIMUM = 0x8,
    TAG_LOCAL_STRING_MAXIMUM = 0x9,
    TAG_LOCAL_DELIMITER = 0xa,
};

typedef struct {
    uint8_t size;
    uint8_t type;
    uint8_t tag;
    uint8_t const* data;
} token;

int token_init(tokenizer_state* state, uint8_t const* data, size_t length);
int token_next(tokenizer_state *S, token* tok);
void token_print(token* tok);

#endif // TOKENIZER_H

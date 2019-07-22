#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "tokenizer.h"
#include "ast.h"

typedef struct parser_state_global parser_state_global;

enum {
    COLLECTION_PHYSICAL = 0,
    COLLECTION_APPLICATION,
    COLLECTION_LOGICAL,
    COLLECTION_REPORT,
    COLLECTION_NAMEDARRAY,
    COLLECTION_USAGESWITCH,
    COLLECTION_USAGEMODIFIER,
    COLLECTION_ROOT = 255
};


struct parser_state_global {
    uint8_t reportId;
    int16_t reportSize;
    int16_t reportCount;
    uint32_t usagePage;
    parser_state_global* next;
};

typedef struct {
    parser_state_global* globalState;

    // State from local items
    int16_t usageCount;
    uint32_t usage[16];
    uint32_t usageMinimum;
    uint32_t usageMaximum;
    uint16_t curBit;

    ast_node* rootNode;
    ast_node* curNode;
} parser_state;

void parser_init(parser_state* S);
void parser_deinit(parser_state* S);
int parser_token(parser_state* S, token* tok);

#endif

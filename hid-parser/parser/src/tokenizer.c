#include "tokenizer.h"
#include <assert.h>
#include <stdio.h>

static const token error_token = {
    .size = 0,
    .type = TYPE_INVALID,
    .tag = 0,
    .data = 0
};

int token_init(tokenizer_state *state, uint8_t const* data, size_t length)
{
    state->data = data;
    state->length = length;
    state->curPos = 0;

    return 0;
}

static void token_advance(tokenizer_state* S, size_t ofs) {
    S->curPos += ofs;
    assert (S->curPos <= S->length);
}

static int token_next_long(tokenizer_state *S, token* tok) {
    if (S->curPos + 2 >= S->length) {
        *tok = error_token;
        return 1;
    }

    tok->size = S->data[S->curPos + 1];
    tok->tag = S->data[S->curPos + 2];
    tok->type = TYPE_RESERVED;
    tok->data = &S->data[S->curPos + 3];

    token_advance(S, tok->size + 3);
    return 0;
}

int token_next(tokenizer_state *S, token* tok) {
    if (S->curPos >= S->length) {
        *tok = error_token;
        return 1;
    }

    if (S->data[S->curPos] == 0xfe) {
        return token_next_long(S, tok);
    }

    tok->size = S->data[S->curPos] & 0x03;
    tok->size = (uint8_t) (1 << tok->size) >> 1;
    tok->type = (S->data[S->curPos] >> 2) & 0x03;
    tok->tag = (S->data[S->curPos] >> 4) & 0x0f;
    tok->data = &S->data[S->curPos + 1];

    token_advance(S, tok->size + 1);
    return 0;
}

void token_print(token* tok) {
    char* desc = "???";

    if (tok->type == TYPE_MAIN) {
        switch (tok->tag) {
            case TAG_MAIN_INPUT:
                desc = "Main.INPUT";
                break;
            case TAG_MAIN_OUTPUT:
                desc = "Main.OUTPUT";
                break;
            case TAG_MAIN_FEATURE:
                desc = "Main.FEATURE";
                break;
            case TAG_MAIN_COLLECTION:
                desc = "Main.COLLECTION";
                break;
            case TAG_MAIN_END_COLLECTION:
                desc = "Main.END_COLLECTION";
                break;
        }
    } else if (tok->type == TYPE_GLOBAL) {
        switch (tok->tag) {
            case TAG_GLOBAL_USAGE_PAGE:
                desc = "Global.USAGE_PAGE";
                break;
            case TAG_GLOBAL_LOGICAL_MINIMUM:
                desc = "Global.LOGICAL_MINIMUM";
                break;
            case TAG_GLOBAL_LOGICAL_MAXIMUM:
                desc = "Global.LOGICAL_MAXIMUM";
                break;
            case TAG_GLOBAL_PHYSICAL_MINIMUM:
                desc = "Global.PHYSICAL_MINIMUM";
                break;
            case TAG_GLOBAL_PHYSICAL_MAXIMUM:
                desc = "Global.PHYSICAL_MAXIMUM";
                break;
            case TAG_GLOBAL_UNIT_EXPONENT:
                desc = "Global.UNIT_EXPONENT";
                break;
            case TAG_GLOBAL_UNIT:
                desc = "Global.UNIT";
                break;
            case TAG_GLOBAL_REPORT_SIZE:
                desc = "Global.REPORT_SIZE";
                break;
            case TAG_GLOBAL_REPORT_ID:
                desc = "Global.REPORT_ID";
                break;
            case TAG_GLOBAL_REPORT_COUNT:
                desc = "Global.REPORT_COUNR";
                break;
            case TAG_GLOBAL_PUSH:
                desc = "Global.PUSH";
                break;
            case TAG_GLOBAL_POP:
                desc = "Global.POP";
                break;
        }

    } else if (tok->type == TYPE_LOCAL) {
        switch (tok->tag) {
            case TAG_LOCAL_USAGE:
                desc = "Local.USAGE";
                break;
            case TAG_LOCAL_USAGE_MINIMUM:
                desc = "Local.USAGE_MAXIMUM";
                break;
            case TAG_LOCAL_USAGE_MAXIMUM:
                desc = "Local.USAGE_MINIMUM";
                break;
            case TAG_LOCAL_DESIGNATOR_INDEX:
                desc = "Local.DESIGNATOR_INDEX";
                break;
            case TAG_LOCAL_DESIGNATOR_MINIMUM:
                desc = "Local.DESIGNATOR_MINIMUM";
                break;
            case TAG_LOCAL_DESIGNATOR_MAXIMUM:
                desc = "Local.DESIGNATOR_MAXIMUM";
                break;
            case TAG_LOCAL_STRING_INDEX:
                desc = "Local.STRING_INDEX";
                break;
            case TAG_LOCAL_STRING_MINIMUM:
                desc = "Local.STRING_MINIMUM";
                break;
            case TAG_LOCAL_STRING_MAXIMUM:
                desc = "Local.STRING_MAXIMUM";
                break;
            case TAG_LOCAL_DELIMITER:
                desc = "Local.DELIMITER";
                break;
        }
    }

    printf("%s\r\n", desc);
}

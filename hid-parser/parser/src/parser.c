#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "util.h"
#include <FreeRTOS.h>

void parser_init(parser_state* S) {
    *S = (parser_state) {
        .usageCount = 0,
        .usageMinimum = 0,
        .usageMaximum = 0,
        .curBit = 0,
        .rootNode = ast_alloc(),
    };

    S->curNode = S->rootNode;

    S->globalState = pvPortMalloc(sizeof (parser_state_global));

    *S->globalState = (parser_state_global) {
        .reportId = 0,
        .reportSize = 0,
        .reportCount = 0,
        .usagePage = 0,
        .next = NULL,
    };

}

void parser_deinit(parser_state* S) {
    ast_free(S->rootNode);
    while(S->globalState) {
        void* cur = S->globalState;
        S->globalState = S->globalState->next;
        vPortFree(cur);
    }
}

static void global_state_push(parser_state* S) {
    parser_state_global *newstate = pvPortMalloc(sizeof(parser_state_global));
    newstate->next = S->globalState;
    S->globalState = newstate;
}

static void global_state_pop(parser_state* S) {
    parser_state_global* oldstate = S->globalState;
    S->globalState = S->globalState->next;
    vPortFree(oldstate);
}

static void set_usages(parser_state* S) {
    parser_state_global* G = S->globalState;
    if (S->usageCount < 0) {
        assert((uint16_t) (S->usageMaximum - S->usageMinimum + 1) == G->reportCount);
        for (uint32_t i = 0; i < (uint16_t) G->reportCount; ++i) {
            S->usage[i] = S->usageMinimum + i;
        }
        S->usageCount = G->reportCount;
    }
}

static void add_rpts(parser_state* S, enum ast_report_type t) {
    parser_state_global* G = S->globalState;
    set_usages(S);
    assert(S->usageCount == 0 || S->usageCount == 1 || S->usageCount == G->reportCount);
    if (S->usageCount == 0) {
        for (int i = 0; i < G->reportCount; ++i) {
            ast_node* child = ast_add_child(S->curNode, NODE_REPORT_FIELD);
            ast_report* rpt = &child->data.report;
            rpt->id = G->reportId;
            rpt->start = S->curBit;
            S->curBit += (uint16_t) G->reportSize;
            rpt->size = (uint16_t) G->reportSize;
            rpt->usage = USAGE_NONE; //TODO: Fix?
            rpt->count = 1;
            rpt->type = t;
            dbg_printf("    Padding\r\n");
        }
    } else {
        for (int i = 0; i < S->usageCount; ++i) {
            ast_node* child = ast_add_child(S->curNode, NODE_REPORT_FIELD);
            ast_report* rpt = &child->data.report;
            rpt->id = G->reportId;
            rpt->start = S->curBit;
            S->curBit += (uint16_t) G->reportSize;
            rpt->size = (uint16_t) G->reportSize;
            rpt->usage = S->usage[i % S->usageCount];
            rpt->count = S->usageCount == 1 ? (uint16_t) G->reportCount : 1;
            rpt->type = t;
            dbg_printf("    Usage: %04X\r\n", S->usage[i]);
        }
    }

}

int parser_token(parser_state* S, token* tok) {
    parser_state_global* G = S->globalState;
    if (tok->type == TYPE_MAIN) {
        switch (tok->tag) {
            case TAG_MAIN_INPUT:
                dbg_printf("Input report id=%d, size=%d, count=%d\r\n", G->reportId, G->reportSize, G->reportCount);
                add_rpts(S, REPORT_INPUT);
                break;
            case TAG_MAIN_OUTPUT:
                dbg_printf("Output report id=%d, size=%d, count=%d\r\n", G->reportId, G->reportSize, G->reportCount);
                add_rpts(S, REPORT_OUTPUT);
                break;
            case TAG_MAIN_FEATURE:
                dbg_printf("Feature report id=%d, size=%d, count=%d\r\n", G->reportId, G->reportSize, G->reportCount);
                add_rpts(S, REPORT_FEATURE);
                break;
            case TAG_MAIN_COLLECTION:
                assert(tok->size <= 1);

                for (int i = 0; i < S->usageCount; ++i) {
                    dbg_printf("    Usage: %04X\r\n", S->usage[i]);
                }
                assert(S->usageCount == 1);
                S->curNode = ast_add_child(S->curNode, NODE_COLLECTION);
                S->curNode->data.collection.usage = S->usage[0];
                S->curNode->data.collection.type = tok->size ? tok->data[0] : 0;
                dbg_printf("Collection, type=%d\r\n", S->curNode->data.collection.type);
                break;
            case TAG_MAIN_END_COLLECTION:
                S->curNode = ast_exit_child(S->curNode);
                dbg_printf("End collection\r\n");
                break;
        }
        S->usageCount = 0;
    } else if (tok->type == TYPE_GLOBAL) {
        switch (tok->tag) {
            case TAG_GLOBAL_USAGE_PAGE:
                assert(tok->size <= 2);
                G->usagePage = 0;
                memcpy((uint8_t*)&G->usagePage, tok->data, tok->size);
                break;
            case TAG_GLOBAL_LOGICAL_MINIMUM:
                break;
            case TAG_GLOBAL_LOGICAL_MAXIMUM:
                break;
            case TAG_GLOBAL_PHYSICAL_MINIMUM:
                break;
            case TAG_GLOBAL_PHYSICAL_MAXIMUM:
                break;
            case TAG_GLOBAL_UNIT_EXPONENT:
                break;
            case TAG_GLOBAL_UNIT:
                break;
            case TAG_GLOBAL_REPORT_SIZE:
                assert(tok->size <= 2);
                G->reportSize = 0;
                memcpy(&G->reportSize, tok->data, tok->size);
                break;
            case TAG_GLOBAL_REPORT_ID:
                assert(tok->size == 1);
                G->reportId = tok->data[0];
                S->curBit = 0;
                S->usageCount = 0;
                break;
            case TAG_GLOBAL_REPORT_COUNT:
                assert(tok->size <= 2);
                G->reportCount = 0;
                memcpy(&G->reportCount, tok->data, tok->size);
                break;
            case TAG_GLOBAL_PUSH:
                global_state_push(S);
                break;
            case TAG_GLOBAL_POP:
                global_state_pop(S);
                break;
        }

    } else if (tok->type == TYPE_LOCAL) {
        switch (tok->tag) {
            case TAG_LOCAL_USAGE:
                assert(tok->size <= 4);
                assert(S->usageCount >= 0);
                S->usage[S->usageCount] = G->usagePage << 16;
                memcpy(&S->usage[S->usageCount], tok->data, tok->size);
                S->usageCount++;
                break;
            case TAG_LOCAL_USAGE_MINIMUM:
                assert(tok->size <= 4);
                S->usageMinimum = G->usagePage << 16;
                memcpy(&S->usageMinimum, tok->data, tok->size);
                S->usageCount = -1;
                break;
            case TAG_LOCAL_USAGE_MAXIMUM:
                assert(tok->size <= 4);
                S->usageMaximum = G->usagePage << 16;
                memcpy(&S->usageMaximum, tok->data, tok->size);
                S->usageCount = -1;
                break;
            case TAG_LOCAL_DESIGNATOR_INDEX:
                break;
            case TAG_LOCAL_DESIGNATOR_MINIMUM:
                break;
            case TAG_LOCAL_DESIGNATOR_MAXIMUM:
                break;
            case TAG_LOCAL_STRING_INDEX:
                break;
            case TAG_LOCAL_STRING_MINIMUM:
                break;
            case TAG_LOCAL_STRING_MAXIMUM:
                break;
            case TAG_LOCAL_DELIMITER:
                // TODO: We should stop parsing locals after a closing delimiter
                //   and continue after next main item
                break;
        }
    }
    return 0;
}

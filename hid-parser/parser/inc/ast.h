#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define USAGE_NONE 0

typedef struct ast_node ast_node;
typedef struct ast_collection ast_collection;
typedef struct ast_report ast_report;

enum ast_node_t {
    NODE_COLLECTION,
    NODE_REPORT_FIELD,
    NODE_FIND_RESULT,
};

enum ast_report_type {
    REPORT_INPUT,
    REPORT_OUTPUT,
    REPORT_FEATURE
};

/*
 * We support only linear units,
 * as noting else is needed for touchpads.
 */
enum unit {
	UNIT_CM = 1,
	UNIT_INCH = 3,
};

struct ast_collection {
    uint16_t size;
    uint8_t type;
    uint32_t usage;
    ast_node **children;
};

struct ast_report {
    uint8_t id;
    uint16_t start;
    uint16_t size;
    uint16_t count;
    uint32_t usage;
    enum unit unit;
    int32_t physMax, physMin;
    int32_t logMin, logMax;
    int8_t unitExpo;
    enum ast_report_type type;
};

struct ast_node {
    enum ast_node_t type;
    ast_node* parent;
    union ast_node_data {
        ast_collection collection;
        ast_report report;
    } data;
};

ast_node* ast_alloc();
ast_node* ast_add_child(ast_node* node, enum ast_node_t type);
ast_node* ast_exit_child(ast_node* node);
ast_node* ast_find(ast_node* root, bool (*match)(ast_node*));
uint16_t ast_get_rpt_len(ast_node* root, uint8_t rpt_id);
void ast_free(ast_node* root);
void ast_print(ast_node* root);

#include "ast.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

ast_node* ast_alloc() {
    ast_node* curNode = malloc(sizeof(ast_node));
    curNode->type = NODE_COLLECTION;
    curNode->parent = NULL;
    curNode->data.collection.size = 0;
    curNode->data.collection.usage = 0;
    curNode->data.collection.children = NULL;
    return curNode;
}

ast_node* ast_add_child(ast_node* node, enum ast_node_t type) {
    ast_node* newnode = malloc(sizeof (ast_node));
    newnode->parent = node;
    node->data.collection.size++;
    node->data.collection.children =
            realloc(
                node->data.collection.children,
                sizeof(ast_node*) * node->data.collection.size
            );
    node->data.collection.children[node->data.collection.size  - 1] = newnode;
    newnode->type = type;
    switch(type) {
        case NODE_COLLECTION:
            memset(&newnode->data, 0, sizeof (ast_collection));
            break;
        case NODE_REPORT_FIELD:
            memset(&newnode->data, 0, sizeof (ast_report));
            break;
        case NODE_FIND_RESULT:
            memset(&newnode->data, 0, sizeof (ast_collection));
            break;
    }
    return newnode;
}

ast_node* ast_exit_child(ast_node* node) {
    return node->parent;
}

void ast_free(ast_node* root) {
    switch(root->type) {
    case NODE_COLLECTION: {
        for (uint32_t i = 0; i < root->data.collection.size; ++i) {
            ast_free(root->data.collection.children[i]);
        }
        if (root->data.collection.children) {
            free(root->data.collection.children);
        }
    } break;
    case NODE_FIND_RESULT: {
        for (uint32_t i = 0; i < root->data.collection.size; ++i) {
            free(root->data.collection.children[i]);
        }
        if (root->data.collection.children) {
            free(root->data.collection.children);
        }
    } break;
    default: break;
    }

    free(root);
}

static void _ast_print(ast_node* node, uint32_t indent_level) {
    char indent[16];
    indent_level = indent_level > 4 ? 4 : indent_level;
    memset(indent, 0, 16);
    memset(indent, ' ', indent_level * 4);
    switch(node->type) {
    case NODE_COLLECTION: {
        printf("%sCollection, usage=%X\r\n", indent, node->data.collection.usage);
        for (uint32_t i = 0; i < node->data.collection.size; ++i) {
            _ast_print(node->data.collection.children[i], indent_level + 1);
        }
    } break;
    case NODE_REPORT_FIELD: {
        printf("%sReport field %d, id=%d, usage=%X, start=%d, size=%d, count=%d\r\n", indent, node->data.report.type, node->data.report.id, node->data.report.usage, node->data.report.start, node->data.report.size, node->data.report.count);
    } break;
    case NODE_FIND_RESULT: {
        printf("%sResults:\r\n", indent);
        for (uint32_t i = 0; i < node->data.collection.size; ++i) {
            _ast_print(node->data.collection.children[i], indent_level + 1);
        }
    } break;
    }
}

void ast_print(ast_node* root) {
    _ast_print(root, 0);
}

static ast_node* _ast_find(ast_node* node, bool (*match)(ast_node*), ast_node* out) {
    out->type = NODE_FIND_RESULT;

    if (match(node)) {
        ast_node* newnode = ast_add_child(out, node->type);
        memcpy(&newnode->data, &node->data, sizeof(union ast_node_data));
    }

    switch(node->type) {
        case NODE_FIND_RESULT:
        case NODE_COLLECTION: {
            for (uint32_t i = 0; i < node->data.collection.size; ++i) {
                _ast_find(node->data.collection.children[i], match, out);
            }
        } break;
        case NODE_REPORT_FIELD: break;
    }
    return out;
}

ast_node* ast_find(ast_node* root, bool (*match)(ast_node*)) {
    return _ast_find(root, match, ast_alloc());
}

uint16_t ast_get_rpt_len(ast_node* root, uint8_t rpt_id) {
    if (root->type == NODE_REPORT_FIELD && root->data.report.id == rpt_id) {
        return root->data.report.size * root->data.report.count;
    }

    if (root->type == NODE_REPORT_FIELD) {
        return 0;
    }

    uint16_t sum = 0;
    for (uint32_t i = 0; i < root->data.collection.size; ++i) {
        sum += ast_get_rpt_len(root->data.collection.children[i], rpt_id);
    }

    return sum;
}

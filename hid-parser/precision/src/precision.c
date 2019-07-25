#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include "precision.h"
#include "util.h"

static bool match_fingers(ast_node* node) {
    return
            node->type == NODE_COLLECTION &&
            node->data.collection.usage == 0xd0022 &&
            node->parent &&
            node->parent->type == NODE_COLLECTION &&
            node->parent->data.collection.usage == 0xd0005;
}

static bool match_digitizer(ast_node* node) {
    return
            node->type == NODE_COLLECTION &&
            node->data.collection.usage == 0xd0005;
}

static bool match_caps(ast_node* node) {
    return
            node->type == NODE_REPORT_FIELD &&
            (
                node->data.report.usage == 0xd0055 ||
                node->data.report.usage == 0xd0059
            );
}

static bool match_latency(ast_node* node) {
    return
            node->type == NODE_REPORT_FIELD &&
            node->data.report.usage == 0xd0060;
}

static bool match_cert_status(ast_node* node) {
    return
            node->type == NODE_REPORT_FIELD &&
            node->data.report.usage == 0xff0000c5;
}

static bool match_configuration(ast_node* node) {
    return
            node->type == NODE_COLLECTION &&
            node->data.collection.usage == 0xd000e;
}

static bool match_input_mode(ast_node* node) {
    return
            node->type == NODE_REPORT_FIELD &&
            node->data.report.usage == 0xd0052;
}

static bool match_selective_reporting(ast_node* node) {
    return
            node->type == NODE_REPORT_FIELD &&
            (
                node->data.report.usage == 0xd0057 ||
                node->data.report.usage == 0xd0058
            );
}

static bool match_mouse(ast_node* node) {
    return
            node->type == NODE_COLLECTION &&
			node->data.collection.usage == 0x10002;
}

static bool match_rpts(ast_node* node) {
    return
            node->type == NODE_REPORT_FIELD;
}

typedef struct {
    uint32_t usage;
    uint16_t ofs;
} mapping;

uint8_t fill_mover(bitmover_data* mv, ast_node* root, mapping* map, uint16_t len) {
    uint8_t id = 0;
    ast_collection* rootdata = &root->data.collection;

    for (int i = 0; i < rootdata->size; ++i) {
        if (rootdata->children[i]->type != NODE_REPORT_FIELD) {
            continue;
        }
        ast_report* element = &rootdata->children[i]->data.report;

        for (uint16_t j = 0; j < len; ++j) {
            if (element->usage == map[j].usage) {
                bitmover_remember(mv, map[j].ofs, element->start, element->size * element->count);
                id = element->id;
                break;
            }
        }
    }
    return id;
}

int prec_init_from_ast(ast_node* root, prec_config *movers) {
    /* Windows Precision Touchpad input report */ {
        // Digitizer
        ast_node *_digitizer = ast_find(root, match_digitizer);

        assert(_digitizer->data.collection.size == 1);
        ast_node *digitizer = _digitizer->data.collection.children[0];

        mapping map[] = {
            {0x0d0056, 8 * offsetof(prec_report_info, scanTime)},
            {0x0d0054, 8 * offsetof(prec_report_info, contactCount)},
            {0x090001, 8 * offsetof(prec_report_info, button)},
        };

        movers->prec_report_id = fill_mover(&movers->digitizer, digitizer, map, 3);
        movers->prec_report_len = ast_get_rpt_len(root, movers->prec_report_id);
        ast_free(_digitizer);

        // Fingers
        ast_node* _fingers = ast_find(root, match_fingers);
        ast_collection* fingers = &_fingers->data.collection;

        movers->fingerCount = fingers->size;

        for (int i = 0; i < fingers->size; ++i) {
            if (fingers->children[i]->type != NODE_COLLECTION) {
                continue;
            }

            mapping map[] = {
                {0x0d0051, 8 * offsetof(prec_finger, ID)},
                {0x010030, 8 * offsetof(prec_finger, X)},
                {0x010031, 8 * offsetof(prec_finger, Y)},
                {0x0d0042, 8 * offsetof(prec_finger, tip)},
                {0x0d0047, 8 * offsetof(prec_finger, confidence)},
            };

            fill_mover(&movers->finger[i], fingers->children[i], map, 5);
            assert(movers->finger[i].count == 5);

            /* Now get the units */
            {
            	ast_collection* fingColl = &fingers->children[i]->data.collection;
            	for (int i = 0; i < fingColl->size; ++i) {
            		if (fingColl->children[i]->type == NODE_REPORT_FIELD && (fingColl->children[i]->data.report.usage & ~1) == 0x010030) {
            			int idx = fingColl->children[i]->data.report.usage & 1;
            			ast_report* rpt = &fingColl->children[i]->data.report;
            			movers->phys[idx].physicalMaximum = rpt->physMax;
            			movers->phys[idx].physicalMinimum = rpt->physMin;
            			movers->phys[idx].logicalMaximum = rpt->logMax;
            			movers->phys[idx].logicalMinimum = rpt->logMin;
            			movers->phys[idx].unit = rpt->unit;
            			movers->phys[idx].unitExponent = rpt->unitExpo;
            		}
            	}
            	assert(movers->phys[0].unit);
            	assert(movers->phys[1].unit);
            }
        }
        ast_free(_fingers);
    }

    /* Device capabilities feature report */ {
        ast_node *_digitizer = ast_find(root, match_digitizer);
        ast_node *_caps = ast_find(_digitizer, match_caps);

        mapping map[] = {
            {0x0d0055, 8 * offsetof(prec_caps_report, contactMax)},
            {0x0d0059, 8 * offsetof(prec_caps_report, buttonType)},
        };

        movers->caps_report_id = fill_mover(&movers->caps, _caps, map, 2);
        assert(movers->caps.count == 2);
        movers->caps_report_len = ast_get_rpt_len(root, movers->caps_report_id);
        ast_free(_digitizer);
        ast_free(_caps);
    }

    /* Latency mode feature report */ {
        ast_node *_digitizer = ast_find(root, match_digitizer);
        ast_node *_latency = ast_find(_digitizer, match_latency);

        mapping map[] = {
            {0x0d0060, 8 * offsetof(prec_latency_report, mode)},
        };

        movers->latency_report_id = fill_mover(&movers->latency, _latency, map, 1);
        assert(movers->latency.count == 1);
        movers->latency_report_len = ast_get_rpt_len(root, movers->latency_report_id);
        ast_free(_digitizer);
        ast_free(_latency);
    }

    /* Device certification status feature report */ {
        ast_node *_digitizer = ast_find(root, match_digitizer);
        ast_node *_cert = ast_find(_digitizer, match_cert_status);

        mapping map[] = {
            {0xff0000c5, 0 /* dummy*/},
        };

        bitmover_data data;
        bitmover_init(&data);
        movers->cert_status_report_id = fill_mover(&data, _cert, map, 1);
        assert(data.count == 1);
        movers->cert_status_report_len = ast_get_rpt_len(root, movers->cert_status_report_id);
        ast_free(_digitizer);
        ast_free(_cert);
    }

    /* Input mode feature report */ {
        ast_node *_config = ast_find(root, match_configuration);
        ast_node *_input_mode = ast_find(_config, match_input_mode);

        mapping map[] = {
            {0x0d0052, 8 * offsetof(prec_input_mode_report, mode)},
        };

        movers->input_mode_report_id = fill_mover(&movers->input_mode, _input_mode, map, 1);
        assert(movers->input_mode.count == 1);
        movers->input_mode_report_len = ast_get_rpt_len(root, movers->input_mode_report_id);
        ast_free(_config);
        ast_free(_input_mode);
    }

    /* Selective reporting feature report */ {
        ast_node *_digitizer = ast_find(root, match_configuration);
        ast_node *_sel_rep = ast_find(_digitizer, match_selective_reporting);

        mapping map[] = {
            {0x0d0057, 8 * offsetof(prec_selective_reporting_report, surface)},
            {0x0d0058, 8 * offsetof(prec_selective_reporting_report, sw)},
        };

        movers->selective_reporting_report_id = fill_mover(&movers->selective_reporting, _sel_rep, map, 2);
        assert(movers->selective_reporting.count == 2);
        movers->selective_reporting_report_len = ast_get_rpt_len(root, movers->selective_reporting_report_id);
        ast_free(_digitizer);
        ast_free(_sel_rep);
    }

    /* Plain old mouse report */ {
		ast_node *_mouse = ast_find(root, match_mouse);
		ast_node *_rep = ast_find(_mouse, match_rpts);

		mapping map[] = {
			{0x090001, 8 * offsetof(prec_mouse_report, btn1)},
			{0x090002, 8 * offsetof(prec_mouse_report, btn2)},
			{0x010030, 8 * offsetof(prec_mouse_report, X)},
			{0x010031, 8 * offsetof(prec_mouse_report, Y)},
		};

		movers->mouse_report_id = fill_mover(&movers->mouse, _rep, map, 4);
		assert(movers->mouse.count == 4);
		movers->mouse_report_len = ast_get_rpt_len(root, movers->mouse_report_id);
		ast_free(_mouse);
		ast_free(_rep);
	}

    return 0;
}

void bitmover_init(bitmover_data* data) {
    data->count = 0;
}

void bitmover_remember(bitmover_data* data, uint16_t dst_ofs, uint16_t src_ofs, uint16_t len) {
    dbg_printf("Remember: src=%d, dst=%d, len=%d\r\n", src_ofs, dst_ofs, len);
    data->moves[data->count++] = (struct bitmover_moves) {
        .dst_ofs = dst_ofs,
        .src_ofs = src_ofs,
        .len = len
    };
}

void bitmover_move(bitmover_data* data, void* src, void* dst) {
    for (uint16_t i = 0; i < data->count; ++i) {
        memcpy_bit(dst, src, data->moves[i].dst_ofs, data->moves[i].src_ofs, data->moves[i].len);
    }
}

void bitmover_move_rev(bitmover_data* data, void* src, void* dst) {
    for (uint16_t i = 0; i < data->count; ++i) {
        memcpy_bit(src, dst, data->moves[i].src_ofs, data->moves[i].dst_ofs, data->moves[i].len);
    }
}

void memcpy_bit(void* dst, void* src, uint16_t dst_ofs, uint16_t src_ofs, uint16_t len) {
    dbg_printf("Move: src=%d, dst=%d, len=%d\r\n", src_ofs, dst_ofs, len);
    uint8_t *srcu = (uint8_t*)src, *dstu = (uint8_t*) dst;
    for (uint16_t i = 0; i < len; ++i) {
        uint16_t _src = src_ofs + i;
        uint16_t _dst = dst_ofs + i;
        uint16_t src_byte = _src / 8;
        uint16_t dst_byte = _dst / 8;
        uint8_t src_bit = _src % 8;
        uint8_t dst_bit = _dst % 8;

        if (srcu[src_byte] & (1 << src_bit)) {
            dstu[dst_byte] |= (1 << dst_bit);
        } else {
            dstu[dst_byte] &= ~(1 << dst_bit);
        }
    }
}

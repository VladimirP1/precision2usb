#pragma once
#include "ast.h"
#include <stdbool.h>

typedef struct prec_config prec_config;
typedef struct prec_state prec_state;
typedef struct prec_finger prec_finger;
typedef struct prec_report_info prec_report_info;
typedef struct prec_report prec_report;
typedef struct prec_caps_report prec_caps_report;
typedef struct prec_latency_report prec_latency_report;
typedef struct prec_input_mode_report prec_input_mode_report;
typedef struct prec_selective_reporting_report prec_selective_reporting_report;
typedef struct prec_mouse_report prec_mouse_report;

struct prec_finger {
    uint8_t ID;
    uint16_t X;
    uint16_t Y;
    uint8_t tip;
    uint8_t confidence;
};

struct prec_report_info {
	uint16_t scanTime;
	uint8_t contactCount;
	uint8_t button;
};

typedef struct {
    uint16_t count;
    struct bitmover_moves {
        uint16_t dst_ofs;
        uint16_t src_ofs;
        uint16_t len;
    } moves[16];
} bitmover_data;

// Windows Precision Touchpad input report data
struct prec_report {
	prec_report_info info;
    prec_finger fingers[5];
};

// Device capabilities feature report data
struct prec_caps_report {
    uint8_t contactMax;
    uint8_t buttonType;
};

// Latency mode feature report
struct prec_latency_report {
    uint8_t mode;
};

// Input mode feature report
struct prec_input_mode_report {
    uint8_t mode;
};

// Selective reporting feature report
struct prec_selective_reporting_report {
    uint8_t surface;
    uint8_t sw;
};

// Plain old mouse report
struct prec_mouse_report {
    uint8_t btn1;
    uint8_t btn2;
    int8_t X;
    int8_t Y;
};

struct prec_config {
    // Windows Precision Touchpad input report
    uint8_t prec_report_id;
    uint16_t prec_report_len;
    bitmover_data digitizer;
    bitmover_data finger[5];
    uint16_t fingerCount;
    // Device capabilities feature report
    uint8_t caps_report_id;
    uint16_t caps_report_len;
    bitmover_data caps;
    // Latency mode feature report
    uint8_t latency_report_id;
    uint16_t latency_report_len;
    bitmover_data latency;
    // Device certification status feature report
    uint8_t cert_status_report_id;
    uint16_t cert_status_report_len;
    // Input mode feature report
    uint8_t input_mode_report_id;
    uint16_t input_mode_report_len;
    bitmover_data input_mode;
    // Selective reporting feature report
    uint8_t selective_reporting_report_id;
    uint16_t selective_reporting_report_len;
    bitmover_data selective_reporting;
    // Plain old mouse report
    uint8_t mouse_report_id;
    uint16_t mouse_report_len;
    bitmover_data mouse;
};

//extern prec_config movers;

struct prec_state {
	bool wasPrecision;
	prec_mouse_report mouse;
	prec_input_mode_report inputMode;
	prec_selective_reporting_report selectiveRep;
	prec_caps_report deviceCaps;
	prec_latency_report latency;
	prec_report input;
};

void bitmover_init(bitmover_data* data);
void bitmover_remember(bitmover_data* data, uint16_t dst_ofs, uint16_t src_ofs, uint16_t len);
void bitmover_move(bitmover_data* data, void* src, void* dst);
void bitmover_move_rev(bitmover_data* data, void* src, void* dst);

void memcpy_bit(void* dst, void* src, uint16_t dst_ofs, uint16_t src_ofs, uint16_t len);

int prec_init_from_ast(ast_node* root, prec_config *movers);

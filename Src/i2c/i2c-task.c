#include "i2c.h"
#include "ast.h"
#include "parser.h"
#include "tokenizer.h"
#include "precision.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <message_buffer.h>
#include <libopencm3/cm3/cortex.h>

hid_dev_desc device_desc;
static uint8_t buf[1024];
prec_state precState;
prec_config movers;

void parse_report_desc(uint8_t* buf, size_t len) {
	token tok;
	tokenizer_state tS;
	parser_state pS;
	token_init(&tS, buf, len);
	parser_init(&pS);
	while(!token_next(&tS, &tok)) {
		parser_token(&pS, &tok);
	}
	//ast_print(pS.rootNode);
	prec_init_from_ast(pS.rootNode, &movers);
	parser_deinit(&pS);
}

static int update_state(prec_state* S, prec_config* C, uint8_t* report, uint16_t len) {
	if (!len) {
		return 1;
	}

	if (report[0] == C->caps_report_id) {
		bitmover_move(&C->caps, report + 1, &S->deviceCaps);
	} else if (C->latency_report_id) {
		bitmover_move(&C->latency, report + 1, &S->latency);
	} else if (C->input_mode_report_id) {
		bitmover_move(&C->input_mode, report + 1, &S->inputMode);
	} else if (C->selective_reporting_report_id) {
		bitmover_move(&C->selective_reporting, report + 1, &S->selectiveRep);
	} else if (C->mouse_report_id) {
		bitmover_move(&C->mouse, report + 1, &S->mouse);
	} else if (C->prec_report_id) {
		bitmover_move(&C->digitizer, report + 1, &S->input);
		for (uint16_t i = 0; i < C->fingerCount; ++i) {
			bitmover_move(&C->finger[i], report + 1, &S->input.fingers[i]);
		}
	}
	return 0;
}

static int update_device(uint8_t id, uint16_t len, bitmover_data* mv, void* src) {
	buf[0] = id;
	memset(buf, 0, len + 1);
	bitmover_move_rev(mv, buf + 1, src);
	uint8_t status = i2c_cmd_set_report(0x2c,  device_desc.wCommandRegister, device_desc.wDataRegister, buf, len + 1);
	if (status) {
		//printf("Error setting report\r\n");
	}
	return 0;
}

static void get_report(uint8_t id) {
	uint16_t len = (uint16_t) sizeof(buf);
	uint8_t status = i2c_cmd_get_report(0x2c,  device_desc.wCommandRegister, device_desc.wDataRegister, id, buf, &len);
	if (status) {
		//printf("Error getting report\r\n");
	}
	update_state(&precState, &movers, buf + 2, len - 2);
	//printf("id=%d, len=%d\r\n", id, len);
}

static void get_report_input() {
	static uint8_t fingIdx = 0;
	static uint8_t fingTotal = 0;

	uint16_t len = sizeof(buf);
	if (i2c_get_report(0x2c, device_desc.wInputRegister, buf, &len)) {
		return;
	}
	uint8_t *rptBuf = buf + 3;

	precState.wasPrecision = buf[2] == movers.prec_report_id;

	prec_report_info nfo;
	bitmover_move(&movers.digitizer, rptBuf, &nfo);

	if (nfo.contactCount != 0) {
		precState.input.info = nfo;
		fingTotal = nfo.contactCount;
		fingIdx = 0;
	}

	uint8_t fingLeft = fingTotal - fingIdx;
	fingLeft = movers.fingerCount > fingLeft ? fingLeft : movers.fingerCount;

	for (uint8_t i = 0; i < fingLeft; ++i) {
		bitmover_move(&movers.finger[i], rptBuf, &precState.input.fingers[fingIdx++]);
	}
}

void i2c_task(void* arg) {
	sem_alert = xSemaphoreCreateBinary();
	publicInterface.toHostReportBuf = xMessageBufferCreate(128);
	publicInterface.toDeviceReportBuf = xMessageBufferCreate(128);

	i2c_init();
	int_init();
	cdcacm_read((char*)buf, 1);
	i2c_recover();
	i2c_recover();
	vTaskDelay(10);

	int status = i2c_read_reg(0x2c,0x20,&device_desc,30);

	vTaskDelay(10);

	status = i2c_read_reg(0x2c, device_desc.wReportDescRegister, buf, device_desc.wReportDescLength);
	parse_report_desc(buf, device_desc.wReportDescLength);

	get_report(movers.caps_report_id);
	precState.inputMode.mode = 3;
	update_device(movers.input_mode_report_id, movers.input_mode_report_len/8, &movers.input_mode, &precState.inputMode);

	command* cmd = pvPortMalloc(256);
	xSemaphoreGive(sem_alert);
	while(1) {
		xSemaphoreTake(sem_alert, portMAX_DELAY);

		/* check for input report and send to usb task */
		get_report_input();
		if (precState.wasPrecision) {
			cmd->cmdType = CMD_INPUT_PREC;
			memcpy(cmd->data, &precState.input, sizeof(prec_report));
		} else {
			cmd->cmdType = CMD_INPUT_MOUSE;
			memcpy(cmd->data, &precState.mouse, sizeof(prec_mouse_report));
		}
		xMessageBufferSend(publicInterface.toHostReportBuf, &cmd, 256, 0);

		/* check for incoming feature reports from usb task */
		size_t bytes = xMessageBufferReceive(publicInterface.toDeviceReportBuf, &cmd, 256, 0);
		if (!bytes) {
			continue;
		}

		switch (cmd->cmdType) {
			case CMD_FEATURE_INPUT_MODE:
				memcpy(&precState.inputMode, cmd->data, sizeof(prec_input_mode_report));
				update_device(movers.input_mode_report_id, movers.input_mode_report_len/8, &movers.input_mode, &precState.inputMode);
				break;
			default: break;
		}
	}
}

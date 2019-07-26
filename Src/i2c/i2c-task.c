#include "i2c.h"
#include "ast.h"
#include "parser.h"
#include "tokenizer.h"
#include "precision.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <message_buffer.h>
#include <usb_hid_cdc.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/stm32/gpio.h>

hid_dev_desc device_desc;
uint8_t buf[1024];
prec_state precState;
static prec_config movers;
uint8_t i2cDevice = 0;

static void parse_report_desc(uint8_t* buf, size_t len) {
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
	memset(buf, 0, len + 1);
	buf[0] = id;
	bitmover_move_rev(mv, buf + 1, src);
	uint8_t status = i2c_cmd_set_report(i2cDevice,  device_desc.wCommandRegister, device_desc.wDataRegister, buf, len + 1);
	assert(!status);
	return 0;
}

static void get_report(uint8_t id) {
	uint16_t len = (uint16_t) sizeof(buf);
	uint8_t status = i2c_cmd_get_report(i2cDevice,  device_desc.wCommandRegister, device_desc.wDataRegister, id, buf, &len);
	assert(!status);
	update_state(&precState, &movers, buf + 2, len - 2);
	//printf("id=%d, len=%d\r\n", id, len);
}

static int get_report_input() {
	static uint8_t fingIdx = 0;
	static uint8_t fingTotal = 0;

	uint16_t len = sizeof(buf);
	if (i2c_get_report(i2cDevice, device_desc.wInputRegister, buf, &len)) {
		return 1;
	}
	uint8_t *rptBuf = buf + 3;

	precState.wasPrecision = buf[2] == movers.prec_report_id;

	if (precState.wasPrecision) {
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
	} else {
		bitmover_move(&movers.mouse, rptBuf, &precState.mouse);
	}
	return 0;
}

void i2c_task(void* arg) {
	sem_alert = xSemaphoreCreateBinary();
	publicInterface.toHostReportBuf = xMessageBufferCreate(128);
	publicInterface.toDeviceReportBuf = xMessageBufferCreate(128);

	vTaskDelay(10);

	i2c_init();
	int_init();
	int status = 1;
	while(status || device_desc.wHIDDescLength != 30 || device_desc.wReportDescLength > (1024 - 2)) {
		i2c_recover();
		vTaskDelay(1);
		i2c_recover();
		vTaskDelay(50);

		i2cDevice = 0x2c; // i2c_scan();

		status = i2c_read_reg(i2cDevice,0x20,&device_desc,30);
	}

	vTaskDelay(5);

	status = i2c_read_reg(i2cDevice, device_desc.wReportDescRegister, buf, device_desc.wReportDescLength);
	parse_report_desc(buf, device_desc.wReportDescLength);

	usb_hid_setup_units(movers.phys);

	usb_init();

	get_report(movers.caps_report_id);

	command* cmd = pvPortMalloc(256);

	xSemaphoreGive(sem_alert);
	while(1) {
		xSemaphoreTake(sem_alert, 400);

		if (gpio_get(GPIOB, GPIO12)) {
			continue;
		}

		/* check for input report and send to usb task */
		if (get_report_input()) {
			continue;
		}

		/*{
			prec_mouse_report rpt = {
					.X = 0,
					.Y = 0,
					.btn1 = 0,
					.btn2 = 0,
					.btn3 = 0,
			};
			if ((xTaskGetTickCount() % 10000) > 5000) {
				//precState.input.info.contactCount = 1;
				//rpt.X = 1;
				rpt.btn3 = 1;
			}

			cmd->cmdType = CMD_INPUT_MOUSE2;
			memcpy(cmd->data, &rpt, sizeof(prec_mouse_report));
			xMessageBufferSend(publicInterface.toHostReportBuf, cmd, sizeof(prec_report) + sizeof(cmd_type), 0);
		}*/

		if (precState.wasPrecision) {
			cmd->cmdType = CMD_INPUT_PREC;
			memcpy(cmd->data, &precState.input, sizeof(prec_report));
		} else {
			cmd->cmdType = CMD_INPUT_MOUSE;
			memcpy(cmd->data, &precState.mouse, sizeof(prec_mouse_report));
		}
		xMessageBufferSend(publicInterface.toHostReportBuf, cmd, sizeof(prec_report) + sizeof(cmd_type), 0);

		/* info display */ {
			static int requests = 0;
			static TickType_t ticks_prev = 0;
			TickType_t ticks_now = xTaskGetTickCount();
			++requests;
			if(ticks_now - ticks_prev > 1000) {
				printf("ADR: %02x  FPS: %d rpt/s  MODE: %s\r\n", i2cDevice, requests, precState.wasPrecision ? "precision" : "legacy");
				ticks_prev = ticks_now;
				requests = 0;
			}
		}

		/* check for incoming feature reports from usb task */
		size_t bytes = xMessageBufferReceive(publicInterface.toDeviceReportBuf, cmd, 256, 0);
		if (!bytes) {
			continue;
		}

		switch (cmd->cmdType) {
			case CMD_FEATURE_INPUT_MODE:
				memcpy(&precState.inputMode, cmd->data, sizeof(prec_input_mode_report));
				update_device(movers.input_mode_report_id, movers.input_mode_report_len/8, &movers.input_mode, &precState.inputMode);
				break;
			case CMD_FEATURE_LATENCY_MODE:
				memcpy(&precState.latency, cmd->data, sizeof(prec_latency_report));
				update_device(movers.latency_report_id, movers.latency_report_len/8, &movers.latency, &precState.latency);
				break;
			case CMD_FEATURE_SELECTIVE_REPORTING:
				memcpy(&precState.selectiveRep, cmd->data, sizeof(prec_selective_reporting_report));
				update_device(movers.selective_reporting_report_id, movers.selective_reporting_report_len/8, &movers.selective_reporting, &precState.selectiveRep);
				break;
			default: break;
		}
	}
}

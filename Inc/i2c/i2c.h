#pragma once
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <stdint.h>
#include <message_buffer.h>


uint8_t i2c_handle(uint8_t in);
void i2c_init();
void i2c_start();
void i2c_stop();
void i2c_recover();
uint8_t i2c_scan();
uint8_t i2c_rcvbyte(uint8_t ack);
uint8_t i2c_sendbyte(uint8_t data);
uint8_t i2c_transfer(uint8_t adr, uint8_t* wbuf, size_t wlen, uint8_t*rbuf,
		size_t rlen);

extern SemaphoreHandle_t sem_alert;
void int_init();

uint8_t i2c_read_reg(uint8_t adr, uint16_t reg, void* buf, uint16_t len);
uint8_t i2c_write_reg(uint8_t adr, uint16_t reg, void* buf, uint16_t len);
uint8_t i2c_get_report(uint8_t adr, uint16_t reg, void* buf, uint16_t* len);
uint8_t i2c_set_report(uint8_t adr, uint16_t reg, void* buf, uint16_t len);
uint8_t i2c_cmd_get_report(uint8_t adr, uint16_t cmd_reg, uint16_t data_reg, uint8_t rpt_id, uint8_t *buf, uint16_t*len);
uint8_t i2c_cmd_set_report(uint8_t adr, uint16_t cmd_reg, uint16_t data_reg, uint8_t *buf, uint16_t len);
typedef struct {
	 uint16_t wHIDDescLength;
	 uint16_t bcdVersion;
	 uint16_t wReportDescLength;
	 uint16_t wReportDescRegister;
	 uint16_t wInputRegister;
	 uint16_t wMaxInputLength;
	 uint16_t wOutputRegister;
	 uint16_t wMaxOutputLength;
	 uint16_t wCommandRegister;
	 uint16_t wDataRegister;
	 uint16_t wVendorID;
	 uint16_t wProductID;
	 uint16_t wVersionID;
	 uint16_t RESERVED;

} hid_dev_desc;

typedef enum {
	CMD_INPUT_PREC,
	CMD_INPUT_MOUSE,
	CMD_INPUT_MOUSE2,
	CMD_FEATURE_INPUT_MODE,
	CMD_FEATURE_LATENCY_MODE,
	CMD_FEATURE_SELECTIVE_REPORTING,
} cmd_type;

typedef struct {
	cmd_type cmdType;
	uint8_t data[];
} command;

typedef struct {
	MessageBufferHandle_t toHostReportBuf;
	MessageBufferHandle_t toDeviceReportBuf;
} i2c_hid_interface;

i2c_hid_interface publicInterface;

void i2c_task(void* arg);

#include <FreeRTOS.h>
#include <task.h>
#include <string.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/cm3/nvic.h>
#include <io.h>
#include <stdio.h>
#include <semphr.h>
#include "i2c.h"

// ---- i2c bitbang ----

#define G_SDA() gpio_get(GPIOB, GPIO_I2C2_SDA)
#define SDA(x) if(x) gpio_set(GPIOB, GPIO_I2C2_SDA); else  gpio_clear(GPIOB, GPIO_I2C2_SDA);
#define SCL(x) if(x) { gpio_set(GPIOB, GPIO_I2C2_SCL); uint32_t _tmout = 100000; while(!gpio_get(GPIOB,GPIO_I2C2_SCL) && _tmout--); } else gpio_clear(GPIOB, GPIO_I2C2_SCL);
#define HCLK() for(int i=0;i<13;i++){__asm__("nop");};
#define CLK() HCLK();HCLK();

uint8_t i2c_handle(uint8_t in) {
	if(in) {
		i2c_recover();
		CLK();
	}
	return in;
}

void i2c_start() {
	SDA(1);
	SCL(1);
	HCLK();
	SDA(0);
	HCLK();
	SCL(0);
}

uint8_t i2c_rcvbyte(uint8_t ack) {
	uint8_t data = 0;
	SDA(1);
	for (uint8_t i = 0; i < 8; ++i) {
		data <<= 1;
		CLK();
		SCL(1);
		CLK();
		data |= G_SDA() ? 1 : 0;
		SCL(0);
	}
	HCLK();
	SDA(!ack);
	HCLK();
	SCL(1);
	CLK();
	SCL(0);
	CLK();
	return data;
}

uint8_t i2c_sendbyte(uint8_t data) {
	for (uint8_t i = 0; i < 8; ++i) {
		HCLK();
		SDA(data & 0x80);
		HCLK();
		SCL(1);
		CLK();
		SCL(0);
		data <<= 1;
	}
	HCLK();
	SDA(1);
	HCLK();
	SCL(1);
	CLK();
	uint8_t ack = !G_SDA();
	SCL(0);
	CLK();
	return ack;
}

void i2c_stop() {
	SDA(0);
	HCLK();
	SCL(1);
	HCLK();
	SDA(1);
	HCLK();
}

void i2c_recover() {
	i2c_sendbyte(0xff);
	i2c_stop();
}

void i2c_init() {
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_AFIO);

	SDA(1);
	SCL(1);

	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ,
			  GPIO_CNF_OUTPUT_OPENDRAIN,
		      GPIO_I2C2_SCL | GPIO_I2C2_SDA);

}

uint8_t i2c_scan() {
	for (int i = 0; i < 127; ++i) {
		if ((i & 0xf0 == 0) || (i & 0xf0 == 0xf0)) {
			continue;
		}
		i2c_start();
		if (i2c_sendbyte(i << 1)) { // WRITE
			i2c_stop();
			i2c_handle(1);
			return i;
		}
		i2c_stop();
		i2c_handle(1);
		vTaskDelay(1);
	}
	return 0;
}

uint8_t i2c_transfer(uint8_t adr, uint8_t* wbuf, size_t wlen, uint8_t*rbuf, size_t rlen) {
	i2c_start();
	if (!i2c_sendbyte(adr << 1)) { // WRITE
		return i2c_handle(1);
	}
	for (size_t i = 0; i < wlen; ++i) {
		if (!i2c_sendbyte(wbuf[i])) {
			return i2c_handle(2);
		}
	}
	if(rlen) {
		i2c_start();
		if (!i2c_sendbyte(adr << 1 | 1)) { // READ
			return i2c_handle(3);
		}
		for (size_t i = 0; i < rlen; ++i) {
			rbuf[i] = i2c_rcvbyte(i != rlen - 1);
		}
	}
	i2c_stop();
	return 0;
}

/* External interrupt */

SemaphoreHandle_t sem_alert;

void int_init() {
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_AFIO);

	nvic_set_priority(NVIC_EXTI15_10_IRQ, configMAX_SYSCALL_INTERRUPT_PRIORITY + 32);
	nvic_enable_irq(NVIC_EXTI15_10_IRQ);

	gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO_I2C2_SMBAI);

	exti_select_source(EXTI12, GPIOB);
	exti_set_trigger(EXTI12, EXTI_TRIGGER_FALLING);
	exti_enable_request(EXTI12);
}

void exti15_10_isr() {
	exti_reset_request(EXTI12);
	BaseType_t hp_task_woken = pdFALSE;
	xSemaphoreGiveFromISR(sem_alert, &hp_task_woken);
	if (hp_task_woken) {
		taskYIELD();
	}
}

/* I2C hid specific commands */
static uint8_t static_buf[128];

uint8_t i2c_read_reg(uint8_t adr, uint16_t reg, void* buf, uint16_t len) {
	return i2c_transfer(adr, (uint8_t*) &reg, 2, (uint8_t*) buf, len);
}

uint8_t i2c_write_reg(uint8_t adr, uint16_t reg, void* buf, uint16_t len) {
	uint8_t nbuf[len + 2];
	memcpy(nbuf + 2, buf, len);
	memcpy(nbuf, &reg, 2);
	return i2c_transfer(adr, nbuf, len + 2, NULL, 0);
}

uint8_t i2c_set_report(uint8_t adr, uint16_t reg, void* buf, uint16_t len) {
	uint8_t nbuf[len + 4];
	len += 2;
	memcpy(nbuf + 4, buf, len - 2);
	memcpy(nbuf + 2, &len, 2);
	memcpy(nbuf, &reg, 2);
	return i2c_transfer(adr, nbuf, len + 2, NULL, 0);
}

uint8_t i2c_get_report(uint8_t adr, uint16_t reg, void* buf, uint16_t* len) {
	i2c_start();
	if (!i2c_sendbyte(adr << 1)) { // WRITE
		return i2c_handle(1);
	}
	for (size_t i = 0; i < 2; ++i) {
		if (!i2c_sendbyte(((uint8_t*)&reg)[i])) {
			return i2c_handle(2);
		}
	}

	i2c_start();
	if (!i2c_sendbyte(adr << 1 | 1)) { // READ
		return i2c_handle(3);
	}
	if (*len < 2) {
		return i2c_handle(4);
	}
	((uint8_t*)buf)[0] = i2c_rcvbyte(1);
	((uint8_t*)buf)[1] = i2c_rcvbyte(1);

	uint16_t rlen = *((uint16_t*)buf);

	if (*((uint16_t*)buf) < 2) {
		return i2c_handle(12);
	}

	if (rlen > *len) {
		return i2c_handle(5);
	}
	*len = rlen;
	if(rlen) {
		for (size_t i = 2; i < rlen; ++i) {
			((uint8_t*)buf)[i] = i2c_rcvbyte(i != rlen - 1);
		}
	}
	i2c_stop();
	return 0;
}

uint8_t i2c_cmd_get_report(uint8_t adr, uint16_t cmd_reg, uint16_t data_reg, uint8_t rpt_id, uint8_t *buf, uint16_t*len) {

	uint8_t cmd[] = {0x30 | rpt_id, 0x02};

	if (i2c_write_reg(adr, cmd_reg, cmd, 2)) {
		return 1;
	}
	if (i2c_read_reg(adr, data_reg, (uint8_t*)len, 2)) {
		return 2;
	}
	if (i2c_read_reg(adr, data_reg, (uint8_t*)buf, *len)) {
		return 3;
	}
	return 0;
}

uint8_t i2c_cmd_set_report(uint8_t adr, uint16_t cmd_reg, uint16_t data_reg, uint8_t *buf, uint16_t len) {
	struct cmd {
		uint16_t cmd_reg;
		uint8_t rpt_id : 4;
		uint8_t rpt_type : 4;
		uint8_t opcode;
		union {
			struct {
				uint16_t data_reg;
				uint16_t len;
				uint8_t data[1];
			} __attribute__((packed)) small;
			struct {
				uint8_t rpt_id;
				uint16_t data_reg;
				uint16_t len;
				uint8_t data[1];
			} __attribute__((packed)) big;
		} __attribute__((packed)) alt;
	} __attribute__((packed));

	struct cmd *_cmd = (struct cmd*) static_buf;

	_cmd->cmd_reg = cmd_reg;
	_cmd->rpt_id = buf[0] < 15 ? buf[0] : 0xf;
	_cmd->rpt_type = 0x03;
	_cmd->opcode = 0x03;
	if (buf[0] < 15) {
		_cmd->alt.small.len = len + 2;
		_cmd->alt.small.data_reg = data_reg;
		memcpy(_cmd->alt.small.data, buf, len);
	} else {
		_cmd->alt.big.rpt_id = buf[0];
		_cmd->alt.big.len = len + 2;
		_cmd->alt.big.data_reg = data_reg;
		memcpy(_cmd->alt.big.data, buf, len);
	}

	if (i2c_transfer(adr, static_buf, len + 8 + (buf[0] >= 15), NULL, 0)) {
		return 1;
	}
	return 0;
}

uint8_t i2c_cmd_reset(uint8_t adr, uint16_t cmd_reg) {
	struct cmd {
		uint16_t cmd_reg;
		uint8_t rpt_id : 4;
		uint8_t rpt_type : 4;
		uint8_t opcode;
	} __attribute__((packed));

	struct cmd *_cmd = (struct cmd*) static_buf;

	_cmd->cmd_reg = cmd_reg;
	_cmd->rpt_id = 0x00;
	_cmd->rpt_type = 0x00;
	_cmd->opcode = 0x01;

	if (i2c_transfer(adr, static_buf, sizeof(struct cmd), NULL, 0)) {
		return 1;
	}
	return 0;
}



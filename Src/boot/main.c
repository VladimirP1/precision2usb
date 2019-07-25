#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/systick.h>

#include "FreeRTOS.h"
#include "task.h"
#include "rtos_init.h"
#include "usb_hid_cdc.h"
#include "stream_buffer.h"
#include "termio.h"
#include "io.h"
#include "i2c.h"

#include <stdlib.h>
#include <stdio.h>

static void basic_hw_init() {
	//rcc_clock_setup_in_hsi_out_48mhz();
	rcc_clock_setup_in_hse_8mhz_out_72mhz();

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_AFIO);

	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
}

static void init_task(void* arg) {
	vTaskDelay(1000);
	xTaskCreate(i2c_task, "i2c", 3000, NULL, configMAX_PRIORITIES-2, NULL);
	vTaskDelete(0);
}

int main() {
	basic_hw_init();
	xTaskCreate(init_task, "INIT", 100, 0, configMAX_PRIORITIES-1, NULL);
	rtos_start();
}

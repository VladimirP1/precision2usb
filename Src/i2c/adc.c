#include <adc.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/f1/nvic.h>

#include <FreeRTOS.h>
#include <semphr.h>
#include <message_buffer.h>

static SemaphoreHandle_t adc_semphr;
MessageBufferHandle_t adc_msgbuf = 0;

void adc1_2_isr() {
	adc_clear_flag(ADC1, ADC_SR_EOC);
	BaseType_t hp_task_woken = false;
	xSemaphoreGiveFromISR(adc_semphr, &hp_task_woken);
	if (hp_task_woken) {
		taskYIELD();
	}
}

void adc_task(void* arg) {
	uint8_t channel_array[16];

	rcc_periph_clock_enable(RCC_ADC1);

	adc_power_off(ADC1);

	adc_disable_scan_mode(ADC1);
	adc_set_single_conversion_mode(ADC1);
	adc_disable_external_trigger_regular(ADC1);
	adc_set_right_aligned(ADC1);
	adc_enable_temperature_sensor();
	adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_28DOT5CYC);

	adc_power_on(ADC1);

	vTaskDelay(10);

	adc_reset_calibration(ADC1);
	adc_calibrate(ADC1);

	channel_array[0] = 16;
	channel_array[1] = 16;
	channel_array[2] = 16;
	channel_array[3] = 16;
	adc_set_regular_sequence(ADC1, 1, channel_array);

	adc_semphr = xSemaphoreCreateBinary();
	adc_msgbuf = xMessageBufferCreate(12);

	nvic_set_priority(NVIC_ADC1_2_IRQ, configMAX_SYSCALL_INTERRUPT_PRIORITY + 24);
	nvic_enable_irq(NVIC_ADC1_2_IRQ);
	adc_enable_eoc_interrupt(ADC1);

	while (1) {
		adc_start_conversion_direct(ADC1);

		xSemaphoreTake(adc_semphr, portMAX_DELAY);

		uint16_t reading[4];
		reading[0] = adc_read_regular(ADC1);
		reading[1] = adc_read_regular(ADC1);
		reading[2] = adc_read_regular(ADC1);
		reading[3] = adc_read_regular(ADC1);

		xMessageBufferSend(adc_msgbuf, &reading, 2 * 4, portMAX_DELAY);
	}
}

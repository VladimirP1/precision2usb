#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/systick.h>

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName ) {
	while(1);
}

void vPortSetupTimerInterrupt( void )
{
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
	systick_set_reload(( configCPU_CLOCK_HZ / configTICK_RATE_HZ ) - 1UL);
	systick_interrupt_enable();
	systick_counter_enable();
}

void rtos_start() {
	scb_set_priority_grouping(SCB_AIRCR_PRIGROUP_GROUP16_NOSUB);
	vTaskStartScheduler();
	while (1);
}

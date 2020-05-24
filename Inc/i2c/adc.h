#pragma once

#include <FreeRTOS.h>
#include <message_buffer.h>

extern MessageBufferHandle_t adc_msgbuf;

void adc_task(void* arg);

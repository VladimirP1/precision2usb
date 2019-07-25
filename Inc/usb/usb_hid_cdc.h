#pragma once

#include <stream_buffer.h>
#include <precision.h>

void usb_init();
void usb_hid_setup_units(struct prec_config_physinfo phys[2]);

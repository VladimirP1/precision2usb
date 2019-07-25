/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2010 Gareth McMullin <gareth@blacksphere.co.nz>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <FreeRTOS.h>
#include <hid_constants.h>
#include <task.h>
#include <stream_buffer.h>
#include "FreeRTOSConfig.h"
#include <semphr.h>
#include "i2c.h"
#include <hid.h>
#include <precision.h>
#include <string.h>

#include <stdlib.h>
#include <math.h>

#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/hid.h>
#include <libopencm3/usb/cdc.h>


static usbd_device *usbd_dev;
static SemaphoreHandle_t cdc_tx_semaphore;
static SemaphoreHandle_t cdc_rx_semaphore;
static SemaphoreHandle_t hid_input_semaphore;
static StreamBufferHandle_t cdc_tx_buf;
static StreamBufferHandle_t cdc_rx_buf;
static bool usb_ready = false;

const struct usb_device_descriptor dev_descr = {
		.bLength = USB_DT_DEVICE_SIZE,
		.bDescriptorType = USB_DT_DEVICE,
		.bcdUSB = 0x0200,
		.bDeviceClass = 0xef,
		.bDeviceSubClass = 0x02,
		.bDeviceProtocol = 0x01,
		.bMaxPacketSize0 = 64,
		.idVendor = 0x0483,
		.idProduct = 0x5710,
		.bcdDevice = 0x0200,
		.iManufacturer = 1,
		.iProduct = 2,
		.iSerialNumber = 3,
		.bNumConfigurations = 1,
};

// CDC ACM

/*
 * This notification endpoint isn't implemented. According to CDC spec its
 * optional, but its absence causes a NULL pointer dereference in Linux
 * cdc_acm driver.
 */
static const struct usb_endpoint_descriptor comm_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x83,
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = 16,
	.bInterval = 255,
}};

static const struct usb_endpoint_descriptor data_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x01,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = 64,
	.bInterval = 1,
}, {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x82,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = 64,
	.bInterval = 1,
}};

static const struct {
	struct usb_cdc_header_descriptor header;
	struct usb_cdc_call_management_descriptor call_mgmt;
	struct usb_cdc_acm_descriptor acm;
	struct usb_cdc_union_descriptor cdc_union;
} __attribute__((packed)) cdcacm_functional_descriptors = {
	.header = {
		.bFunctionLength = sizeof(struct usb_cdc_header_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_HEADER,
		.bcdCDC = 0x0110,
	},
	.call_mgmt = {
		.bFunctionLength =
			sizeof(struct usb_cdc_call_management_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_CALL_MANAGEMENT,
		.bmCapabilities = 0,
		.bDataInterface = 1,
	},
	.acm = {
		.bFunctionLength = sizeof(struct usb_cdc_acm_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_ACM,
		.bmCapabilities = 0,
	},
	.cdc_union = {
		.bFunctionLength = sizeof(struct usb_cdc_union_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_UNION,
		.bControlInterface = 0,
		.bSubordinateInterface0 = 1,
	 }
};

static const struct usb_interface_descriptor comm_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 1,
	.bInterfaceClass = USB_CLASS_CDC,
	.bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
	.bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
	.iInterface = 0,

	.endpoint = comm_endp,

	.extra = &cdcacm_functional_descriptors,
	.extralen = sizeof(cdcacm_functional_descriptors)
}};

static const struct usb_interface_descriptor data_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 1,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_DATA,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,

	.endpoint = data_endp,
}};


static const struct usb_iface_assoc_descriptor cdc_assoc = {
	.bLength = USB_DT_INTERFACE_ASSOCIATION_SIZE,
	.bDescriptorType = USB_DT_INTERFACE_ASSOCIATION,
	.bFirstInterface = 0,
	.bInterfaceCount = 2,
	.bFunctionClass = USB_CLASS_CDC,
	.bFunctionSubClass = USB_CDC_SUBCLASS_ACM,
	.bFunctionProtocol = USB_CDC_PROTOCOL_AT,
	.iFunction = 0,
};

// HID

static struct {
	struct usb_hid_descriptor hid_descriptor;
	struct {
		uint8_t bReportDescriptorType;
		uint16_t wDescriptorLength;
	} __attribute__((packed)) hid_report;
} __attribute__((packed)) hid_function = {
	.hid_descriptor = {
		.bLength = sizeof(hid_function),
		.bDescriptorType = USB_DT_HID,
		.bcdHID = 0x0100,
		.bCountryCode = 0,
		.bNumDescriptors = 1,
	},
	.hid_report = {
		.bReportDescriptorType = USB_DT_REPORT,
		.wDescriptorLength = 0,
	}
};

const struct usb_endpoint_descriptor hid_endpoint = {
		.bLength = USB_DT_ENDPOINT_SIZE,
		.bDescriptorType = USB_DT_ENDPOINT,
		.bEndpointAddress = 0x81,
		.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
		.wMaxPacketSize = 64,
		.bInterval = 0x01,
};

const struct usb_interface_descriptor hid_iface = {
		.bLength = USB_DT_INTERFACE_SIZE,
		.bDescriptorType = USB_DT_INTERFACE,
		.bInterfaceNumber = 2,
		.bAlternateSetting = 0,
		.bNumEndpoints = 1,
		.bInterfaceClass = USB_CLASS_HID,
		.bInterfaceSubClass = 1, /* boot */
		.bInterfaceProtocol = 2, /* mouse */
		.iInterface = 0,

		.endpoint = &hid_endpoint,

		.extra = &hid_function,
		.extralen = sizeof(hid_function),
};

static const struct usb_iface_assoc_descriptor hid_assoc = {
	.bLength = USB_DT_INTERFACE_ASSOCIATION_SIZE,
	.bDescriptorType = USB_DT_INTERFACE_ASSOCIATION,
	.bFirstInterface = 2,
	.bInterfaceCount = 1,
	.bFunctionClass = USB_CLASS_HID,
	.bFunctionSubClass = 1,
	.bFunctionProtocol = 2,
	.iFunction = 0,
};

// General

const struct usb_interface ifaces[] = {{
	.num_altsetting = 1,
	.iface_assoc = &cdc_assoc,
	.altsetting = comm_iface,
}, {
	.num_altsetting = 1,
	.altsetting = data_iface,
}, {
	.num_altsetting = 1,
	.iface_assoc = &hid_assoc,
	.altsetting = &hid_iface,
}
};

const struct usb_config_descriptor config = {
	.bLength = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType = USB_DT_CONFIGURATION,
	.wTotalLength = 0,
	.bNumInterfaces = 3,
	.bConfigurationValue = 1,
	.iConfiguration = 0,
	.bmAttributes = 0xC0,
	.bMaxPower = 0x32,

	.interface = ifaces,
};

static const char *usb_strings[] = {
	"Black Sphere Technologies",
	"HID Demo",
	"DEMO",
};

/* Buffer to be used for control requests. */
uint8_t usbd_control_buffer[128];

static enum usbd_request_return_codes hid_control_request(usbd_device *dev, struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
			void (**complete)(usbd_device *, struct usb_setup_data *))
{
	(void)complete;
	(void)dev;

	/* Handle the HID report descriptor. */
	if ((req->bmRequestType == 0x81)
			&& (req->bRequest == USB_REQ_GET_DESCRIPTOR)
			&& (req->wValue == 0x2200)) {
		*buf = (uint8_t *)hid_report_descriptor;
		*len = hid_function.hid_report.wDescriptorLength;

		return USBD_REQ_HANDLED;
	}
	return USBD_REQ_NOTSUPP;
}

static uint8_t precision_feature_rpt[2];
static enum usbd_request_return_codes precision_get_set_report(usbd_device *dev, struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
			void (**complete)(usbd_device *, struct usb_setup_data *))
{
	(void)complete;
	(void)dev;

	// Device capabilities
	if ((req->bRequest == 0x01) && (req->wValue == 0x0302)) {
		precision_feature_rpt[0] = 2;
		precision_feature_rpt[1] = IS_PRESSPAD ? 0x15 : 0x05;

		*buf = precision_feature_rpt;
		*len = 2;

		return USBD_REQ_HANDLED;
	}

	// Certification status
	if ((req->bRequest == 0x01) && (req->wValue == 0x0306)) {
		*buf = (uint8_t *) certification_status_report;
		*len = sizeof(certification_status_report);

		return USBD_REQ_HANDLED;
	}

	// Latency mode
	if ((req->bRequest == 0x09) && (req->wValue == 0x0307)) {
		uint8_t _cmd[sizeof(prec_latency_report) + sizeof(cmd_type)];
		command* cmd = (command*) _cmd;

		cmd->cmdType = CMD_FEATURE_LATENCY_MODE;
		((prec_latency_report*)cmd->data)->mode = (*buf)[1];
		xMessageBufferSend(publicInterface.toDeviceReportBuf, cmd, sizeof(prec_latency_report) + sizeof(cmd_type), 0);

		return USBD_REQ_HANDLED;
	}

	// Input mode
	if ((req->bRequest == 0x09) && (req->wValue == 0x0303)) {
		uint8_t _cmd[sizeof(prec_input_mode_report) + sizeof(cmd_type)];
		command* cmd = (command*) _cmd;

		cmd->cmdType = CMD_FEATURE_INPUT_MODE;
		((prec_input_mode_report*)cmd->data)->mode = (*buf)[1];
		xMessageBufferSend(publicInterface.toDeviceReportBuf, cmd, sizeof(prec_input_mode_report) + sizeof(cmd_type), 0);

		return USBD_REQ_HANDLED;
	}

	// Selective reporting
	if ((req->bRequest == 0x09) && (req->wValue == 0x0305)) {
		uint8_t _cmd[sizeof(prec_selective_reporting_report) + sizeof(cmd_type)];
		command* cmd = (command*) _cmd;

		cmd->cmdType = CMD_FEATURE_SELECTIVE_REPORTING;
		((prec_selective_reporting_report*)cmd->data)->surface = (*buf)[1] & 1;
		((prec_selective_reporting_report*)cmd->data)->sw = ((*buf)[1] >> 1) & 1;
		xMessageBufferSend(publicInterface.toDeviceReportBuf, cmd, sizeof(prec_selective_reporting_report) + sizeof(cmd_type), 0);

		return USBD_REQ_HANDLED;
	}

	return USBD_REQ_NOTSUPP;
}

static enum usbd_request_return_codes cdcacm_control_request(usbd_device *usbd_dev, struct usb_setup_data *req, uint8_t **buf,
		uint16_t *len, void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
	(void)complete;
	(void)buf;
	(void)usbd_dev;

	switch(req->bRequest) {
	case USB_CDC_REQ_SET_CONTROL_LINE_STATE: {
		/*
		 * This Linux cdc_acm driver requires this to be implemented
		 * even though it's optional in the CDC spec, and we don't
		 * advertise it in the ACM functional descriptor.
		 */
		char local_buf[10];
		struct usb_cdc_notification *notif = (void *)local_buf;

		/* We echo signals back to host as notification. */
		notif->bmRequestType = 0xA1;
		notif->bNotification = USB_CDC_NOTIFY_SERIAL_STATE;
		notif->wValue = 0;
		notif->wIndex = 0;
		notif->wLength = 2;
		local_buf[8] = req->wValue & 3;
		local_buf[9] = 0;

		return USBD_REQ_HANDLED;
		}
	case USB_CDC_REQ_SET_LINE_CODING:
		if(*len < sizeof(struct usb_cdc_line_coding))
			return USBD_REQ_NOTSUPP;

		return USBD_REQ_HANDLED;
	}
	return USBD_REQ_NOTSUPP;
}

static enum usbd_request_return_codes class_interface_control_request(usbd_device *usbd_dev, struct usb_setup_data *req, uint8_t **buf,
		uint16_t *len, void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req)) {

	if (req->wIndex == 2) {
		return precision_get_set_report(usbd_dev, req, buf, len, complete);
	} else {
		return cdcacm_control_request(usbd_dev, req, buf, len, complete);
	}
}

// We use a patched version of opencm3, which notifies about a packet only once.
// See: https://github.com/libopencm3/libopencm3/commit/e243ebb7924c2be6cddbad6538378f1e3e13ed6e

static void cdcacm_data_rx_cb(usbd_device *usbd_dev, uint8_t ep)
{
	BaseType_t hp_task_woken = pdFALSE;
	xSemaphoreGiveFromISR(cdc_rx_semaphore, &hp_task_woken);
	if (hp_task_woken) {
		taskYIELD();
	}
}

static void cdcacm_data_tx_cb(usbd_device *usbd_dev, uint8_t ep)
{
	BaseType_t hp_task_woken = pdFALSE;
	xSemaphoreGiveFromISR(cdc_tx_semaphore, &hp_task_woken);
	if (hp_task_woken) {
		taskYIELD();
	}
}

static void cdcacm_data_rx_task(void* arg) {
	char buf[64];
	while(1) {
		xSemaphoreTake(cdc_rx_semaphore, portMAX_DELAY);

		cm_disable_interrupts();
		int len = usbd_ep_read_packet(usbd_dev, 0x01, buf, 64);
		cm_enable_interrupts();

		if (len) {
			xStreamBufferSend(cdc_rx_buf, buf, len, portMAX_DELAY);
		}
	}
}

static void cdcacm_data_tx_task(void* arg) {
	char buf[64];

	// Give the semaphore to self for the first transmission
	xSemaphoreGive(cdc_tx_semaphore);

	while (1) {
		size_t len = xStreamBufferReceive(cdc_tx_buf, buf, 64, portMAX_DELAY);

		xSemaphoreTake(cdc_tx_semaphore, 256);

		cm_disable_interrupts();
		uint16_t ret = usbd_ep_write_packet(usbd_dev, 0x82, buf, len);
		cm_enable_interrupts();

		/*if (!ret) {
			// In case of an error, just ignore it
			xSemaphoreGive(cdc_tx_semaphore);
		}*/
	}
}

static void hid_input_ok(usbd_device *usbd_dev, uint8_t ep)
{
	BaseType_t hp_task_woken = pdFALSE;
	xSemaphoreGiveFromISR(hid_input_semaphore, &hp_task_woken);
	if (hp_task_woken) {
		taskYIELD();
	}
}

void hid_service_task(void* arg) {
	command* cmd = pvPortMalloc(256);

	// Give the semaphore to self for the first transmission
	xSemaphoreGive(hid_input_semaphore);

	while (1) {
		size_t len = xMessageBufferReceive(publicInterface.toHostReportBuf, cmd, 256, portMAX_DELAY);

		if (!len) {
			continue;
		}

		switch (cmd->cmdType) {
			case CMD_INPUT_MOUSE: {
				xSemaphoreTake(hid_input_semaphore, 128);
				prec_mouse_report* inRpt = (prec_mouse_report*) cmd->data;
				struct mouse_report rpt = {
					.id = 1,
					.__pad0 = 0,
					.X = inRpt->X,
					.Y = inRpt->Y,
					.btn1 = inRpt->btn1,
					.btn2 = inRpt->btn2
				};
				cm_disable_interrupts();
				uint16_t ret = usbd_ep_write_packet(usbd_dev, 0x81, &rpt, sizeof(struct mouse_report));
				cm_enable_interrupts();

				if (!ret) {
					// In case of an error, just ignore it
					xSemaphoreGive(hid_input_semaphore);
				}
			} break;
			case CMD_INPUT_PREC: {
				xSemaphoreTake(hid_input_semaphore, 128);
				prec_report* inRpt = (prec_report*) cmd->data;
				struct precision_report rpt = {
					.id = 4,
					.time = inRpt->info.scanTime,
					.contactCnt = inRpt->info.contactCount,
					.btn = inRpt->info.button,
					.__pad1 = 0,
					.__pad2 = 0x80 >> 1,
				};

				for (uint8_t i = 0; i < rpt.contactCnt; ++i) {
					rpt.fingers[i].X = inRpt->fingers[i].X;
					rpt.fingers[i].Y = inRpt->fingers[i].Y;
					rpt.fingers[i].contId = inRpt->fingers[i].ID;
					rpt.fingers[i].valid = inRpt->fingers[i].confidence;
					rpt.fingers[i].tip = inRpt->fingers[i].tip;
					rpt.fingers[i].__pad0 = 0;
				}

				for (uint8_t i = rpt.contactCnt; i < 5; ++i) {
					rpt.fingers[i].X = rpt.fingers[i].Y = rpt.fingers[i].contId =
							rpt.fingers[i].valid = rpt.fingers[i].tip = 0;
					rpt.fingers[i].__pad0 = 0;
				}
				cm_disable_interrupts();
				uint16_t ret = usbd_ep_write_packet(usbd_dev, 0x81, &rpt, sizeof(struct precision_report));
				cm_enable_interrupts();

				if (!ret) {
					// In case of an error, just ignore it
					xSemaphoreGive(hid_input_semaphore);
				}
			} break;
			default: break;
		}
	}
}

size_t cdcacm_write(char const* buf, size_t len) {
	if (usb_ready) {
		return xStreamBufferSend(cdc_tx_buf, buf, len, portMAX_DELAY);
	}
	return 0;
}

size_t cdcacm_read(char * buf, size_t len) {
	if (usb_ready) {
		return xStreamBufferReceive(cdc_rx_buf, buf, len, portMAX_DELAY);
	}
	return 0;
}

static void hid_set_config(usbd_device *dev, uint16_t wValue)
{
	(void)wValue;
	(void)dev;

	// HID
	usbd_ep_setup(dev, 0x81, USB_ENDPOINT_ATTR_INTERRUPT, 64, hid_input_ok);

	usbd_register_control_callback(
					dev,
					USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE,
					USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
					hid_control_request);

	// CDC
	usbd_ep_setup(usbd_dev, 0x01, USB_ENDPOINT_ATTR_BULK, 64, cdcacm_data_rx_cb);
	usbd_ep_setup(usbd_dev, 0x82, USB_ENDPOINT_ATTR_BULK, 64, cdcacm_data_tx_cb);
	usbd_ep_setup(usbd_dev, 0x83, USB_ENDPOINT_ATTR_INTERRUPT, 16, NULL);

	// Both
	usbd_register_control_callback(
				usbd_dev,
				USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
				USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
				class_interface_control_request);

}

void usb_hid_setup_units(struct prec_config_physinfo phys[2])
{
	//base: 35
	bitmover_data data = {
			.count = 1,
			.moves = {
					{8 * 0, 8 * offsetof(struct prec_config_physinfo, logicalMinimum), 8 * 2},
					{8 * 3, 8 * offsetof(struct prec_config_physinfo, logicalMaximum), 8 * 2},
					{8 * 6, 8 * offsetof(struct prec_config_physinfo, physicalMinimum), 8 * 2},
					{8 * 9, 8 * offsetof(struct prec_config_physinfo, physicalMaximum), 8 * 2},
					{8 * 12, 8 * offsetof(struct prec_config_physinfo, unitExponent), 4},
					{8 * 14, 8 * offsetof(struct prec_config_physinfo, unit), 4},
			}
	};
	bitmover_move(&data, &phys[0], hid_report_descriptor_finger + 35);
	bitmover_move(&data, &phys[1], hid_report_descriptor_finger + 59);

	uint8_t* cur = hid_report_descriptor;
	memcpy(cur, hid_report_descriptor_prefix, sizeof(hid_report_descriptor_prefix));
	cur += sizeof(hid_report_descriptor_prefix);
	for (int i = 0; i < 5; ++i) {
		memcpy(cur, hid_report_descriptor_finger, sizeof(hid_report_descriptor_finger));
		cur += sizeof(hid_report_descriptor_finger);
	}
	memcpy(cur, hid_report_descriptor_suffix, sizeof(hid_report_descriptor_suffix));
	cur += sizeof(hid_report_descriptor_suffix);

	hid_function.hid_report.wDescriptorLength = cur - hid_report_descriptor;
}

void usb_init()
{
	//rcc_clock_setup_in_hsi_out_48mhz();
	rcc_clock_setup_in_hse_8mhz_out_72mhz();

	rcc_periph_clock_enable(RCC_GPIOA);
	/*
	 * This is a somewhat common cheap hack to trigger device re-enumeration
	 * on startup.  Assuming a fixed external pullup on D+, (For USB-FS)
	 * setting the pin to output, and driving it explicitly low effectively
	 * "removes" the pullup.  The subsequent USB init will "take over" the
	 * pin, and it will appear as a proper pullup to the host.
	 * The magic delay is somewhat arbitrary, no guarantees on USBIF
	 * compliance here, but "it works" in most places.
	 */
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ,
		GPIO_CNF_OUTPUT_PUSHPULL, GPIO12);
	gpio_clear(GPIOA, GPIO12);
	for (unsigned i = 0; i < 800000; i++) {
		__asm__("nop");
	}

	cdc_rx_buf = xStreamBufferCreate(128, 0);
	cdc_tx_buf = xStreamBufferCreate(128, 0);
	cdc_tx_semaphore = xSemaphoreCreateBinary();
	cdc_rx_semaphore = xSemaphoreCreateBinary();
	hid_input_semaphore = xSemaphoreCreateBinary();

	usbd_dev = usbd_init(&st_usbfs_v1_usb_driver, &dev_descr, &config, usb_strings, 3, usbd_control_buffer, sizeof(usbd_control_buffer));
	usbd_register_set_config_callback(usbd_dev, hid_set_config);

	nvic_set_priority(NVIC_USB_HP_CAN_TX_IRQ, configMAX_SYSCALL_INTERRUPT_PRIORITY);
	nvic_set_priority(NVIC_USB_LP_CAN_RX0_IRQ, configMAX_SYSCALL_INTERRUPT_PRIORITY);
	nvic_set_priority(NVIC_USB_WAKEUP_IRQ, configMAX_SYSCALL_INTERRUPT_PRIORITY);
	nvic_enable_irq(NVIC_USB_HP_CAN_TX_IRQ);
	nvic_enable_irq(NVIC_USB_LP_CAN_RX0_IRQ);
	nvic_enable_irq(NVIC_USB_WAKEUP_IRQ);

	xTaskCreate(cdcacm_data_tx_task, "cdc_send", 100, NULL, configMAX_PRIORITIES - 1, NULL);
	xTaskCreate(cdcacm_data_rx_task, "cdc_rcv", 100, NULL, configMAX_PRIORITIES - 1, NULL);
	xTaskCreate(hid_service_task, "hid_service", 100, NULL, configMAX_PRIORITIES - 1, NULL);

	usb_ready = true;
}

void usb_wakeup_isr()
{
	usb_lp_can_rx0_isr();
}

void usb_hp_can_tx_isr() {
	usb_lp_can_rx0_isr();
}

void usb_lp_can_rx0_isr(){
	if (usbd_dev) {
		usbd_poll(usbd_dev);
	}
}

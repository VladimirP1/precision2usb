#pragma once


struct precision_report {
  uint8_t id : 8;
  struct {
	  uint8_t valid : 1;
	  uint8_t tip : 1;
	  uint8_t __pad0 : 2;
	  uint8_t contId : 4;
	  uint16_t X : 16;
	  uint16_t Y : 16;
  } fingers[5];
  uint16_t time : 16;
  uint8_t contactCnt : 8;
  uint8_t btn : 1;
  uint8_t __pad2 : 7;
  uint16_t __pad1 : 16;
} __attribute__((packed));

struct mouse_report {
	uint8_t id : 8;
	uint8_t btn1 : 1;
	uint8_t btn2 : 1;
	uint8_t __pad0 : 6;
	int8_t X : 8;
	int8_t Y : 8;
} __attribute__((packed));

/*static precision_report touch_report = {
  .id = 4,
  .tip = 1,
  .valid = 1,
  .X = 7,
  .Y = 5,
  .time = 0x3333,
  .contactCnt = 1,
  .contId = 0,
  .btn = 0,
  .__pad0 = 0,
  .__pad1 = 0,
  .__pad2 = 0x80 >> 1
};*/

/*void sys_tick_handler(void) {
	if (!rptMode) {
		static int x = 0;
		static int dir = 1;
		uint8_t mouse_report[] = { 1, 0, 0, 0, 0, 0, 0, 0, 0 };

		mouse_report[2] = dir;
		x += dir;
		if (x > 2)
			dir = -dir;
		if (x < -2)
			dir = -dir;

		usbd_ep_write_packet(usbd_dev, 0x81, mouse_report, sizeof(mouse_report));
	} else {
		static double ang = 0;
		ang += 0.005;

		touch_report.contId = 0;
		touch_report.valid = 1;
		touch_report.tip = 1;
		touch_report.X = 1500 + sin(ang) * 20;
		touch_report.Y = 1000 + cos(ang) * 20;
		//touch_report.btn = (touch_report.time % 10000 > 5000);
		touch_report.time += 17 * 2; //167
		touch_report.contactCnt = 1;//(touch_report.time % 10000) > 5000;
		usbd_ep_write_packet(usbd_dev, 0x81, &touch_report, sizeof(touch_report));
	}
}*/

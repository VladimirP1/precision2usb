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
  } __attribute__((packed)) fingers[5];
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

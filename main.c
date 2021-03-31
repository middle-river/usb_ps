/*
  USB Power Supply
  2021-03-26
  T. Nakagawa

  FET: PD0
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <string.h>
#include <stdint.h>
#include <util/delay.h>
#include "usbdrv.h"

PROGMEM const char usbHidReportDescriptor[22] = {
    0x06, 0x00, 0xff,              // USAGE_PAGE (Generic Desktop)
    0x09, 0x01,                    // USAGE (Vendor Usage 1)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x09, 0x00,                    //   USAGE (Undefined)
    0xb2, 0x02, 0x01,              //   FEATURE (Data,Var,Abs,Buf)
    0xc0                           // END_COLLECTION
};

static uint8_t request;

void sw_on() {
  DDRD |= _BV(0);
}

void sw_off() {
  DDRD &= ~_BV(0);
}

uint8_t usbFunctionWrite(uint8_t *data, uint8_t len) {
  if (len == 1) {
    memcpy(&request, data, len);
    if (request != 0) {
      sw_on();
    } else {
      sw_off();
    }
    return 1;
  } else {
    return -1;
  }
}

usbMsgLen_t usbFunctionSetup(uint8_t data[8]) {
  usbRequest_t *rq = (usbRequest_t *)data;

  if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
    if (rq->bRequest == USBRQ_HID_GET_REPORT) {
      usbMsgPtr = (usbMsgPtr_t)&request;
      return 1;
    } else if (rq->bRequest == USBRQ_HID_SET_REPORT) {
      return USB_NO_MSG;
    }
  }
  return 0;
}

int main(void) {
  uint8_t i;

  // Initialize the FET gate.
  DDRD &= ~_BV(0);
  PORTD &= ~_BV(0);

  wdt_enable(WDTO_1S);
  usbInit();
  usbDeviceDisconnect();
  for (i = 0; i < 250; i++) {  // Wait for 250ms.
    wdt_reset();
    _delay_ms(1);
  }
  usbDeviceConnect();
  sei();
  for(; ; ){
    wdt_reset();
    usbPoll();
  }
  return 0;
}

#pragma once

#include <libusb-1.0/libusb.h>

libusb_device_handle *usb_init(void);

void usb_cleanup(libusb_device_handle *handle);

void usb_send(libusb_device_handle *handle, const uint8_t *buf, const uint8_t buf_len);

void usb_recv(libusb_device_handle *handle, uint8_t *buf, const uint8_t buf_len);

#pragma once

#include <libusb-1.0/libusb.h>

/*
 * Initialize the hyperFIDO usb device's libusb handle.
 * Returns 0 on success, -1 on failure.
 * Error message is obtainable through the log module.
 */
int usb_init(libusb_device_handle **handle);

/*
 * Release the hyperFIDO usb device's libusb handle.
 * Returns 0 on success, -1 on failure.
 * Error message is obtainable through the log module.
 */
int usb_cleanup(libusb_device_handle *handle);

/*
 * Send the given data to device as an interrupt transfer.
 * Returns 0 on success, -1 on failure.
 * Error message is obtainable through the log module.
 */
int usb_send(libusb_device_handle *handle, const uint8_t *buf, const uint8_t buf_len);

/*
 * Receive data from the device as an interrupt transfer.
 * Returns 0 on success, -1 on failure.
 * Error message is obtainable through the log module.
 */
int usb_recv(libusb_device_handle *handle, uint8_t *buf, const uint8_t buf_len);

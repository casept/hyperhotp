#pragma once

#include <libusb-1.0/libusb.h>
#include <stdbool.h>

#include "u2fhid.h"
#include "usb.h"

#define HYPERHOTP_SERIAL_LEN     8
#define HYPERHOTP_SEED_LEN_ASCII 40
#define HYPERHOTP_SEED_LEN_HEX   20

/*
 * Initializes the device, the protocol and allocates a U2FHID channel ID.
 * Returns 0 on success, -1 on failure.
 * Error message can be obtained from the log module.
 */
int hyperhotp_init(libusb_device_handle **handle, FIDOCID cid);

/*
 * Checks whether the device has been programmed, and returns the HOTP key's serial if yes.
 * Returns 1 if programmed, 0 if not programmed, -1 on failure.
 * Error message can be obtained from the log module.
 */
int hyperhotp_check_programmed(libusb_device_handle *handle, const FIDOCID cid, char serial[HYPERHOTP_SERIAL_LEN]);

/*
 * Resets the device, clearing any HOTP data.
 * Returns 0 on success, -1 on failure.
 * Error message can be obtained from the log module.
 */
int hyperhotp_reset(libusb_device_handle *handle, const FIDOCID cid);

/*
 * Programs the device.
 * Returns 0 on success, -1 on failure.
 * Error message can be obtained from the log module.
 */
int hyperhotp_program(libusb_device_handle *handle, const FIDOCID cid, const bool is_8_char_code,
                      const char serial[HYPERHOTP_SERIAL_LEN], const char seed[HYPERHOTP_SEED_LEN_ASCII]);

/*
 * Cleans up resources.
 * Returns 0 on success, -1 on failure.
 * Error message can be obtained from the log module.
 */
int hyperhotp_cleanup(libusb_device_handle *handle);

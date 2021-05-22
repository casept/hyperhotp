#include "hyperhotp.h"

#include <libusb-1.0/libusb.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "log.h"
#include "u2fhid.h"

void hyperhotp_init(libusb_device_handle *handle, FIDOCID cid) { fido_alloc_channel(handle, cid); }

// This seems to be a magic sequence the Windows client executes before every transaction.
static void hyperhotp_magic(libusb_device_handle *handle, const FIDOCID cid) {
    // Ping
    log_debug("Sending ping");
    // No idea why this is so large or what the data means
    const uint8_t data[14] = {0x00, 0xa4, 0x04, 0x00, 0x09, 0xd1, 0x56, 0x00, 0x01, 0x32, 0x83, 0x26, 0x01, 0x01};
    const FIDOInitPacket ping = fido_craft_packet(cid, U2FHID_ADPU_RAW, 14, data);
    fido_send_packet(handle, ping);

    // Pong
    log_debug("Waiting for pong");
    const FIDOInitPacket pong = fido_recv_packet(handle);
    if (fido_is_error_packet(pong)) {
        log_fatal("Failed to send ping: Got error response back");
    }
    log_debug("Pong");
}

bool hyperhotp_check_programmed(libusb_device_handle *handle, const FIDOCID cid, char serial[HYPERHOTP_SERIAL_LEN]) {
    hyperhotp_magic(handle, cid);

    // Send a packet to get programmed serial
    const uint8_t data[4] = {0x00, 0xe6, 0x00, 0x00};
    const FIDOInitPacket req = fido_craft_packet(cid, U2FHID_ADPU_RAW, 4, data);
    fido_send_packet(handle, req);

    // Parse response
    FIDOInitPacket resp = fido_recv_packet(handle);
    if (fido_is_error_packet(resp)) {
        log_fatal("Failed to check whether key is programmed: Got error response back");
    }
    // Seems like this byte is always set when a key is programmed
    bool key_programmed = false;
    switch (resp.data[11]) {
        case 0x00:
            key_programmed = false;
            break;
        case 0x90:
            key_programmed = true;
            break;
        default:
            log_fatal("Failed to check whether key is programmed: Encountered unexpected value in response");
            break;
    }
    if (key_programmed) {
        memcpy(serial, resp.data + 3, HYPERHOTP_SERIAL_LEN * sizeof(uint8_t));  // NOLINT (GCC doesn't support _s)
    }
    return key_programmed;
}

static bool hyperhotp_transaction_succeeded(const FIDOInitPacket resp) {
    if (resp.data[0] == 0x69) {
        return false;
    } else if (resp.data[0] == 0x90) {
        return true;
    } else {
        log_fatal("Unknown bytes in response from device when reading whether reset succeeded");
    }
    return false;
}

bool hyperhotp_reset(libusb_device_handle *handle, const FIDOCID cid) {
    char serial[HYPERHOTP_SERIAL_LEN];

    if (!hyperhotp_check_programmed(handle, cid, serial)) {
        log_debug("Device is not programmed, nothing to reset");
        return false;
    } else {
        log_debug("Device is programmed, proceeding with reset");
    }

    // Send reset request
    const uint8_t data[4] = {0x00, 0x07, 0x00, 0x00};
    const FIDOInitPacket req = fido_craft_packet(cid, U2FHID_ADPU_RAW, 4, data);
    fido_send_packet(handle, req);

    // Check response for success
    const FIDOInitPacket resp = fido_recv_packet(handle);
    if (!hyperhotp_transaction_succeeded(resp) || fido_is_error_packet(resp)) {
        log_fatal("Failed to reset device: Device reported failure (perhaps you didn't push the button?)");
    } else {
        if (hyperhotp_check_programmed(handle, cid, serial)) {
            log_fatal("Failed to reset device: Device reported successful reset, but device is not actually reset");
        } else {
            return true;
        }
    }
    return false;
}

static bool ascii_is_hex(const char x) {
    return ((x >= '0' && x <= '9') || (x >= 'a' && x <= 'f') || (x >= 'A' && x <= 'F'));
}

static uint8_t ascii_2_hex(const char a_char, const char b_char) {
    const uint8_t a = (uint8_t)a_char;
    const uint8_t b = (uint8_t)b_char;

    uint8_t h = 0;
    if (a >= '0' && a <= '9') {
        h = ((a - '0') << 4);
    } else if (a >= 'a' && a <= 'f') {
        h = ((a - 'a' + 10) << 4);
    } else if (a >= 'A' && a <= 'F') {
        h = ((a - 'A' + 10) << 4);
    } else {
        log_fatal("Failed to convert char to hex: Out of range");
    }

    if (b >= '0' && b <= '9') {
        h = h | (b - '0');
    } else if (b >= 'a' && b <= 'f') {
        h = h | (b - 'a' + 10);
    } else if (b >= 'A' && b <= 'F') {
        h = h | (b - 'A' + 10);
    } else {
        log_fatal("Failed to convert char to hex: Out of range");
    }
    return h;
}

void hyperhotp_program(libusb_device_handle *handle, const FIDOCID cid, const bool is_8_char_code,
                       const char serial[HYPERHOTP_SERIAL_LEN], const char seed[HYPERHOTP_SEED_LEN_ASCII]) {
    char curr_serial[HYPERHOTP_SERIAL_LEN];
    if (hyperhotp_check_programmed(handle, cid, curr_serial)) {
        log_fatal("Failed to program device: Device is already programmed. Please reset and try again.");
    }

    // Check whether seed is valid
    for (size_t i = 0; i < HYPERHOTP_SEED_LEN_ASCII; i++) {
        if (!ascii_is_hex(seed[i])) {
            log_fatal("Failed to program device: Seed contains non-hex characters");
        }
    }

    // Convert seed from ASCII to hex
    uint8_t hex_seed[HYPERHOTP_SEED_LEN_HEX] = {0};
    size_t j = 0;
    for (size_t i = 0; i < HYPERHOTP_SEED_LEN_HEX; i++) {
        hex_seed[i] = ascii_2_hex(seed[j], seed[j + 1]);
        j += 2;
    }

    // Send programming request
    uint8_t data[0x28] = {
        0x00, 0x09, 0x00, 0x00, 0x23, 0x53, 0x16, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x51, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };  // All non-0 fields are magic
    if (is_8_char_code) {
        data[9] = 0x08;
    } else {
        data[9] = 0x06;
    }
    memcpy(data + 10, hex_seed, HYPERHOTP_SEED_LEN_HEX);  // NOLINT (GCC doesn't support _s)
    memcpy(data + 32, serial, HYPERHOTP_SERIAL_LEN);      // NOLINT (GCC doesn't support _s)
    const FIDOInitPacket req = fido_craft_packet(cid, U2FHID_ADPU_RAW, 0x28, data);
    fido_send_packet(handle, req);

    // Check whether programming succeeded
    const FIDOInitPacket resp = fido_recv_packet(handle);
    char new_serial[HYPERHOTP_SERIAL_LEN] = {0};
    if (!hyperhotp_transaction_succeeded(resp) || fido_is_error_packet(resp)) {
        log_fatal("Failed to program device: Device reported failure (perhaps you didn't push the button?)");
    } else if (!hyperhotp_check_programmed(handle, cid, new_serial)) {
        log_fatal(
            "Failed to program device: Device reported successful programming, but device is not actually programmed");
    } else if (strncmp(serial, new_serial, HYPERHOTP_SERIAL_LEN) != 0) {
        log_fatal(
            "Failed to program device: Device reported successful programming, but serial number doesn't match the one "
            "programmed");
    } else {
        printf("Programming complete!\n");
    }
}

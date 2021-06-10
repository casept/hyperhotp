#pragma once

#include <libusb-1.0/libusb.h>
#include <stdbool.h>
#include <stdint.h>

#define FIDO_PACKET_SIZE     64
#define FIDO_CID_LEN         4
#define FIDO_PACKET_DATA_LEN 57

#define U2FHID_INIT      0x86
#define U2FHID_ERROR     0xBF
#define U2FHID_ADPU_RAW  0x83
#define U2FHID_NONCE_LEN 8

// FIDO channel ID
typedef uint8_t FIDOCID[4];

// FIDO message initial packet
typedef struct {
    // Channel ID
    FIDOCID cid;
    // Command ID
    uint8_t cmd;
    // Payload length (high part)
    uint8_t bcnth;
    // Payload length (low part)
    uint8_t bcntl;
    // Payload data
    uint8_t data[FIDO_PACKET_DATA_LEN];
} FIDOInitPacket;

FIDOInitPacket fido_craft_packet(const FIDOCID cid, const uint8_t cmd, const uint8_t data_len, const uint8_t *data);

int fido_send_packet(libusb_device_handle *handle, const FIDOInitPacket packet);

int fido_recv_packet(libusb_device_handle *handle, FIDOInitPacket *packet);

int fido_alloc_channel(libusb_device_handle *handle, FIDOCID cid);

bool fido_is_error_packet(const FIDOInitPacket packet);

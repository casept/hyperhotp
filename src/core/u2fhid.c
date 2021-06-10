#include "u2fhid.h"

#include <libusb-1.0/libusb.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "log.h"
#include "usb.h"

static const FIDOCID U2FHID_BROADCAST_CID = {0xff, 0xff, 0xff, 0xff};

int fido_send_packet(libusb_device_handle *handle, const FIDOInitPacket packet) {
    uint8_t buf[FIDO_PACKET_SIZE];
    memset(buf, 0, FIDO_PACKET_SIZE * sizeof(uint8_t));  // NOLINT (GCC doesn't support _s)

    memcpy(buf + 0, packet.cid, FIDO_CID_LEN);  // NOLINT (GCC doesn't support _s)
    buf[4] = packet.cmd;
    buf[5] = packet.bcnth;
    buf[6] = packet.bcntl;
    memcpy(buf + 7, packet.data, FIDO_PACKET_DATA_LEN);  // NOLINT (GCC doesn't support _s)

    return usb_send(handle, buf, FIDO_PACKET_SIZE);
}

int fido_recv_packet(libusb_device_handle *handle, FIDOInitPacket *packet) {
    memset(packet, 0, sizeof(FIDOInitPacket));
    uint8_t buf[FIDO_PACKET_SIZE];
    memset(buf, 0, FIDO_PACKET_SIZE * sizeof(uint8_t));  // NOLINT (GCC doesn't support _s)

    int err = usb_recv(handle, buf, FIDO_PACKET_SIZE);
    if (err != 0) {
        return -1;
    }
    memcpy(packet->cid, buf + 0, FIDO_CID_LEN);  // NOLINT (GCC doesn't support _s)
    packet->cmd = buf[4];
    packet->bcnth = buf[5];
    packet->bcntl = buf[6];
    memcpy(packet->data, buf + 7, FIDO_PACKET_DATA_LEN);  // NOLINT (GCC doesn't support _s)

    return 0;
}

FIDOInitPacket fido_craft_packet(const FIDOCID cid, const uint8_t cmd, const uint8_t data_len, const uint8_t *data) {
    if (data_len > FIDO_PACKET_DATA_LEN) {
        log_fatal("Failed to craft U2FHID packet: Data too long");
    }
    FIDOInitPacket packet = {0};
    packet.cmd = cmd;
    packet.bcntl = data_len;
    memcpy(packet.cid, cid, FIDO_CID_LEN);
    memcpy(packet.data, data, data_len);
    return packet;
}

bool fido_is_error_packet(const FIDOInitPacket packet) { return packet.cmd == U2FHID_ERROR; }

int fido_alloc_channel(libusb_device_handle *handle, FIDOCID cid) {
    log_debug("Allocating channel");
    // Craft alloc request packet
    // The Windows programmer seems to always use this nonce
    const uint8_t chosen_by_fair_dice_roll_guaranteed_to_be_random[U2FHID_NONCE_LEN] = {0xcd, 0x4b, 0x74, 0xbd,
                                                                                        0x89, 0x5e, 0xa5, 0x00};
    const FIDOInitPacket req = fido_craft_packet(U2FHID_BROADCAST_CID, U2FHID_INIT, U2FHID_NONCE_LEN,
                                                 chosen_by_fair_dice_roll_guaranteed_to_be_random);
    int err = fido_send_packet(handle, req);
    if (err != 0) {
        return -1;
    }

    // Parse response packet
    FIDOInitPacket resp;
    err = fido_recv_packet(handle, &resp);
    if (err != 0) {
        return -1;
    }
    // Check packet type
    if (resp.cmd != U2FHID_INIT) {
        log_error("Failed to allocate U2FHID channel: Unexpected response command type");
    }
    // Check nonce
    if (strncmp((const char *)resp.data, (const char *)chosen_by_fair_dice_roll_guaranteed_to_be_random,
                U2FHID_NONCE_LEN) == 0) {
        log_debug("Nonce checks out");
    } else {
        log_error("Failed to allocate U2FHID channel: Nonce did not match");
    }
    // Extract CID
    memcpy(cid, resp.data + U2FHID_NONCE_LEN, FIDO_CID_LEN * sizeof(uint8_t));
    log_debug("Allocated channel");
    return 0;
}

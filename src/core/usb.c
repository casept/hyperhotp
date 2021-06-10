#include "usb.h"

#include <libusb-1.0/libusb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "log.h"

// There are revisions with different vendor IDs floating around
#define HYPERSECU_VID_1 0x2ccf
#define HYPERSECU_VID_2 0x096e

// Not sure whether this list is exhaustive
#define HYPERFIDO_PID 0x0854

// Might be better to discover this dynamically
#define HYPERHOTP_IFACE_NUM    1
#define HYPERHOTP_IN_ENDPOINT  0x83
#define HYPERHOTP_OUT_ENDPOINT 0x04

static bool is_wanted_device(libusb_device *dev) {
    struct libusb_device_descriptor desc = {0};
    int err = libusb_get_device_descriptor(dev, &desc);
    if (err != 0) {
        log_fatal("Could not get descriptor for device");
    }

    // TODO: Support all VIDs/PIDs
    if (desc.idVendor != HYPERSECU_VID_1) {
        return false;
    }
    if (desc.idProduct != HYPERFIDO_PID) {
        return false;
    }
    return true;
}

static int usb_find_and_init_device(libusb_device_handle **handle) {
    // discover devices
    libusb_device **list;
    ssize_t cnt = libusb_get_device_list(NULL, &list);
    ssize_t i = 0;
    int err = 0;
    if (cnt < 0) {
        log_error("Could not get device list from libusb");
        return -1;
    }

    libusb_device *found = NULL;
    for (i = 0; i < cnt; i++) {
        libusb_device *device = list[i];
        // TODO: handle multiple devices being present (cmd arg or menu to choose)
        if (is_wanted_device(device)) {
            if (found != NULL) {
                log_error("More than one eligible device detected! Please unplug all but one and try again");
                return -1;
            }
            log_debug("Found device");
            found = device;
        }
    }

    if (found != NULL) {
        // Open device
        err = libusb_open(found, handle);
        if (err != 0) {
            log_error_libusb("Failed to open device", err);
            return -1;
        }

        // Try to detach kernel driver
        err = libusb_set_auto_detach_kernel_driver(*handle, true);
        if (err != 0) {
            log_debug("Failed to detach kernel driver. On platforms where this is unsupported that's not a problem.");
        }

        // Claim FIDO interface from kernel
        err = libusb_claim_interface(*handle, HYPERHOTP_IFACE_NUM);
        if (err != 0) {
            log_error_libusb("Failed to claim device from kernel", err);
            return -1;
        }

        libusb_free_device_list(list, true);
        return 0;
    } else {
        libusb_free_device_list(list, true);
        log_error("Device could not be found, perhaps it's not plugged in?");
        return -1;
    }
}

int usb_init(libusb_device_handle **handle) {
    int err = libusb_init(NULL);
    if (err != 0) {
        log_error_libusb("Failed to init libusb", err);
        return -1;
    }

    // Configure debugging
#ifdef DEBUG
    err = libusb_set_option(NULL, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_WARNING);
    if (err != 0) {
        log_error_libusb("Failed to set libusb log level", err);
        return -1;
    }
    libusb_set_log_cb(NULL, log_libusb_callback, LIBUSB_LOG_CB_GLOBAL);
#endif

    // Find device
    err = usb_find_and_init_device(handle);
    if (err != 0) {
        return -1;
    }
    return 0;
}

int usb_send(libusb_device_handle *handle, const uint8_t *buf, const uint8_t buf_len) {
    log_sent(buf, buf_len);
    int transferred = 0;
    // TODO: Timeout
    int err = libusb_interrupt_transfer(handle, HYPERHOTP_OUT_ENDPOINT, (unsigned char *)buf, buf_len, &transferred,
                                        0);  // NOLINT (This is a send, so libusb doesn't write)
    if (err != 0) {
        log_error_libusb("Failed to perform interrupt transfer", err);
        return -1;
    }
    // TODO: Build reliable transmission abstraction if needed
    if (transferred != buf_len) {
        log_error("Failed to perform interrupt transfer: Not all data got sent");
        return -1;
    }
    return 0;
}

int usb_recv(libusb_device_handle *handle, uint8_t *buf, const uint8_t buf_len) {
    int transferred = 0;
    // TODO: Timeout
    int err = libusb_interrupt_transfer(handle, HYPERHOTP_IN_ENDPOINT, buf, buf_len, &transferred, 0);
    if (err != 0) {
        log_error_libusb("Failed to perform interrupt transfer", err);
        return -1;
    }
    // TODO: Build reliable transmission abstraction if needed
    if (transferred != buf_len) {
        log_error("Failed to perform interrupt transfer: Not all data got received");
    }
    log_received(buf, buf_len);
    return 0;
}

int usb_cleanup(libusb_device_handle *handle) {
    int err = libusb_release_interface(handle, HYPERHOTP_IFACE_NUM);
    if (err != 0) {
        log_error_libusb("Failed to release device interface", err);
        return -1;
    }
    libusb_close(handle);
    libusb_exit(NULL);
    return 0;
}

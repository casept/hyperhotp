#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../core/hyperhotp.h"
#include "../core/log.h"
#include "../core/u2fhid.h"
#include "../core/usb.h"
#include "cli.h"

static void check(libusb_device_handle *handle, const FIDOCID cid) {
    char serial[HYPERHOTP_SERIAL_LEN] = {0};
    const bool programmed = hyperhotp_check_programmed(handle, cid, serial);
    if (programmed) {
        printf("Device is programmed, serial: %.8s\n", serial);
    } else {
        printf("Device is not programmed\n");
    }
}

static void reset(libusb_device_handle *handle, const FIDOCID cid) {
    if (hyperhotp_reset(handle, cid)) {
        printf("Reset complete!\n");
    } else {
        log_fatal("reset failed, maybe device is already reset...\n");
    }
}

static void program(libusb_device_handle *handle, const FIDOCID cid, const CLIConfig cfg) {
    hyperhotp_program(handle, cid, cfg.is_8_char_code, cfg.serial, cfg.seed);
}

int main(int argc, char *argv[]) {
    libusb_device_handle *handle = usb_init();
    FIDOCID cid;
    hyperhotp_init(handle, cid);

    const CLIConfig cfg = cli_parse(argc, argv);

    switch (cfg.action) {
        case CLI_ACTION_INVALID:
        case CLI_ACTION_HELP:
            cli_print_help(argv[0]);
            usb_cleanup(handle);
            exit(EXIT_FAILURE);
            break;
        case CLI_ACTION_CHECK:
            check(handle, cid);
            break;
        case CLI_ACTION_RESET:
            reset(handle, cid);
            break;
        case CLI_ACTION_PROGRAM:
            program(handle, cid, cfg);
            break;
        default:
            log_fatal("Unknown CLI action, this is a bug");
            break;
    }

    usb_cleanup(handle);
    return EXIT_SUCCESS;
}

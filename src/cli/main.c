#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../core/hyperhotp.h"
#include "../core/log.h"
#include "../core/u2fhid.h"
#include "cli.h"

static void check(libusb_device_handle *handle, const FIDOCID cid) {
    char serial[HYPERHOTP_SERIAL_LEN] = {0};
    const int programmed = hyperhotp_check_programmed(handle, cid, serial);
    if (programmed == 1) {
        printf("Device is programmed, serial: %.8s\n", serial);
    } else if (programmed == 0) {
        printf("Device is not programmed\n");
    } else {
        char *err_str = log_get_last_error_string();
        fprintf(stderr, "Failed to check whether device is programmed, error message: %s\n", err_str);
        log_free_error_string(err_str);
        exit(EXIT_FAILURE);
    }
}

static void reset(libusb_device_handle *handle, const FIDOCID cid) {
    if (hyperhotp_reset(handle, cid) == 0) {
        printf("Reset complete!\n");
    } else {
        char *err_str = log_get_last_error_string();
        fprintf(stderr, "Failed to reset device, error message: %s\n", err_str);
        log_free_error_string(err_str);
        exit(EXIT_FAILURE);
    }
}

static void program(libusb_device_handle *handle, const FIDOCID cid, const CLIConfig cfg) {
    const int err = hyperhotp_program(handle, cid, cfg.is_8_char_code, cfg.serial, cfg.seed);
    if (err != 0) {
        char *err_str = log_get_last_error_string();
        fprintf(stderr, "Failed to program device, error message: %s\n", err_str);
        log_free_error_string(err_str);
        exit(EXIT_FAILURE);
    } else {
        printf("Programming complete!\n");
    }
}

int main(int argc, const char *argv[]) {
    libusb_device_handle *handle = NULL;
    FIDOCID cid;
    int err = hyperhotp_init(&handle, cid);
    if (err != 0) {
        char *msg = log_get_last_error_string();
        log_fatal(msg);
        log_free_error_string(msg);
    }

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

    hyperhotp_cleanup(handle);
    return EXIT_SUCCESS;
}

#include "cli.h"

#include <libusb.h>
#include <stdio.h>
#include <string.h>

#include "../core/hyperhotp.h"

CLIConfig cli_parse(const int argc, const char* argv[]) {
    // TODO: Use getopt or something to make this order-independent
    CLIConfig conf = {0};
    // Have an action?
    if (argc < 2) {
        conf.action = CLI_ACTION_INVALID;
        return conf;
    }
    // Action is valid?
    if (strncmp(argv[1], "help", 100) == 0) {
        conf.action = CLI_ACTION_HELP;
    } else if (strncmp(argv[1], "check", 100) == 0) {
        conf.action = CLI_ACTION_CHECK;
    } else if (strncmp(argv[1], "reset", 100) == 0) {
        conf.action = CLI_ACTION_RESET;
    } else if (strncmp(argv[1], "program", 100) == 0) {
        conf.action = CLI_ACTION_PROGRAM;
    } else {
        conf.action = CLI_ACTION_INVALID;
    }

    // Get arguments for programming
    if (conf.action == CLI_ACTION_PROGRAM) {
        // We need everything to be specified
        // If not, assume the default key length (6 bytes)
        bool have_explicit_length = false;
        conf.is_8_char_code = false;
        switch (argc) {
            case 4:
                have_explicit_length = false;
                break;
            case 5:
                have_explicit_length = true;
                break;
            default:
                conf.action = CLI_ACTION_INVALID;
                return conf;
        }

        size_t arg_offset = 2;
        if (have_explicit_length) {
            // Check whether a key length has been explictly set.
            if (strncmp(argv[arg_offset], "-6", strlen("-6")) == 0) {
                conf.is_8_char_code = false;
            } else if (strncmp(argv[arg_offset], "-8", strlen("-8")) == 0) {
                conf.is_8_char_code = true;
            } else {
                conf.action = CLI_ACTION_INVALID;
                return conf;
            }
            // Other arguments shift over by 1 if key length is passed
            arg_offset++;
        }

        // Obtain serial
        if (strlen(argv[arg_offset]) != HYPERHOTP_SERIAL_LEN) {
            conf.action = CLI_ACTION_INVALID;
        }
        strncpy(conf.serial, argv[arg_offset], HYPERHOTP_SERIAL_LEN);
        arg_offset++;

        // Obtain seed
        if (strlen(argv[arg_offset]) != HYPERHOTP_SEED_LEN_ASCII) {
            conf.action = CLI_ACTION_INVALID;
        }
        strncpy(conf.seed, argv[arg_offset], HYPERHOTP_SEED_LEN_ASCII);
        arg_offset++;
    }

    return conf;
}

void cli_print_help(const char* binary_path) {
    fprintf(stderr, "Usage: %s [help|check|reset|program] [-68] <8-character serial number> <40-character hex seed>\n",
            binary_path);
}

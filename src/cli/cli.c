#include "cli.h"

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
        if (argc != 5) {
            conf.action = CLI_ACTION_INVALID;
            return conf;
        }

        // Obtain serial
        if (strlen(argv[2]) != HYPERHOTP_SERIAL_LEN) {
            conf.action = CLI_ACTION_INVALID;
        }
        strncpy(conf.serial, argv[2], HYPERHOTP_SERIAL_LEN);

        // Obtain seed
        if (strlen(argv[3]) != HYPERHOTP_SEED_LEN_ASCII) {
            conf.action = CLI_ACTION_INVALID;
        }
        strncpy(conf.seed, argv[3], HYPERHOTP_SEED_LEN_ASCII);

        // Obtain code length
        if (strncmp(argv[4], "false", strlen("false")) == 0) {
            conf.is_8_char_code = false;
        } else if (strncmp(argv[4], "true", strlen("true")) == 0) {
            conf.is_8_char_code = true;
        } else {
            conf.action = CLI_ACTION_INVALID;
        }
    }

    return conf;
}

void cli_print_help(const char* binary_path) {
    fprintf(stderr,
            "Usage: %s [help|check|reset|program] <8-character serial number> <40-character hex seed> <whether to "
            "generate 6-character (false) or 8-character (true) HOTP codes>\n",
            binary_path);
}

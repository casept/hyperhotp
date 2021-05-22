#pragma once

#include <stdbool.h>

#include "hyperhotp.h"

typedef enum {
    CLI_ACTION_INVALID,
    CLI_ACTION_HELP,
    CLI_ACTION_CHECK,
    CLI_ACTION_RESET,
    CLI_ACTION_PROGRAM,
} CLIAction;

typedef struct {
    CLIAction action;
    char serial[HYPERHOTP_SERIAL_LEN];
    char seed[HYPERHOTP_SEED_LEN_ASCII];
    bool is_8_char_code;
} CLIConfig;

CLIConfig cli_parse(const int argc, const char* argv[]);

void cli_print_help(const char* binary_path);

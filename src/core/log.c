#include <libusb-1.0/libusb.h>
#include <memory.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Pointer to the last error string.
static char *LOG_LAST_ERROR_STRING = NULL;

#define MAX_LIBUSB_ERR_LEN 8192

void log_error(const char *msg) {
    const size_t len = strlen(msg) + 1;
    char *err_str = (char *)malloc(len * sizeof(char));
    strncpy(err_str, msg, (unsigned long)len);
    LOG_LAST_ERROR_STRING = err_str;
// Also log to stderr (useful for debugging the error message mechanism itself)
#ifdef DEBUG
    fprintf(stderr, "[ERROR] %s\n", msg);
    fflush(stderr);
#endif
}

void log_error_libusb(const char *msg, const int libusb_err) {
    char *err_str = (char *)malloc(MAX_LIBUSB_ERR_LEN * sizeof(char));
    snprintf(err_str, MAX_LIBUSB_ERR_LEN, "%s: Libusb says: \"%s\"\n", msg, libusb_strerror(libusb_err));
    LOG_LAST_ERROR_STRING = err_str;
}

char *log_get_last_error_string(void) {
    const size_t len = strlen(LOG_LAST_ERROR_STRING) + 1;
    char *copy = (char *)malloc(len * sizeof(char));
    strncpy(copy, LOG_LAST_ERROR_STRING, (unsigned long)len);
    copy[len - 1] = '\0';

    return copy;
}

void log_free_error_string(char *str) {
    free(str);
    str = NULL;
}

void log_fatal(const char *msg) {
    fprintf(stderr, "[FATAL] %s\n", msg);
    fflush(stderr);
    exit(EXIT_FAILURE);
}

void log_debug(const char *msg) {
    (void)msg;

#ifdef DEBUG
    printf("[DEBUG] %s\n", msg);
    fflush(stdout);
#endif
}

void log_sent(const unsigned char *buf, const size_t buf_len) {
    (void)buf;
    (void)buf_len;

#ifdef DEBUG
    printf("[SENT] {");
    for (size_t i = 0; i < buf_len; i++) {
        printf("0x%02X ", buf[i]);
    }
    printf("}\n");
    fflush(stdout);
#endif
}

void log_received(const unsigned char *buf, const size_t buf_len) {
    (void)buf;
    (void)buf_len;

#ifdef DEBUG
    printf("[RECV] {");
    for (size_t i = 0; i < buf_len; i++) {
        printf("0x%02X ", buf[i]);
    }
    printf("}\n");
    fflush(stdout);
#endif
}

void log_libusb_callback(libusb_context *ctx, enum libusb_log_level level, const char *str) {
    (void)ctx;
    switch (level) {
        case LIBUSB_LOG_LEVEL_NONE:
        case LIBUSB_LOG_LEVEL_DEBUG:
        case LIBUSB_LOG_LEVEL_INFO:
            printf("[LIBUSB DEBUG] %s\n", str);
            break;
        case LIBUSB_LOG_LEVEL_WARNING:
            fprintf(stderr, "[LIBUSB WARNING] %s\n", str);
            break;
        case LIBUSB_LOG_LEVEL_ERROR:
            fprintf(stderr, "[LIBUSB ERROR] %s\n", str);
            break;
        default:
            printf("[LIBUSB UNKNOWN] %s\n", str);
            break;
    }
    fflush(stdout);
}

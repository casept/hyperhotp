#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <stdlib.h>

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

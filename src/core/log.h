#pragma once

#include <libusb-1.0/libusb.h>
#include <stddef.h>

void log_fatal(const char* msg);

void log_error(const char* msg);
void log_error_libusb(const char* msg, const int libusb_err);
char* log_get_last_error_string(void);
void log_free_error_string(char* str);

void log_debug(const char* msg);

void log_sent(const unsigned char* buf, const size_t buf_len);

void log_received(const unsigned char* buf, const size_t len);

void log_libusb_callback(libusb_context* ctx, enum libusb_log_level level, const char* str);

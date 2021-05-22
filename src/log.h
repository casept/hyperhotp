#pragma once

#include <libusb-1.0/libusb.h>
#include <stddef.h>

void log_fatal(const char* msg);

void log_debug(const char* msg);

void log_sent(const unsigned char* buf, const size_t buf_len);

void log_received(const unsigned char* buf, const size_t len);

void log_libusb_callback(libusb_context* ctx, enum libusb_log_level level, const char* str);

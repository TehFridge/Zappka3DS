#ifndef LOGS_H
#define LOGS_H
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <3ds.h>
#include <stdarg.h>
#define LOG_FILE "/3ds/curl_log.txt"

void init_logger();
u64 get_free_mem();
void log_to_file(const char *format, ...);
void close_logger();

#endif // LOGS_H
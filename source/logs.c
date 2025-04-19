#include "logs.h"

static FILE *log_file = NULL;
// Create a persistent text buffer at the start of the program


void init_logger() {
    log_file = fopen(LOG_FILE, "a"); // Open once at the start
}
u64 get_free_mem() {
    u64 freeMem;
    svcGetSystemInfo(&freeMem, 0, 1);
    return freeMem;
}
void log_to_file(const char *format, ...) {
    if (!log_file) return;

    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    fprintf(log_file, "\n");
    va_end(args);

    fflush(log_file); // Flush instead of closing after every write
}

void close_logger() {
    if (log_file) {
        fclose(log_file); // Close once at the end
        log_file = NULL;
    }
}
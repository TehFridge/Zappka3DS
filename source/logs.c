#include "logs.h"

bool logplz = false;
static FILE *log_file = NULL;


void init_logger() {
    log_file = fopen(LOG_FILE, "a");
}
u64 get_free_mem() {
    u64 freeMem;
    svcGetSystemInfo(&freeMem, 0, 1);
    return freeMem;
}
void log_to_file(const char *format, ...) {
    if (!log_file) return;
	if (logplz) {
		va_list args;
		va_start(args, format);
		vfprintf(log_file, format, args);
		fprintf(log_file, "\n");
		va_end(args);

		fflush(log_file);
	}
}

void close_logger() {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}
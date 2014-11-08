#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "logging/gdbwire_logger.h"

static const char *gdbwire_logger_level_str[GDBWIRE_LOGGER_ERROR+1] = {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR"
};

void
gdbwire_logger_log(const char *file, int line, enum gdbwire_logger_level level,
        const char *fmt, ...)
{
    static int checked_env = 0;
    static int gdbwire_debug_to_stderr;
    char *buf;
    int size;

    va_list ap;
    va_start(ap, fmt);

    size = vsnprintf(0, 0, fmt, ap);
    buf = malloc(sizeof(char)*size + 1);

    va_start(ap, fmt);
    size = vsnprintf(buf, size + 1, fmt, ap);
    va_end(ap);

    if (checked_env == 0) {
        checked_env = 1;
        gdbwire_debug_to_stderr = getenv("GDBWIRE_DEBUG_TO_STDERR") != NULL;
    }

    if (gdbwire_debug_to_stderr) {
        fprintf(stderr, "gdbwire_logger_log: [%s] %s:%d %s\n",
            gdbwire_logger_level_str[level], file, line, buf);
    }

    free(buf);
}

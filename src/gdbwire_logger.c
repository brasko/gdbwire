/*
 * Copyright (C) 2014 Robert Rossi <bob@brasko.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "gdbwire_logger.h"

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

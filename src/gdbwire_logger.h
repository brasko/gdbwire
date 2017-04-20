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

#ifndef __GDBWIRE_LOGGER_H__
#define __GDBWIRE_LOGGER_H__

#include "gdbwire_result.h"

#ifdef __cplusplus 
extern "C" { 
#endif 

enum gdbwire_logger_level {
    GDBWIRE_LOGGER_DEBUG,
    GDBWIRE_LOGGER_INFO,
    GDBWIRE_LOGGER_WARN,
    GDBWIRE_LOGGER_ERROR
};

/**
 * Log a statement to the logger.
 *
 * This is typically not called directly. Use the below macros instead.
 * The macros automatically supply the file, line and level arguments.
 *
 * @param file
 * The filename the logger was invoked from.
 *
 * @param line
 * The line number the logger was invoked from.
 *
 * @param level
 * The level associated with the log message.
 *
 * @param fmt
 * The format string for the message (printf formatting).
 *
 * @param ...
 * Any additional format arguments.
 */
void gdbwire_logger_log(const char *file, int line,
        enum gdbwire_logger_level level, const char *fmt, ...);

/* The macros intended to be used for logging */
#define gdbwire_debug(fmt, ...)(gdbwire_logger_log(__FILE__, __LINE__, \
        GDBWIRE_LOGGER_DEBUG, fmt, ##__VA_ARGS__))
#define gdbwire_info(fmt, ...)(gdbwire_logger_log(__FILE__, __LINE__, \
        GDBWIRE_LOGGER_INFO, fmt, ##__VA_ARGS__))
#define gdbwire_warn(fmt, ...)(gdbwire_logger_log(__FILE__, __LINE__, \
        GDBWIRE_LOGGER_WARN, fmt, ##__VA_ARGS__))
#define gdbwire_error(fmt, ...)(gdbwire_logger_log(__FILE__, __LINE__, \
        GDBWIRE_LOGGER_ERROR, fmt, ##__VA_ARGS__))

#ifdef __cplusplus 
}
#endif 

#endif

#ifndef __GDBWIRE_LOGGER_H__
#define __GDBWIRE_LOGGER_H__

#include "logging/gdbwire_result.h"

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

// The macros intended to be used for logging
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

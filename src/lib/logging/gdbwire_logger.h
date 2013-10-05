#ifndef __GDBWIRE_LOGGER_H__
#define __GDBWIRE_LOGGER_H__

/**
 * Open, close and write to a log file.
 *
 * Currently a single log file is supported per application.
 */

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
 * Initialize the logger.
 *
 * Currently, this will blast the log file if it is currently on
 * disk and will recreate it opening it with write privledges.
 *
 * @param path
 * The path to write the logging information to.
 *
 * @return
 * 0 on success or -1 on error.
 */
int gdbwire_logger_open(const char *path);

/**
 * Shut down the logging.
 *
 * If a NULL pointer is passed to this function it will ignore it
 * and successfully return.
 *
 *   clog_info(CLOG(MY_LOGGER), "Hello, world!");
 */
void gdbwire_logger_close();

/**
 * Set the minimum level of messages that should be written to the log.
 * Messages below this level will not be written.  By default, loggers are
 * created with level == GDBWIRE_LOGGER_DEBUG.
 *
 * @param level
 * The new minimum log level.
 *
 * @return
 * 0 on success or -1 on failure.
 */
int gdbwire_logger_set_level(enum gdbwire_logger_level level);

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

#define CLOG_SILENT
#define CLOG_MAIN
#include "logging/clog.h"

#include "logging/gdbwire_logger.h"
#include "logging/gdbwire_assert.h"

static const int GDBWIRE_LOGGER = 0;

static enum clog_level
convert_to_clog_level(enum gdbwire_logger_level level)
{
    enum clog_level clevel;

    switch (level) {
        case GDBWIRE_LOGGER_DEBUG:
            clevel = CLOG_DEBUG;
            break;
        case GDBWIRE_LOGGER_INFO:
            clevel = CLOG_INFO;
            break;
        case GDBWIRE_LOGGER_WARN:
            clevel = CLOG_WARN;
            break;
        case GDBWIRE_LOGGER_ERROR:
            clevel = CLOG_ERROR;
            break;
    }

    return clevel;
}

enum gdbwire_result
gdbwire_logger_open(const char *path)
{
    enum gdbwire_result result = GDBWIRE_OK;
    int fd;
    mode_t mode = 0;

#if defined(S_IRUSR)
    mode |= S_IRUSR;
#endif
#if defined(S_IWUSR)
    mode |= S_IWUSR;
#endif
#if defined(S_IRGRP)
    mode |= S_IRGRP;
#endif
#if defined(S_IROTH)
    mode |= S_IROTH;
#endif

    fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, mode);
    GDBWIRE_ASSERT_ERRNO(fd != -1);

    if (clog_init_fd(GDBWIRE_LOGGER, fd) != 0) {
        close(fd);
        result = GDBWIRE_LOGIC;
    }

    return result;
}

void
gdbwire_logger_close()
{
    clog_free(GDBWIRE_LOGGER);
}

enum gdbwire_result
gdbwire_logger_set_level(enum gdbwire_logger_level level)
{
    int result = clog_set_level(GDBWIRE_LOGGER, convert_to_clog_level(level));
    GDBWIRE_ASSERT(result != 0);
    return GDBWIRE_OK;
}

void
gdbwire_logger_log(const char *file, int line, enum gdbwire_logger_level level,
        const char *fmt, ...)
{
    enum clog_level clevel = convert_to_clog_level(level);
    va_list ap;
    va_start(ap, fmt);

    _clog_log(file, line, (enum clog_level)level, GDBWIRE_LOGGER, fmt, ap);
}

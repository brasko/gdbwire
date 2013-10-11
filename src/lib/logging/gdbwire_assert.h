#ifndef GDBWIRE_ERROR_H
#define GDBWIRE_ERROR_H

#include "logging/gdbwire_logger.h"

/**
 * Validate that the expression evaluates to true.
 *
 * If the expression does not evaluate to true, log the error and
 * return a GDBWIRE_ASSERT status code.
 *
 * Otherwise, if the expression does evaluate to true, do nothing.
 *
 * @param expr
 * The expression to evaluate.
 */
#define GDBWIRE_ASSERT(expr) \
    do { \
        if (!(expr)) { \
            gdbwire_error("Assertion failure, expr[%s]," #expr); \
            return GDBWIRE_ASSERT; \
        } \
    } while (0)

/**
 * Validate that the expression evaluates to true.
 *
 * This particular assertion macro is used when a system library
 * call fails and that library call has an associated errno status
 * to describe the failure reason.
 *
 * If the expression does not evaluate to true, log the error,
 * along with the errno value and message and return a GDBWIRE_ASSERT
 * status code.
 *
 * Otherwise, if the expression does evaluate to true, do nothing.
 *
 * @param expr
 * The expression to evaluate.
 */
#define GDBWIRE_ASSERT_ERRNO(expr) \
    do { \
        if (!(expr)) { \
            gdbwire_error("Assertion failure, expr[%s]," \
                "errno[%d], strerror[%s]", \
                #expr, errno, strerror(errno)); \
            return GDBWIRE_ASSERT; \
        } \
    } while (0)

#endif /* GDBWIRE_ERROR_H */

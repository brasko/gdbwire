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

#ifndef GDBWIRE_ERROR_H
#define GDBWIRE_ERROR_H

#include "gdbwire_result.h"
#include "gdbwire_logger.h"

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
            gdbwire_error("Assertion failure, expr[%s]", #expr); \
            return GDBWIRE_ASSERT; \
        } \
    } while (0)

/**
 * Validate that the expression evaluates to true.
 *
 * If the expression does not evaluate to true, log the error,
 * set the variable provided to GDBWIRE_ASSERT and goto the label
 * provided.
 *
 * Otherwise, if the expression does evaluate to true, do nothing.
 *
 * @param expr
 * The expression to evaluate.
 *
 * @param variable
 * The result variable to assign the value GDBWIRE_ASSERT to.
 *
 * @param label
 * The label to jump to if the expression evaluates to False.
 */
#define GDBWIRE_ASSERT_GOTO(expr, variable, label) \
    do { \
        if (!(expr)) { \
            gdbwire_error("Assertion failure, expr[%s], " \
                "label[%s]", #expr, #label); \
            variable = GDBWIRE_ASSERT; \
            goto label; \
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

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

#ifndef GDBWIRE_RESULT_H
#define GDBWIRE_RESULT_H

enum gdbwire_result {
    /* The result of the operation was successful */
    GDBWIRE_OK,

    /**
     * An assertion failed in the calling code.
     *
     * Functions are encouraged to assert expressions they expect
     * to be true. The macro GDBWIRE_ASSERT and GDBWIRE_ASSERT_ERRNO
     * are useful for asserting expressions, and upon failure, to
     * automatically log the assertion expression and return
     * this result status.
     */
    GDBWIRE_ASSERT,

    /**
     * An internal logic error has occurred.
     *
     * In general, this should be used when a function can no
     * longer carry out it's contract and must abort.
     *
     * This happens, for instance, when a called function returns
     * an error status, or when invalid input was provided, etc.
     */
    GDBWIRE_LOGIC,

    /**
     * The system is out of memory.
     *
     * Will occur when malloc, strdup, calloc, etc fail to allocate memory.
     */
    GDBWIRE_NOMEM
};

#endif /* GDBWIRE_RESULT_H */

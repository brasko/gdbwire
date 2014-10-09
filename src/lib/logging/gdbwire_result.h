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
    GDBWIRE_LOGIC
};

#endif /* GDBWIRE_RESULT_H */

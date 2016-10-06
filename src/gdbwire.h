#ifndef GDBWIRE_H
#define GDBWIRE_H

#ifdef __cplusplus 
extern "C" { 
#endif 

#include <stdlib.h>
#include "gdbwire_result.h"
#include "gdbwire_mi_pt.h"
#include "gdbwire_mi_command.h"

/* The opaque gdbwire context */
struct gdbwire;

/**
 * The primary mechanism for gdbwire to send events to the caller.
 *
 * The flow is like this:
 * - create a gdbwire instance
 * - loop:
 *   - call gdbwire functions to send commands to gdb
 *   - receive callback events with results when they become available
 * - destroy the instance
 */
struct gdbwire_callbacks {
    /**
     * An arbitrary pointer to associate with the events.
     *
     * This pointer will be passed back to the caller in each event.
     */
    void *context;
    
    /**
     * A console, target or log output event has occured.
     *
     * @param context
     * The context pointer above.
     *
     * @param stream_record
     * The stream record to display to the user.
     */
    void (*gdbwire_stream_record_fn)(void *context,
            struct gdbwire_mi_stream_record *stream_record);

    /**
     * An asynchronous output event.
     *
     * @param context
     * The context pointer above.
     *
     * @param async_record
     * The asychronous record output by GDB.
     */
    void (*gdbwire_async_record_fn)(void *context,
            struct gdbwire_mi_async_record *async_record);

    /**
     * A result output event.
     *
     * @param context
     * The context pointer above.
     *
     * @param result_record
     * The result record output by GDB.
     */
    void (*gdbwire_result_record_fn)(void *context,
            struct gdbwire_mi_result_record *result_record);

    /**
     * A prompt output event.
     *
     * @param context
     * The context pointer above.
     *
     * @param prompt
     * The prompt output to display to the user.
     */
    void (*gdbwire_prompt_fn)(void *context, const char *prompt);

    /**
     * A gdbwire parse error occurred.
     *
     * If you receive this callback, that means the gdbwire parser
     * failed to parse some gdb/mi coming out of gdb.
     * Please send the parameters received in this callback to the
     * gdbwire develpment team.
     *
     * @param context
     * The context pointer above.
     *
     * @param mi
     * The mi string that gdbwire could not parse.
     *
     * @param token
     * The token the error occurred on.
     *
     * @param position
     * The position of the token the error occurred on.
     */
    void (*gdbwire_parse_error_fn)(void *context, const char *mi,
            const char *token, struct gdbwire_mi_position position);
};

/**
 * Create a gdbwire context.
 *
 * Each gdbwire structure is capable of talking to a single gdb instance.
 *
 * @param callbacks
 * The callback functions for when events should be sent. Be sure to
 * initialize all of the callback functions. If a callback event is
 * initialized to NULL, it will not be called.
 *
 * @return
 * A new gdbwire instance or NULL on error.
 */
struct gdbwire *gdbwire_create(struct gdbwire_callbacks callbacks);

/**
 * Destroy a gdbwire context.
 *
 * This function will do nothing if the instance is NULL.
 *
 * @param gdbwire
 * The instance of gdbwire to destroy
 */
void gdbwire_destroy(struct gdbwire *wire);

/**
 * Push some GDB output characters to gdbwire for processing.
 *
 * Currently, the calling application is responsible for reading the
 * output of GDB and sending it to gdbwire. This may change in the future.
 * Call this function with output from GDB when it is available.
 *
 * During this function, callback events may be invoked to alert the
 * caller of useful gdbwire_mi events.
 *
 * @param wire
 * The gdbwire context to operate on.
 *
 * @param data
 * The data to push to gdbwire for interpretation.
 *
 * @param size
 * The size of the data to push to gdbwire.
 *
 * @return
 * GDBWIRE_OK on success or appropriate error result on failure.
 */
enum gdbwire_result gdbwire_push_data(struct gdbwire *wire, const char *data,
        size_t size);

/**
 * Handle an interpreter-exec command.
 *
 * Typically, a front end would start gdb with the MI interface and create
 * a corresponding gdbwire instance. The front end would feed the gdbwire
 * instance all of the MI output. In this scenario, gdbwire triggers
 * callbacks when interesting events occur.
 *
 * Some GDB front ends use the annotate interface with gdb, and will
 * transition to using MI through the use of the interpreter-exec command.
 * In this scenario, the front end will send GDB a single interpreter-exec
 * command and will want to interpret the output of only that command.
 * For this use case, a gdbwire instance is not necessary for the front end,
 * nor any of the callbacks associated with that instance.
 *
 * This function provides a way for a front end to interpret the output
 * of a single interpreter-exec command with out the need for creating
 * a gdbwire instance or any gdbwire callbacks.
 *
 * @param interpreter_exec_output
 * The MI output from GDB for the interpreter exec command.
 *
 * @param kind
 * The interpreter-exec command kind.
 *
 * @param out_mi_command
 * Will return an allocated gdbwire mi command if GDBWIRE_OK is returned
 * from this function. You should free this memory with
 * gdbwire_mi_command_free when you are done with it.
 *
 * @return
 * The result of this function.
 */
enum gdbwire_result gdbwire_interpreter_exec(
        const char *interpreter_exec_output,
        enum gdbwire_mi_command_kind kind,
        struct gdbwire_mi_command **out_mi_command);

#ifdef __cplusplus 
}
#endif 

#endif

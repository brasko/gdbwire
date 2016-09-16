#ifndef GDBWIRE_H
#define GDBWIRE_H

#include <logging/gdbwire_result.h>

#ifdef __cplusplus 
extern "C" { 
#endif 

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
     * A console output event.
     *
     * @param context
     * The context pointer above.
     *
     * @param str
     * The console output to display to the user.
     */
    void (*gdbwire_console_fn)(void *context, const char *str);

    /**
     * A target output event.
     *
     * @param context
     * The context pointer above.
     *
     * @param str
     * The target output to display to the user.
     */
    void (*gdbwire_target_fn)(void *context, const char *str);

    /**
     * A log output event.
     *
     * @param context
     * The context pointer above.
     *
     * @param str
     * The log output to display to the user.
     */
    void (*gdbwire_log_fn)(void *context, const char *str);

    /**
     * A prompt output event.
     *
     * @param context
     * The context pointer above.
     *
     * @param str
     * The prompt output to display to the user.
     */
    void (*gdbwire_prompt_fn)(void *context, const char *str);
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
 * caller of useful gdbmi events.
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

#ifdef __cplusplus 
}
#endif 

#endif

#ifndef __GDBMI_PARSER_H__
#define __GDBMI_PARSER_H__

#ifdef __cplusplus 
extern "C" { 
#endif 

#include "gdbmi_pt.h"

/// The opaque GDB/MI parser context
struct gdbmi_parser;

/**
 * The primary mechanism to alert users of GDB/MI notifications.
 *
 * The flow is like this:
 * - create a parser context (gdbmi_parser_create)
 * - push onto the parser arbitrary amounts of data (gdbmi_parser_push)
 *   - receive callbacks from inside gdbmi_parser_push when
 *     it discovers callbacks the user will find interesting
 * - destroy the parser (gdbmi_parser_destroy)
 */
struct gdbmi_parser_callbacks {
    /**
     * An arbitrary pointer to associate with the callbacks.
     *
     * If the calling api is C++ it is useful to make this an instance
     * of an object you want to bind to the callback functions below.
     */
    void *context;
    
    /**
     * A GDB/MI output command is available.
     *
     * @param context
     * The context pointer above.
     *
     * @param output
     * The gdbmi output command. This output command is now owned by the
     * function being invoked and should be destroyed when necessary.
     */
    void (*gdbmi_output_callback)(void *context, struct gdbmi_output *output);
};

/**
 * Create a GDB/MI parser context.
 *
 * @param callbacks
 * The callback functions to invoke upon discovery of parse data.
 *
 * @return
 * A new GDB/MI parser instance or NULL on error.
 */
struct gdbmi_parser *gdbmi_parser_create(
        struct gdbmi_parser_callbacks callbacks);

/**
 * Destroy a gdbmi_parser context.
 *
 * If a NULL pointer is passed to this function it will ignore it
 * and successfully return.
 *
 * @param parser
 * The instance the parser to destroy
 *
 * @return
 * 0 on succes, or -1 on error.
 */
int gdbmi_parser_destroy(struct gdbmi_parser *parser);

/**
 * Push some parse data onto the parser.
 *
 * During this function, if a gdbmi output command is discovered by
 * the parser (or any other useful GDB/MI notification), it will invoke
 * the appropriate callbacks assigned during parser creation.
 *
 * @param parser
 * The gdbmi parser context to operate on.
 *
 * @param data
 * The parse data to push onto the parser.
 *
 * @return
 * 0 on success or -1 on error.
 */
int gdbmi_parser_push(struct gdbmi_parser *parser, char *data);

#ifdef __cplusplus 
}
#endif 

#endif

#ifndef __GDBMI_PARSER_H__
#define __GDBMI_PARSER_H__

#ifdef __cplusplus 
extern "C" { 
#endif 

#include "logging/gdbwire_result.h"
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
 * This function will do nothing if parser is NULL.
 *
 * @param parser
 * The instance the parser to destroy
 */
void gdbmi_parser_destroy(struct gdbmi_parser *parser);

/**
 * Push a null terminated string onto the parser.
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
 * GDBWIRE_OK on success or appropriate error result on failure.
 */
enum gdbwire_result gdbmi_parser_push(struct gdbmi_parser *parser,
        const char *data);

/**
 * Push some parse data onto the parser.
 *
 * See gdbmi_parser_push for details on function behavior.
 *
 * @param parser
 * The gdbmi parser context to operate on.
 *
 * @param data
 * The parse data to push onto the parser.
 *
 * @param size
 * The size of the data to push onto the parser.
 *
 * @return
 * GDBWIRE_OK on success or appropriate error result on failure.
 */
enum gdbwire_result gdbmi_parser_push_data(struct gdbmi_parser *parser,
        const char *data, size_t size);

#ifdef __cplusplus 
}
#endif 

#endif

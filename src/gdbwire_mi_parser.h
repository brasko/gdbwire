#ifndef GDBWIRE_MI_PARSER_H
#define GDBWIRE_MI_PARSER_H

#ifdef __cplusplus 
extern "C" { 
#endif 

#include "gdbwire_result.h"
#include "gdbwire_mi_pt.h"

/// The opaque GDB/MI parser context
struct gdbwire_mi_parser;

/**
 * The primary mechanism to alert users of GDB/MI notifications.
 *
 * The flow is like this:
 * - create a parser context (gdbwire_mi_parser_create)
 * - push onto the parser arbitrary amounts of data (gdbwire_mi_parser_push)
 *   - receive callbacks from inside gdbwire_mi_parser_push when
 *     it discovers callbacks the user will find interesting
 * - destroy the parser (gdbwire_mi_parser_destroy)
 */
struct gdbwire_mi_parser_callbacks {
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
     * The gdbwire_mi output command. This output command is now owned by the
     * function being invoked and should be destroyed when necessary.
     */
    void (*gdbwire_mi_output_callback)(void *context,
        struct gdbwire_mi_output *output);
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
struct gdbwire_mi_parser *gdbwire_mi_parser_create(
        struct gdbwire_mi_parser_callbacks callbacks);

/**
 * Destroy a gdbwire_mi_parser context.
 *
 * This function will do nothing if parser is NULL.
 *
 * @param parser
 * The instance the parser to destroy
 */
void gdbwire_mi_parser_destroy(struct gdbwire_mi_parser *parser);

/**
 * Push a null terminated string onto the parser.
 *
 * During this function, if a gdbwire_mi output command is discovered by
 * the parser (or any other useful GDB/MI notification), it will invoke
 * the appropriate callbacks assigned during parser creation.
 *
 * @param parser
 * The gdbwire_mi parser context to operate on.
 *
 * @param data
 * The parse data to push onto the parser.
 * 
 * @return
 * GDBWIRE_OK on success or appropriate error result on failure.
 */
enum gdbwire_result gdbwire_mi_parser_push(struct gdbwire_mi_parser *parser,
        const char *data);

/**
 * Push some parse data onto the parser.
 *
 * See gdbwire_mi_parser_push for details on function behavior.
 *
 * @param parser
 * The gdbwire_mi parser context to operate on.
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
enum gdbwire_result gdbwire_mi_parser_push_data(
        struct gdbwire_mi_parser *parser, const char *data, size_t size);

#ifdef __cplusplus 
}
#endif 

#endif

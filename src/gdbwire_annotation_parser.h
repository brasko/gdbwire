#ifndef GDBWIRE_ANNOTATION_PARSER_H
#define GDBWIRE_ANNOTATION_PARSER_H

#ifdef __cplusplus 
extern "C" { 
#endif 

#include "gdbwire_result.h"
#include "gdbwire_string.h"
#include "gdbwire_annotation_pt.h"

/* The opaque GDB/Annotation parser context */
struct gdbwire_annotation_parser;

/**
 * The primary mechanism to alert users of GDB/Annotation notifications.
 *
 * The flow is like this:
 * - create a parser context (gdbwire_annotation_parser_create)
 * - push onto the parser arbitrary amounts of data
 *   using the function (gdbwire_annotation_parser_push)
 *   - receive callbacks from inside gdbwire_annotation_parser_push when
 *     it discovers callbacks the user will find interesting
 * - destroy the parser (gdbwire_annotation_parser_destroy)
 */
struct gdbwire_annotation_parser_callbacks {
    /**
     * An arbitrary pointer to associate with the callbacks.
     *
     * If the calling api is C++ it is useful to make this an instance
     * of an object you want to bind to the callback functions below.
     */
    void *context;
    
    /**
     * A GDB/Annotation output command is available.
     *
     * @param context
     * The context pointer above.
     *
     * @param output
     * The gdbwire_annotation output command. This output command is now
     * owned by the function being invoked and should be destroyed
     * when necessary.
     */
    void (*gdbwire_annotation_output_callback)(void *context,
        struct gdbwire_annotation_output *output);
};

/**
 * Create a GDB/Annotation parser context.
 *
 * @param callbacks
 * The callback functions to invoke upon discovery of parse data.
 *
 * @return
 * A new GDB/Annotation parser instance or NULL on error.
 */
struct gdbwire_annotation_parser *gdbwire_annotation_parser_create(
        struct gdbwire_annotation_parser_callbacks callbacks);

/**
 * Destroy a gdbwire_annotation_parser context.
 *
 * This function will do nothing if parser is NULL.
 *
 * @param parser
 * The instance the parser to destroy
 */
void gdbwire_annotation_parser_destroy(
        struct gdbwire_annotation_parser *parser);

/**
 * Push a null terminated string onto the parser.
 *
 * During this function, if a GDB/Annotation notification is discovered by
 * the parser, it will invoke the appropriate callbacks assigned during
 * parser creation.
 *
 * @param parser
 * The gdbwire_annotation parser context to operate on.
 *
 * @param data
 * The parse data to push onto the parser.
 * 
 * @return
 * GDBWIRE_OK on success or appropriate error result on failure.
 */
enum gdbwire_result gdbwire_annotation_parser_push(
        struct gdbwire_annotation_parser *parser, const char *data);

/**
 * Push some parse data onto the parser.
 *
 * See gdbwire_annotation_parser_push for details on function behavior.
 *
 * @param parser
 * The gdbwire_annotation parser context to operate on.
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
enum gdbwire_result gdbwire_annotation_parser_push_data(
        struct gdbwire_annotation_parser *parser,
        const char *data, size_t size);

#ifdef __cplusplus 
}
#endif 

#endif

#ifndef __GDBMI_PARSER_H__
#define __GDBMI_PARSER_H__

#ifdef __cplusplus 
extern "C" { 
#endif 

#include "gdbmi_pt.h"

/// The opaque GDB/MI parser context
struct gdbmi_parser;

/**
 * Create a GDB/MI parser context.
 *
 * @return
 * A new GDB/MI parser instance or NULL on error.
 */
struct gdbmi_parser *gdbmi_parser_create(void);

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
 * @param parser
 * The gdbmi parser context to operate on.
 *
 * @param data
 * The parse data to push onto the parser.
 *
 * @param pt
 * A gdbmi output command if available or NULL if none discovered yet.
 * This data is now owned by the caller and thus should be freed by the caller.
 *
 * @return
 * 0 on success or -1 on error.
 */
int gdbmi_parser_push(struct gdbmi_parser *parser,
        char *data, struct gdbmi_output **pt);

#ifdef __cplusplus 
}
#endif 

#endif

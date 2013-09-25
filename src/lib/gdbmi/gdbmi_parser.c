#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gdbmi_grammar.h"
#include "gdbmi_parser.h"
#include "containers/gdbwire_string.h"

/* flex prototypes used in this unit */
typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE gdbmi__scan_string(const char *yy_str);
extern void gdbmi__delete_buffer(YY_BUFFER_STATE state);
extern int gdbmi_lex(void);

struct gdbmi_parser {
    /* The buffer pushed into the parser from the user */
    struct gdbwire_string *buffer;
    /* The GDB/MI push parser state */
    gdbmi_pstate *mips;
    /* The GDB/MI output command found during parsing */
    struct gdbmi_output *oc;
};

struct gdbmi_parser *
gdbmi_parser_create(void)
{
    struct gdbmi_parser *parser;

    parser = (struct gdbmi_parser *)calloc(1, sizeof(struct gdbmi_parser));
    if (!parser) {
        return NULL;
    }

    /* Create a new buffer for the user to push parse data into */
    parser->buffer = gdbwire_string_create();
    if (!parser->buffer) {
        free(parser);
        return NULL;
    }

    /* Create a new push parser state instance */
    parser->mips = gdbmi_pstate_new();
    if (!parser->mips) {
        free(parser);
        free(parser->buffer);
        return NULL;
    }

    return parser;
}

int gdbmi_parser_destroy(struct gdbmi_parser *parser)
{
    if (!parser) {
        return 0;
    }

    /* Free the parse buffer */
    if (parser->buffer) {
        gdbwire_string_destroy(parser->buffer);
        parser->buffer = NULL;
    }

    /* Free the push parser instance */
    if (parser->mips) {
        gdbmi_pstate_delete(parser->mips);
        parser->mips = NULL;
    }

    /* The parser output command (parser->oc) should be deleted by the client */

    free(parser);
    parser = NULL;
    return 0;
}

/**
 * Parse a line of output in GDB/MI format.
 *
 * The data must be either a full line or multiple full lines.
 * The normal usage of this function is to call it over and over again with
 * more data and wait for it to return an mi output command.
 *
 * @param parser
 * The parser context to operate on.
 *
 * @param line
 * A line of output in GDB/MI format.
 *
 * @param pt
 * If this function is successful (returns 0), then pt may be set.
 * If the line parameter completes an mi output command, than pt will
 * be non-null and represent the command parsed. Otherwise, if it is waiting
 * for more intput, it will be returned as NULL.
 *
 * The user is responsible for freeing this data structure on there own.
 *
 * \param parse_failed
 * 1 if the parser failed to parse the command, otherwise 0
 * If there was an error, it was written to the global logger.
 * Also, pt is invalid if there is an error, it should not be displayed or freed
 *
 * \return
 * 0 on succes, or -1 on error.
 */
static int
gdbmi_parser_parse_line(struct gdbmi_parser *parser,
        const char *line, struct gdbmi_output **pt, int *parse_failed)
{
    YY_BUFFER_STATE state;
    int pattern;
    int mi_status;

    if (!parser || !line || !pt || !parse_failed) {
        return -1;
    }

    /* Initialize output parameters */
    *pt = 0;
    *parse_failed = 0;

    /* Create a new input buffer for flex. */
    state = gdbmi__scan_string(line);

    /* Create a new input buffer for flex and
     * iterate over all the tokens. */
    do {
        pattern = gdbmi_lex();
        if (pattern == 0)
            break;
        mi_status = gdbmi_push_parse(parser->mips, pattern, NULL, parser);
    } while (mi_status == YYPUSH_MORE);

    /* Parser is done, this should never happen */
    if (mi_status != YYPUSH_MORE && mi_status != 0) {
        *parse_failed = 1;
    /* Parser finished but no output command was found */
    } else if (!parser->oc) {
        *parse_failed = 1;
    } else {
        *pt = parser->oc;
        parser->oc = NULL;
    }

    /* Free the scanners buffer */
    gdbmi__delete_buffer(state);

    return 0;
}

int
gdbmi_parser_push(struct gdbmi_parser *parser, char *data,
        struct gdbmi_output **pt)
{
    int result = 0;

    if (!parser || !data || !pt) {
        return -1;
    }

    result = gdbwire_string_append_cstr(parser->buffer, data);
    if (result == 0) {
        char *data = gdbwire_string_data(parser->buffer);
        size_t size = gdbwire_string_size(parser->buffer);
        size_t pos = gdbwire_string_find_first_of(parser->buffer, "\r\n");
        int status;

        // Search to see if a newline has been reached in gdb/mi.
        // If a line of data has been recieved, process it.
        if (pos != size) {
            size_t end_pos = pos + 1;
            char *command;

            // OK, found a newline. A newline can be \r, \n or \r\n.
            // Figure out which one it is, and pull it out of the buffer
            // Check specifically for \r\n and incrase the end pos
            if (data[pos] == '\r' && end_pos < size &&
                data[end_pos] == '\n') {
                end_pos++;
            }
            
            command = strndup(data, end_pos);
            result = gdbwire_string_erase(parser->buffer, 0, end_pos);
            if (result == 0) {
                result = gdbmi_parser_parse_line(
                        parser, command, pt, &status);
            }
            free(command);
        }
    }

    return result;
}

struct gdbmi_output *
gdbmi_parser_get_output(struct gdbmi_parser *parser)
{
    return parser->oc;
}

void
gdbmi_parser_set_output(struct gdbmi_parser *parser,
        struct gdbmi_output *output)
{
    parser->oc = output;
}

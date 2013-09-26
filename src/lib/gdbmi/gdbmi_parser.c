#include <stdlib.h>
#include <stdio.h>

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
    /* The client parser callbacks */
    struct gdbmi_parser_callbacks callbacks;
};

struct gdbmi_parser *
gdbmi_parser_create(struct gdbmi_parser_callbacks callbacks)
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

    parser->callbacks = callbacks;

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
 * \param parse_failed
 * 1 if the parser failed to parse the command, otherwise 0
 * If there was an error, it was written to the global logger.
 *
 * \return
 * 0 on succes, or -1 on error.
 */
static int
gdbmi_parser_parse_line(struct gdbmi_parser *parser,
        const char *line, int *parse_failed)
{
    YY_BUFFER_STATE state;
    int pattern;
    int mi_status;

    if (!parser || !line || !parse_failed) {
        return -1;
    }

    /* Initialize output parameters */
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
    }

    /* Free the scanners buffer */
    gdbmi__delete_buffer(state);

    return 0;
}

int
gdbmi_parser_push(struct gdbmi_parser *parser, char *data)
{
    int result = 0;

    if (!parser || !data) {
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
            struct gdbwire_string *command;

            command = gdbwire_string_create();

            // OK, found a newline. A newline can be \r, \n or \r\n.
            // Figure out which one it is, and pull it out of the buffer
            // Check specifically for \r\n and incrase the end pos
            if (data[pos] == '\r' && end_pos < size &&
                data[end_pos] == '\n') {
                end_pos++;
            }
            
            gdbwire_string_append_data(command, data, end_pos);
            gdbwire_string_append_data(command, "\0", 1);

            result = gdbwire_string_erase(parser->buffer, 0, end_pos);
            if (result == 0) {
                result = gdbmi_parser_parse_line(parser,
                        gdbwire_string_data(command), &status);
            }
            gdbwire_string_destroy(command);
        }
    }

    return result;
}

struct gdbmi_parser_callbacks
gdbmi_parser_get_callbacks(struct gdbmi_parser *parser)
{
    return parser->callbacks;
}

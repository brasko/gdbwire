#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gdbmi_grammar.h"
#include "gdbmi_parser.h"

int gdbmi_parse(void);

/* flex */
typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE gdbmi__scan_string(const char *yy_str);
extern void gdbmi__delete_buffer(YY_BUFFER_STATE state);
extern FILE *gdbmi_in;
extern int gdbmi_lex(void);
extern char *gdbmi_text;
extern int gdbmi_lineno;

struct gdbmi_parser {
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

    /* Create a new push parser state instance */
    parser->mips = gdbmi_pstate_new();
    if (!parser->mips) {
        free(parser);
        return NULL;
    }

    return parser;
}

int gdbmi_parser_destroy(struct gdbmi_parser *parser)
{
    if (!parser) {
        return 0;
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

int
gdbmi_parser_parse_string(struct gdbmi_parser *parser,
        const char *mi_command, struct gdbmi_output **pt, int *parse_failed)
{
    YY_BUFFER_STATE state;
    int pattern;
    int mi_status;

    if (!parser)
        return -1;

    if (!mi_command)
        return -1;

    if (!parse_failed)
        return -1;

    /* Initialize output parameters */
    *pt = 0;
    *parse_failed = 0;

    /* Create a new input buffer for flex. */
    state = gdbmi__scan_string(strdup(mi_command));

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
gdbmi_parser_parse_file(struct gdbmi_parser *parser,
        const char *mi_command_file, struct gdbmi_output **pt,
        int *parse_failed)
{
    int pattern;
    int mi_status;

    if (!parser)
        return -1;

    if (!mi_command_file)
        return -1;

    if (!parse_failed)
        return -1;

    *pt = 0;
    *parse_failed = 0;

    /* Initialize data */
    gdbmi_in = fopen(mi_command_file, "r");

    if (!gdbmi_in) {
        fprintf(stderr, "%s:%d", __FILE__, __LINE__);
        return -1;
    }

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

    fclose(gdbmi_in);

    return 0;
}

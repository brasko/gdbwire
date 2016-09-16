#include <assert.h>
#include <stdio.h>
#include "gdbwire_mi_parser.h"

/**
 * The gdbwire_mi parser callback function.
 *
 * This function silently accepts gdbwire_mi output commands. If the parser
 * fails to parse a gdbwire_mi output command this function will print the
 * failed gdbwire_mi output line and then abort the program.
 *
 * @param context
 * The context passed to the parser in gdbwire_mi_parser_create.
 *
 * @param output
 * The gdbwire_mi output command discovered by the parser.
 */
void
parser_callback(void *context, struct gdbwire_mi_output *output)
{
    assert(!context);
    assert(output);

    if (output->kind == GDBWIRE_MI_OUTPUT_PARSE_ERROR) {
        printf("\n  Parse Error: %s\n", output->line);
    }

    assert(output->kind != GDBWIRE_MI_OUTPUT_PARSE_ERROR);
    assert(!output->next);
    gdbwire_mi_output_free(output);
}

/**
 * The main loop in the gdbwire_mi example.
 *
 * The main loop is responsible for reading data from stdin and sending it
 * to the gdbwire_mi parser. This happens until stdin is closed and EOF is read.
 *
 * @param parser
 * The gdbwire_mi parser.
 */
void
main_loop(struct gdbwire_mi_parser *parser)
{
    int c;
    enum gdbwire_result result;

    while ((c = getchar()) != EOF) {
        char ch = c;
        printf("%c", ch);
        result = gdbwire_mi_parser_push_data(parser, &ch, 1);
        assert(result == GDBWIRE_OK);
    }
}

/**
 * The gdbwire_mi main function.
 *
 * This function is responsible for allocating the gdbwire_mi parser,
 * calling the main loop which uses the parser and then deleting the
 * gdbwire_mi parser.
 */
int
main(void) {
    struct gdbwire_mi_parser_callbacks callbacks = { 0, parser_callback };
    struct gdbwire_mi_parser *parser;

    parser = gdbwire_mi_parser_create(callbacks);
    assert(parser);
    main_loop(parser);
    gdbwire_mi_parser_destroy(parser);
    return 0;
}

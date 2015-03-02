#include <assert.h>
#include <stdio.h>
#include "gdbmi/gdbmi_parser.h"

/**
 * The gdbmi parser callback function.
 *
 * This function silently accepts gdbmi output commands. If the parser fails
 * to parse a gdbmi output command this function will print the failed gdbmi
 * output line and then abort the program.
 *
 * @param context
 * The context passed to the parser in gdbmi_parser_create.
 *
 * @param output
 * The gdbmi output command discovered by the parser.
 */
void
parser_callback(void *context, struct gdbmi_output *output)
{
    assert(!context);
    assert(output);

    if (output->kind == GDBMI_OUTPUT_PARSE_ERROR) {
        printf("\n  Parse Error: %s\n", output->line);
    }

    assert(output->kind != GDBMI_OUTPUT_PARSE_ERROR);
    assert(!output->next);
    gdbmi_output_free(output);
}

/**
 * The main loop in the gdbmi example.
 *
 * The main loop is responsible for reading data from stdin and sending it
 * to the gdbmi parser. This happens until stdin is closed and EOF is read.
 * This program is particularly useful if run from the command line as
 * follows:
 *   gdb -i=mi <gdb arguments> | examples/gdbmi
 *
 * This allows a user to interact with gdb in arbitrary ways from the 
 * command line. At the same time, the user can determine if the gdbmi
 * parser is able to handle the gdbmi output created by gdb.
 *
 * @param parser
 * The gdbmi parser.
 */
void
main_loop(struct gdbmi_parser *parser)
{
    int c;
    enum gdbwire_result result;

    while ((c = getchar()) != EOF) {
        char ch = c;
        printf("%c", ch);
        result = gdbmi_parser_push_data(parser, &ch, 1);
        assert(result == GDBWIRE_OK);
    }
}

/**
 * The gdbmi main function.
 *
 * This function demonstrates using the gdbmi parser interface.
 *
 * This function is responsible for allocating the gdbmi parser, calling the
 * main loop which uses the parser and then deleting the gdbmi parser.
 */
int
main(void) {
    struct gdbmi_parser_callbacks callbacks = { 0, parser_callback };
    struct gdbmi_parser *parser;

    parser = gdbmi_parser_create(callbacks);
    assert(parser);
    main_loop(parser);
    gdbmi_parser_destroy(parser);
    return 0;
}

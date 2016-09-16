#include <assert.h>
#include <stdio.h>
#include "gdbwire.h"

/**
 * The gdbwire stream record event.
 *
 * @param context
 * The context passed to gdbwire in gdbwire_mi_parser_create.
 *
 * @param stream_record
 * The stream record to display to the user.
 */
void
gdbwire_stream_record(void *context,
    struct gdbwire_mi_stream_record *stream_record)
{
    assert(!context && stream_record);
    printf("%s", stream_record->cstring);
    fflush(stdout);
}

/**
 * The gdbwire prompt event.
 *
 * @param context
 * The context passed to gdbwire in gdbwire_mi_parser_create.
 *
 * @param prompt
 * The prompt to display to the user.
 */
void
gdbwire_prompt(void *context, const char *prompt)
{
    assert(!context && prompt);
    printf("%s", prompt);
    fflush(stdout);
}

/**
 * The gdbwire parse error event.
 *
 * @param context
 * The context passed to gdbwire in gdbwire_mi_parser_create.
 *
 */
void
gdbwire_parse_error(void *context, const char *mi, const char *token,
    struct gdbwire_mi_position position)
{
    assert(!context && mi && token);
    printf("Parse error:\n");
    printf("  at token:[%s]\n", mi);
    printf("  token start column:%d\n", position.start_column);
    printf("  token end column:%d\n", position.end_column);
    printf("  line:[%s]\n", mi);
    fflush(stdout);
}

/**
 * The main loop in the gdbwire_mi example.
 *
 * The main loop is responsible for reading data from stdin and sending it
 * to gdbwire. This happens until stdin is closed and EOF is read.
 *
 * @param parser
 * The gdbwire_mi parser.
 */
void
main_loop(struct gdbwire *wire)
{
    int c;
    enum gdbwire_result result;

    while ((c = getchar()) != EOF) {
        char ch = c;
        result = gdbwire_push_data(wire, &ch, 1);
        assert(result == GDBWIRE_OK);
    }
}

/**
 * The gdbwire example.
 */
int
main(void) {
    struct gdbwire_callbacks callbacks = {
        0,
        gdbwire_stream_record,
        0,
        0,
        gdbwire_prompt,
        gdbwire_parse_error
    };
    struct gdbwire *wire;

    wire = gdbwire_create(callbacks);
    assert(wire);
    main_loop(wire);
    gdbwire_destroy(wire);
    return 0;
}

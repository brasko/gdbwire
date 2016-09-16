#include <assert.h>
#include <stdio.h>
#include "gdbwire/gdbwire.h"

/**
 * The gdbwire console event.
 *
 * @param context
 * The context passed to gdbwire in gdbmi_parser_create.
 *
 * @param str
 * The console string.
 */
void
gdbwire_console(void *context, const char *str)
{
    assert(!context && str);
    printf("%s", str);
    fflush(stdout);
}

/**
 * The gdbwire target event.
 *
 * @param context
 * The context passed to gdbwire in gdbmi_parser_create.
 *
 * @param str
 * The target string.
 */
void
gdbwire_target(void *context, const char *str)
{
    assert(!context && str);
    printf("%s", str);
    fflush(stdout);
}

/**
 * The gdbwire log event.
 *
 * @param context
 * The context passed to gdbwire in gdbmi_parser_create.
 *
 * @param str
 * The log string.
 */
void
gdbwire_log(void *context, const char *str)
{
    assert(!context && str);
    printf("%s", str);
    fflush(stdout);
}

/**
 * The gdbwire prompt event.
 *
 * @param context
 * The context passed to gdbwire in gdbmi_parser_create.
 *
 * @param str
 * The prompt string.
 */
void
gdbwire_prompt(void *context, const char *str)
{
    assert(!context && str);
    printf("%s", str);
    fflush(stdout);
}

/**
 * The main loop in the gdbmi example.
 *
 * The main loop is responsible for reading data from stdin and sending it
 * to gdbwire. This happens until stdin is closed and EOF is read.
 *
 * @param parser
 * The gdbmi parser.
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
        gdbwire_console,
        gdbwire_target,
        gdbwire_log,
        gdbwire_prompt
    };
    struct gdbwire *wire;

    wire = gdbwire_create(callbacks);
    assert(wire);
    main_loop(wire);
    gdbwire_destroy(wire);
    return 0;
}

#include <stdlib.h>

#include "logging/gdbwire_assert.h"
#include "gdbwire/gdbwire.h"
#include "gdbmi/gdbmi_parser.h"

struct gdbwire
{
    /* The gdbmi parser. */
    struct gdbmi_parser *parser;

    /* The client callback functions */
    struct gdbwire_callbacks callbacks;
};

static void
gdbmi_output_callback(void *context, struct gdbmi_output *output) {
    struct gdbwire *wire = (struct gdbwire *)context;

    struct gdbmi_output *cur = output;
    while (cur) {
        if (cur->kind == GDBMI_OUTPUT_PROMPT) {
            if (wire->callbacks.gdbwire_prompt_fn) {
                wire->callbacks.gdbwire_prompt_fn(
                    wire->callbacks.context, cur->line);
            }
        }
    

        if (cur->kind == GDBMI_OUTPUT_OOB &&
            cur->variant.oob_record->kind == GDBMI_STREAM) {
            struct gdbmi_stream_record *stream =
                    cur->variant.oob_record->variant.stream_record;
            switch (stream->kind) {
                case GDBMI_CONSOLE:
                    if (wire->callbacks.gdbwire_console_fn) {
                        wire->callbacks.gdbwire_console_fn(
                            wire->callbacks.context, stream->cstring);
                    }
                    break;
                case GDBMI_TARGET:
                    if (wire->callbacks.gdbwire_target_fn) {
                        wire->callbacks.gdbwire_target_fn(
                            wire->callbacks.context, stream->cstring);
                    }
                    break;
                case GDBMI_LOG:
                    if (wire->callbacks.gdbwire_log_fn) {
                        wire->callbacks.gdbwire_log_fn(
                            wire->callbacks.context, stream->cstring);
                    }
                    break;
            };
        }

        cur = cur->next;
    }

    gdbmi_output_free(output);
}

struct gdbwire *
gdbwire_create(struct gdbwire_callbacks callbacks)
{
    struct gdbwire *result = 0;
    
    result = malloc(sizeof(struct gdbwire));
    if (result) {
        struct gdbmi_parser_callbacks parser_callbacks =
            { result,gdbmi_output_callback };
        result->callbacks = callbacks;
        result->parser = gdbmi_parser_create(parser_callbacks);
        if (!result->parser) {
            free(result);
            result = 0;
        }
    }

    return result;
}

void
gdbwire_destroy(struct gdbwire *gdbwire)
{
    free(gdbwire);
}

enum gdbwire_result
gdbwire_push_data(struct gdbwire *wire, const char *data, size_t size)
{
    enum gdbwire_result result;
    GDBWIRE_ASSERT(wire);
    result = gdbmi_parser_push_data(wire->parser, data, size);
    return result;
}

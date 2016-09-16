#include <stdlib.h>

#include "gdbwire_assert.h"
#include "gdbwire.h"
#include "gdbwire_mi_parser.h"

struct gdbwire
{
    /* The gdbwire_mi parser. */
    struct gdbwire_mi_parser *parser;

    /* The client callback functions */
    struct gdbwire_callbacks callbacks;
};

static void
gdbwire_mi_output_callback(void *context, struct gdbwire_mi_output *output) {
    struct gdbwire *wire = (struct gdbwire *)context;

    struct gdbwire_mi_output *cur = output;

    while (cur) {
        switch (cur->kind) {
            case GDBWIRE_MI_OUTPUT_OOB: {
                struct gdbwire_mi_oob_record *oob_record =
                    cur->variant.oob_record;
                switch (oob_record->kind) {
                    case GDBWIRE_MI_ASYNC:
                        if (wire->callbacks.gdbwire_async_record_fn) {
                            wire->callbacks.gdbwire_async_record_fn(
                                wire->callbacks.context,
                                    oob_record->variant.async_record);
                        }
                        break;
                    case GDBWIRE_MI_STREAM:
                        if (wire->callbacks.gdbwire_stream_record_fn) {
                            wire->callbacks.gdbwire_stream_record_fn(
                                wire->callbacks.context,
                                    oob_record->variant.stream_record);
                        }
                        break;
                }
                break;
            }
            case GDBWIRE_MI_OUTPUT_RESULT:
                if (wire->callbacks.gdbwire_result_record_fn) {
                    wire->callbacks.gdbwire_result_record_fn(
                        wire->callbacks.context, cur->variant.result_record);
                }
                break;
            case GDBWIRE_MI_OUTPUT_PROMPT:
                if (wire->callbacks.gdbwire_prompt_fn) {
                    wire->callbacks.gdbwire_prompt_fn(
                        wire->callbacks.context, cur->line);
                }
                break;
            case GDBWIRE_MI_OUTPUT_PARSE_ERROR:
                if (wire->callbacks.gdbwire_parse_error_fn) {
                    wire->callbacks.gdbwire_parse_error_fn(
                        wire->callbacks.context, cur->line,
                            cur->variant.error.token,
                                cur->variant.error.pos);
                }
                break;
        }

        cur = cur->next;
    }

    gdbwire_mi_output_free(output);
}

struct gdbwire *
gdbwire_create(struct gdbwire_callbacks callbacks)
{
    struct gdbwire *result = 0;
    
    result = malloc(sizeof(struct gdbwire));
    if (result) {
        struct gdbwire_mi_parser_callbacks parser_callbacks =
            { result,gdbwire_mi_output_callback };
        result->callbacks = callbacks;
        result->parser = gdbwire_mi_parser_create(parser_callbacks);
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
    if (gdbwire) {
        gdbwire_mi_parser_destroy(gdbwire->parser);
        free(gdbwire);
    }
}

enum gdbwire_result
gdbwire_push_data(struct gdbwire *wire, const char *data, size_t size)
{
    enum gdbwire_result result;
    GDBWIRE_ASSERT(wire);
    result = gdbwire_mi_parser_push_data(wire->parser, data, size);
    return result;
}

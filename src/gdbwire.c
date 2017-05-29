/**
 * Copyright (C) 2013 Robert Rossi <bob@brasko.net>
 *
 * This file is part of GDBWIRE.
 *
 * GDBWIRE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GDBWIRE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GDBWIRE.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>

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

struct gdbwire_interpreter_exec_context {
    enum gdbwire_result result;
    enum gdbwire_mi_command_kind kind;
    struct gdbwire_mi_command *mi_command;
};

static void gdbwire_interpreter_exec_stream_record(void *context,
    struct gdbwire_mi_stream_record *stream_record)
{
    struct gdbwire_interpreter_exec_context *ctx =
        (struct gdbwire_interpreter_exec_context*)context;
    ctx->result = GDBWIRE_LOGIC;
}

static void gdbwire_interpreter_exec_async_record(void *context,
    struct gdbwire_mi_async_record *async_record)
{
    struct gdbwire_interpreter_exec_context *ctx =
        (struct gdbwire_interpreter_exec_context*)context;
    ctx->result = GDBWIRE_LOGIC;
}

static void gdbwire_interpreter_exec_result_record(void *context,
    struct gdbwire_mi_result_record *result_record)
{
    struct gdbwire_interpreter_exec_context *ctx =
        (struct gdbwire_interpreter_exec_context*)context;

    if (ctx->result == GDBWIRE_OK) {
        ctx->result = gdbwire_get_mi_command(
            ctx->kind, result_record, &ctx->mi_command);
    }
}

static void gdbwire_interpreter_exec_prompt(void *context, const char *prompt)
{
    struct gdbwire_interpreter_exec_context *ctx =
        (struct gdbwire_interpreter_exec_context*)context;
    ctx->result = GDBWIRE_LOGIC;
}

static void gdbwire_interpreter_exec_parse_error(void *context,
        const char *mi, const char *token, struct gdbwire_mi_position
        position)
{
    struct gdbwire_interpreter_exec_context *ctx =
        (struct gdbwire_interpreter_exec_context*)context;
    ctx->result = GDBWIRE_LOGIC;
}


enum gdbwire_result
gdbwire_interpreter_exec(
        const char *interpreter_exec_output,
        enum gdbwire_mi_command_kind kind,
        struct gdbwire_mi_command **out_mi_command)
{
    struct gdbwire_interpreter_exec_context context = {
            GDBWIRE_OK, kind, 0 };
    size_t len;
    enum gdbwire_result result = GDBWIRE_OK;
    struct gdbwire_callbacks callbacks = {
        &context,
        gdbwire_interpreter_exec_stream_record,
        gdbwire_interpreter_exec_async_record,
        gdbwire_interpreter_exec_result_record,
        gdbwire_interpreter_exec_prompt,
        gdbwire_interpreter_exec_parse_error
    };
    struct gdbwire *wire;

    GDBWIRE_ASSERT(interpreter_exec_output);
    GDBWIRE_ASSERT(out_mi_command);

    len = strlen(interpreter_exec_output);

    wire = gdbwire_create(callbacks);
    GDBWIRE_ASSERT(wire);

    result = gdbwire_push_data(wire, interpreter_exec_output, len);
    if (result == GDBWIRE_OK) {
        /* Honor function documentation,
         * When it returns GDBWIRE_OK - the command will exist.
         * Otherwise it will not. */
        if (context.result == GDBWIRE_OK && !context.mi_command) {
            result = GDBWIRE_LOGIC;
        } else if (context.result != GDBWIRE_OK && context.mi_command) {
            result = context.result;
            gdbwire_mi_command_free(context.mi_command);
        } else {
            result = context.result;
            *out_mi_command = context.mi_command;
        }
    }

    gdbwire_destroy(wire);
    return result;
}

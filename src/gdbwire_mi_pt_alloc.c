/*
 * Copyright (C) 2014 Robert Rossi <bob@brasko.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>

#include "gdbwire_mi_pt.h"
#include "gdbwire_mi_pt_alloc.h"

/* struct gdbwire_mi_output */
struct gdbwire_mi_output *
gdbwire_mi_output_alloc(void)
{
    return calloc(1, sizeof (struct gdbwire_mi_output));
}

void
gdbwire_mi_output_free(struct gdbwire_mi_output *param)
{
    if (param) {
        switch (param->kind) {
            case GDBWIRE_MI_OUTPUT_OOB:
                gdbwire_mi_oob_record_free(param->variant.oob_record);
                param->variant.oob_record = NULL;
                break;
            case GDBWIRE_MI_OUTPUT_RESULT:
                gdbwire_mi_result_record_free(param->variant.result_record);
                param->variant.result_record = NULL;
                break;
            case GDBWIRE_MI_OUTPUT_PROMPT:
                break;
            case GDBWIRE_MI_OUTPUT_PARSE_ERROR:
                free(param->variant.error.token);
                param->variant.error.token = NULL;
                break;
        }

        free(param->line);
        param->line = 0;

        gdbwire_mi_output_free(param->next);
        param->next = NULL;

        free(param);
        param = NULL;
    }
}

/* struct gdbwire_mi_result_record */
struct gdbwire_mi_result_record *
gdbwire_mi_result_record_alloc(void)
{
    return calloc(1, sizeof (struct gdbwire_mi_result_record));
}

void
gdbwire_mi_result_record_free(struct gdbwire_mi_result_record *param)
{
    if (param) {
        free(param->token);

        gdbwire_mi_result_free(param->result);
        param->result = NULL;

        free(param);
        param = NULL;
    }
}

/* struct gdbwire_mi_result */
struct gdbwire_mi_result *
gdbwire_mi_result_alloc(void)
{
    return calloc(1, sizeof (struct gdbwire_mi_result));
}

void
gdbwire_mi_result_free(struct gdbwire_mi_result *param)
{
    if (param) {
        if (param->variable) {
            free(param->variable);
            param->variable = NULL;
        }

        switch (param->kind) {
            case GDBWIRE_MI_CSTRING:
                if (param->variant.cstring) {
                    free(param->variant.cstring);
                    param->variant.cstring = NULL;
                }
                break;
            case GDBWIRE_MI_TUPLE:
            case GDBWIRE_MI_LIST:
                gdbwire_mi_result_free(param->variant.result);
                param->variant.result = NULL;
                break;
        }

        gdbwire_mi_result_free(param->next);
        param->next = NULL;

        free(param);
        param = NULL;
    }
}

/* struct gdbwire_mi_oob_record */
struct gdbwire_mi_oob_record *
gdbwire_mi_oob_record_alloc(void)
{
    return calloc(1, sizeof (struct gdbwire_mi_oob_record));
}

void
gdbwire_mi_oob_record_free(struct gdbwire_mi_oob_record *param)
{
    if (param) {
        switch(param->kind) {
            case GDBWIRE_MI_ASYNC:
                gdbwire_mi_async_record_free(param->variant.async_record);
                param->variant.async_record = NULL;
                break;
            case GDBWIRE_MI_STREAM:
                gdbwire_mi_stream_record_free(param->variant.stream_record);
                param->variant.stream_record = NULL;
                break;
        }

        free(param);
        param = NULL;
    }
}

/* struct gdbwire_mi_async_record */
struct gdbwire_mi_async_record *
gdbwire_mi_async_record_alloc(void)
{
    return calloc(1, sizeof (struct gdbwire_mi_async_record));
}

void
gdbwire_mi_async_record_free(struct gdbwire_mi_async_record *param)
{
    if (param) {
        free(param->token);

        gdbwire_mi_result_free(param->result);
        param->result = NULL;

        free(param);
        param = NULL;
    }
}

/* struct gdbwire_mi_stream_record */
struct gdbwire_mi_stream_record *
gdbwire_mi_stream_record_alloc(void)
{
    return calloc(1, sizeof (struct gdbwire_mi_stream_record));
}

void
gdbwire_mi_stream_record_free(struct gdbwire_mi_stream_record *param)
{
    if (param) {
        if (param->cstring) {
            free(param->cstring);
            param->cstring = NULL;
        }

        free(param);
        param = NULL;
    }
}

#include <stdlib.h>

#include "gdbmi_pt.h"
#include "gdbmi_pt_alloc.h"

/* struct gdbmi_output */
struct gdbmi_output *
gdbmi_output_alloc(void)
{
    return calloc(1, sizeof (struct gdbmi_output));
}

void
gdbmi_output_free(struct gdbmi_output *param)
{
    if (param) {
        gdbmi_oob_record_free(param->oob_record);
        param->oob_record = NULL;

        gdbmi_result_record_free(param->result_record);
        param->result_record = NULL;

        gdbmi_output_free(param->next);
        param->next = NULL;

        free(param);
        param = NULL;
    }
}

/* struct gdbmi_result_record */
struct gdbmi_result_record *
gdbmi_result_record_alloc(void)
{
    return calloc(1, sizeof (struct gdbmi_result_record));
}

void
gdbmi_result_record_free(struct gdbmi_result_record *param)
{
    if (param) {
        gdbmi_result_free(param->result);
        param->result = NULL;

        free(param);
        param = NULL;
    }
}

/* struct gdbmi_result */
struct gdbmi_result *
gdbmi_result_alloc(void)
{
    return calloc(1, sizeof (struct gdbmi_result_record));
}

void
gdbmi_result_free(struct gdbmi_result *param)
{
    if (param) {
        if (param->variable) {
            free(param->variable);
            param->variable = NULL;
        }

        gdbmi_value_free(param->value);
        param->value = NULL;

        gdbmi_result_free(param->next);
        param->next = NULL;

        free(param);
        param = NULL;
    }
}

/* struct gdbmi_oob_record */
struct gdbmi_oob_record *
gdbmi_oob_record_alloc(void)
{
    return calloc(1, sizeof (struct gdbmi_oob_record));
}

void
gdbmi_oob_record_free(struct gdbmi_oob_record *param)
{
    if (param) {
        switch(param->kind) {
            case GDBMI_ASYNC:
                gdbmi_async_record_free(param->variant.async_record);
                param->variant.async_record = NULL;
                break;
            case GDBMI_STREAM:
                gdbmi_stream_record_free(param->variant.stream_record);
                param->variant.stream_record = NULL;
                break;
        }

        gdbmi_oob_record_free(param->next);
        param->next = NULL;

        free(param);
        param = NULL;
    }
}

/* struct gdbmi_async_record */
struct gdbmi_async_record *
gdbmi_async_record_alloc(void)
{
    return calloc(1, sizeof (struct gdbmi_async_record));
}

void
gdbmi_async_record_free(struct gdbmi_async_record *param)
{
    if (param) {
        gdbmi_async_output_free(param->async_output);
        param->async_output = NULL;

        free(param);
        param = NULL;
    }
}

/* struct gdbmi_async_output */
struct gdbmi_async_output *
gdbmi_async_output_alloc(void)
{
    return calloc(1, sizeof (struct gdbmi_async_output));
}

void
gdbmi_async_output_free(struct gdbmi_async_output *param)
{
    if (param) {
        gdbmi_result_free(param->result);
        param->result = NULL;

        free(param);
        param = NULL;
    }
}

/* struct gdbmi_value */
struct gdbmi_value *
gdbmi_value_alloc(void)
{
    return calloc(1, sizeof (struct gdbmi_value));
}

void
gdbmi_value_free(struct gdbmi_value *param)
{
    if (param) {
        switch (param->kind) {
            case GDBMI_CSTRING:
                if (param->variant.cstring) {
                    free(param->variant.cstring);
                    param->variant.cstring = NULL;
                }
                break;
            case GDBMI_TUPLE:
                gdbmi_tuple_free(param->variant.tuple);
                param->variant.tuple = NULL;
                break;
            case GDBMI_LIST:
                gdbmi_list_free(param->variant.list);
                param->variant.list = NULL;
                break;
        }

        gdbmi_value_free(param->next);
        param->next = NULL;

        free(param);
        param = NULL;
    }
}

/* struct gdbmi_value */
struct gdbmi_tuple *
gdbmi_tuple_alloc(void)
{
    return calloc(1, sizeof (struct gdbmi_tuple));
}

void
gdbmi_tuple_free(struct gdbmi_tuple *param)
{
    if (param) {
        gdbmi_result_free(param->result);
        param->result = NULL;

        free(param);
        param = NULL;
    }
}

/* struct gdbmi_list */
struct gdbmi_list *
gdbmi_list_alloc(void)
{
    return calloc(1, sizeof (struct gdbmi_list));
}

void
gdbmi_list_free(struct gdbmi_list *param)
{
    if (param) {
        switch (param->kind) {
            case GDBMI_VALUE:
                gdbmi_value_free(param->variant.value);
                param->variant.value = NULL;
                break;
            case GDBMI_RESULT:
                gdbmi_result_free(param->variant.result);
                param->variant.result = NULL;
                break;
        }

        gdbmi_list_free(param->next);
        param->next = NULL;

        free(param);
        param = NULL;
    }
}

/* struct gdbmi_stream_record */
struct gdbmi_stream_record *
gdbmi_stream_record_alloc(void)
{
    return calloc(1, sizeof (struct gdbmi_stream_record));
}

void
gdbmi_stream_record_free(struct gdbmi_stream_record *param)
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

#include <stdio.h>

#include "gdbmi_pt_print.h"

int print_token(long l)
{
    if (l == -1)
        return 0;
    printf("token(%ld)", l);
    return 0;
}

int print_gdbmi_result_class(enum gdbmi_result_class param)
{
    switch (param) {
        case GDBMI_DONE:
            printf("GDBMI_DONE\n");
            break;
        case GDBMI_RUNNING:
            printf("GDBMI_RUNNING\n");
            break;
        case GDBMI_CONNECTED:
            printf("GDBMI_CONNECTED\n");
            break;
        case GDBMI_ERROR:
            printf("GDBMI_ERROR\n");
            break;
        case GDBMI_EXIT:
            printf("EXIT\n");
            break;
        default:
            return -1;
    };

    return 0;
}

int print_gdbmi_output(struct gdbmi_output *param)
{
    struct gdbmi_output *cur = param;
    int result;

    while (cur) {
        result = print_gdbmi_oob_record(cur->oob_record);
        if (result == -1)
            return -1;

        result = print_gdbmi_result_record(cur->result_record);
        if (result == -1)
            return -1;

        cur = cur->next;
    }

    return 0;
}

/* Creating, Destroying and printing record  */
int print_gdbmi_result_record(struct gdbmi_result_record *param)
{
    int result;

    if (!param)
        return 0;

    result = print_token(param->token);
    if (result == -1)
        return -1;

    result = print_gdbmi_result_class(param->result_class);
    if (result == -1)
        return -1;

    result = print_gdbmi_result(param->result);
    if (result == -1)
        return -1;

    return 0;
}


int print_gdbmi_result(struct gdbmi_result *param)
{
    struct gdbmi_result *cur = param;
    int result;

    while (cur) {
        if (cur->variable) {
            printf("variable->(%s)\n", cur->variable);
        } else {
            printf("variable->(NULL)\n");
        }

        result = print_gdbmi_result_kind(cur->kind);
        if (result == -1)
            return -1;

        switch (cur->kind) {
            case GDBMI_CSTRING:
                printf("cstring->(%s)\n", cur->variant.cstring);
                break;
            case GDBMI_TUPLE:
                result = print_gdbmi_result(cur->variant.result);
                break;
            case GDBMI_LIST:
                result = print_gdbmi_result(cur->variant.result);
                break;
        }

        if (result == -1) {
            return -1;
        }

        cur = cur->next;
    }

    return 0;
}

int print_gdbmi_oob_record_kind(enum gdbmi_oob_record_kind param)
{
    switch (param) {
        case GDBMI_ASYNC:
            printf("GDBMI_ASYNC\n");
            break;
        case GDBMI_STREAM:
            printf("GDBMI_STREAM\n");
            break;
        default:
            return -1;
    };

    return 0;
}

int print_gdbmi_oob_record(struct gdbmi_oob_record *param)
{
    struct gdbmi_oob_record *cur = param;
    int result;

    while (cur) {
        result = print_gdbmi_oob_record_kind(cur->kind);
        if (result == -1)
            return -1;

        if (cur->kind == GDBMI_ASYNC) {
            result = print_gdbmi_async_record(cur->variant.async_record);
            if (result == -1)
                return -1;
        } else if (cur->kind == GDBMI_STREAM) {
            result = print_gdbmi_stream_record(cur->variant.stream_record);
            if (result == -1)
                return -1;
        } else
            return -1;

        cur = cur->next;
    }

    return 0;
}

int print_gdbmi_async_record_kind(enum gdbmi_async_record_kind param)
{
    switch (param) {
        case GDBMI_STATUS:
            printf("GDBMI_STATUS\n");
            break;
        case GDBMI_EXEC:
            printf("GDBMI_EXEC\n");
            break;
        case GDBMI_NOTIFY:
            printf("GDBMI_NOTIFY\n");
            break;
        default:
            return -1;
    };

    return 0;
}

int print_gdbmi_stream_record_kind(enum gdbmi_stream_record_kind param)
{
    switch (param) {
        case GDBMI_CONSOLE:
            printf("GDBMI_CONSOLE\n");
            break;
        case GDBMI_TARGET:
            printf("GDBMI_TARGET\n");
            break;
        case GDBMI_LOG:
            printf("GDBMI_LOG\n");
            break;
        default:
            return -1;
    };

    return 0;
}

/* Creating, Destroying and printing async_record  */
int print_gdbmi_async_record(struct gdbmi_async_record *param)
{
    int result;

    if (!param)
        return 0;

    result = print_token(param->token);
    if (result == -1)
        return -1;

    result = print_gdbmi_async_record_kind(param->kind);
    if (result == -1)
        return -1;

    result = print_gdbmi_async_output(param->async_output);
    if (result == -1)
        return -1;

    return 0;
}

/* Creating, Destroying and printing async_output  */
int print_gdbmi_async_output(struct gdbmi_async_output *param)
{
    int result;

    if (!param)
        return 0;

    result = print_gdbmi_async_class(param->async_class);
    if (result == -1)
        return -1;

    result = print_gdbmi_result(param->result);
    if (result == -1)
        return -1;

    return 0;
}

int print_gdbmi_async_class(enum gdbmi_async_class param)
{
    switch (param) {
        case GDBMI_ASYNC_STOPPED:
            printf("GDBMI_ASYNC_STOPPED\n");
            break;
        case GDBMI_ASYNC_RUNNING:
            printf("GDBMI_ASYNC_RUNNING\n");
            break;
        case GDBMI_ASYNC_DOWNLOAD:
            printf("GDBMI_ASYNC_DOWNLOAD\n");
            break;
        case GDBMI_ASYNC_UNSUPPORTED:
            printf("GDBMI_ASYNC_UNSUPPORTED\n");
            break;
    };

    return 0;
}

int print_gdbmi_result_kind(enum gdbmi_result_kind param)
{
    switch (param) {
        case GDBMI_CSTRING:
            printf("GDBMI_CSTRING\n");
            break;
        case GDBMI_TUPLE:
            printf("GDBMI_TUPLE\n");
            break;
        case GDBMI_LIST:
            printf("GDBMI_LIST\n");
            break;
    };

    return 0;
}

/* Creating, Destroying and printing stream_record  */
int print_gdbmi_stream_record(struct gdbmi_stream_record *param)
{
    int result;

    result = print_gdbmi_stream_record_kind(param->kind);
    if (result == -1)
        return -1;
    printf("cstring->(%s)\n", param->cstring);

    return 0;
}

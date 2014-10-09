#include <stdio.h>

#include "gdbmi_pt_print.h"

int print_token(gdbmi_token_t token)
{
    if (token) {
        printf("token(%s)", token);
    }
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
        switch (cur->kind) {
            case GDBMI_OUTPUT_OOB:
                result = print_gdbmi_oob_record(cur->variant.oob_record);
                if (result == -1) {
                    return -1;
                }
                break;
            case GDBMI_OUTPUT_RESULT:
                result = print_gdbmi_result_record(cur->variant.result_record);
                if (result == -1) {
                    return -1;
                }
                break;
            case GDBMI_OUTPUT_PROMPT:
                printf("(gdb)\n");
                break;
            case GDBMI_OUTPUT_PARSE_ERROR:
                printf("%s", param->variant.error.token);
                break;
        }

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
    int result;

    if (param) {
        result = print_gdbmi_oob_record_kind(param->kind);
        if (result == -1)
            return -1;

        if (param->kind == GDBMI_ASYNC) {
            result = print_gdbmi_async_record(param->variant.async_record);
            if (result == -1)
                return -1;
        } else if (param->kind == GDBMI_STREAM) {
            result = print_gdbmi_stream_record(param->variant.stream_record);
            if (result == -1)
                return -1;
        } else
            return -1;
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
        case GDBMI_ASYNC_DOWNLOAD:
            printf("GDBMI_ASYNC_DOWNLOAD\n");
            break;
        case GDBMI_ASYNC_STOPPED:
            printf("GDBMI_ASYNC_STOPPED\n");
            break;
        case GDBMI_ASYNC_RUNNING:
            printf("GDBMI_ASYNC_RUNNING\n");
            break;
        case GDBMI_ASYNC_THREAD_GROUP_ADDED:
            printf("GDBMI_ASYNC_THREAD_GROUP_ADDED\n");
            break;
        case GDBMI_ASYNC_THREAD_GROUP_REMOVED:
            printf("GDBMI_ASYNC_THREAD_GROUP_REMOVED\n");
            break;
        case GDBMI_ASYNC_THREAD_GROUP_STARTED:
            printf("GDBMI_ASYNC_THREAD_GROUP_STARTED\n");
            break;
        case GDBMI_ASYNC_THREAD_GROUP_EXITED:
            printf("GDBMI_ASYNC_THREAD_GROUP_EXITED\n");
            break;
        case GDBMI_ASYNC_THREAD_CREATED:
            printf("GDBMI_ASYNC_THREAD_CREATED\n");
            break;
        case GDBMI_ASYNC_THREAD_EXITED:
            printf("GDBMI_ASYNC_THREAD_EXITED\n");
            break;
        case GDBMI_ASYNC_THREAD_SELECTED:
            printf("GDBMI_ASYNC_THREAD_SELECTED\n");
            break;
        case GDBMI_ASYNC_LIBRARY_LOADED:
            printf("GDBMI_ASYNC_LIBRARY_LOADED\n");
            break;
        case GDBMI_ASYNC_LIBRARY_UNLOADED:
            printf("GDBMI_ASYNC_LIBRARY_UNLOADED\n");
            break;
        case GDBMI_ASYNC_TRACEFRAME_CHANGED:
            printf("GDBMI_ASYNC_TRACEFRAME_CHANGED\n");
            break;
        case GDBMI_ASYNC_TSV_CREATED:
            printf("GDBMI_ASYNC_TSV_CREATED\n");
            break;
        case GDBMI_ASYNC_TSV_MODIFIED:
            printf("GDBMI_ASYNC_TSV_MODIFIED\n");
            break;
        case GDBMI_ASYNC_TSV_DELETED:
            printf("GDBMI_ASYNC_TSV_DELETED\n");
            break;
        case GDBMI_ASYNC_BREAKPOINT_CREATED:
            printf("GDBMI_ASYNC_BREAKPOINT_CREATED\n");
            break;
        case GDBMI_ASYNC_BREAKPOINT_MODIFIED:
            printf("GDBMI_ASYNC_BREAKPOINT_MODIFIED\n");
            break;
        case GDBMI_ASYNC_BREAKPOINT_DELETED:
            printf("GDBMI_ASYNC_BREAKPOINT_DELETED\n");
            break;
        case GDBMI_ASYNC_RECORD_STARTED:
            printf("GDBMI_ASYNC_RECORD_STARTED\n");
            break;
        case GDBMI_ASYNC_RECORD_STOPPED:
            printf("GDBMI_ASYNC_RECORD_STOPPED\n");
            break;
        case GDBMI_ASYNC_CMD_PARAM_CHANGED:
            printf("GDBMI_ASYNC_CMD_PARAM_CHANGED\n");
            break;
        case GDBMI_ASYNC_MEMORY_CHANGED:
            printf("GDBMI_ASYNC_MEMORY_CHANGED\n");
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

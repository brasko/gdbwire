#include <stdio.h>
#include <stdlib.h>

#include "gdbmi_pt.h"

int print_token(long l)
{
    if (l == -1)
        return 0;
    printf("token(%ld)", l);
    return 0;
}

/* Print result class  */
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

/* Creating, Destroying and printing gdbmi_output  */
struct gdbmi_output *create_gdbmi_output(void)
{
    return calloc(1, sizeof (struct gdbmi_output));
}

int destroy_gdbmi_output(struct gdbmi_output *param)
{
    if (!param)
        return 0;

    if (destroy_gdbmi_oob_record(param->oob_record) == -1)
        return -1;
    param->oob_record = NULL;

    if (destroy_gdbmi_result_record(param->result_record) == -1)
        return -1;
    param->result_record = NULL;

    if (destroy_gdbmi_output(param->next) == -1)
        return -1;
    param->next = NULL;

    free(param);
    param = NULL;
    return 0;
}

struct gdbmi_output *
append_gdbmi_output(struct gdbmi_output *list, struct gdbmi_output *item)
{
    if (!item)
        return NULL;

    if (!list)
        list = item;
    else {
        struct gdbmi_output *cur = list;

        while (cur->next)
            cur = cur->next;

        cur->next = item;
    }

    return list;
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
struct gdbmi_result_record *create_gdbmi_result_record(void)
{
    return calloc(1, sizeof (struct gdbmi_result_record));
}

int destroy_gdbmi_result_record(struct gdbmi_result_record *param)
{
    if (!param)
        return 0;

    if (destroy_gdbmi_result(param->result) == -1)
        return -1;
    param->result = NULL;

    free(param);
    param = NULL;
    return 0;
}

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

/* Creating, Destroying and printing result  */
struct gdbmi_result *create_gdbmi_result(void)
{
    return calloc(1, sizeof (struct gdbmi_result_record));
}

int destroy_gdbmi_result(struct gdbmi_result *param)
{
    if (!param)
        return 0;

    if (param->variable) {
        free(param->variable);
        param->variable = NULL;
    }

    if (destroy_gdbmi_value(param->value) == -1)
        return -1;
    param->value = NULL;

    if (destroy_gdbmi_result(param->next) == -1)
        return -1;
    param->next = NULL;

    free(param);
    param = NULL;
    return 0;
}

struct gdbmi_result *
append_gdbmi_result(struct gdbmi_result *list, struct gdbmi_result *item)
{
    if (!item)
        return NULL;

    if (!list)
        list = item;
    else {
        struct gdbmi_result *cur = list;

        while (cur->next)
            cur = cur->next;

        cur->next = item;
    }

    return list;
}

int print_gdbmi_result(struct gdbmi_result *param)
{
    struct gdbmi_result *cur = param;
    int result;

    while (cur) {
        printf("variable->(%s)\n", cur->variable);

        result = print_gdbmi_value(cur->value);
        if (result == -1)
            return -1;

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

/* Creating, Destroying and printing oob_record  */
struct gdbmi_oob_record *create_gdbmi_oob_record(void)
{
    return calloc(1, sizeof (struct gdbmi_oob_record));
}

int destroy_gdbmi_oob_record(struct gdbmi_oob_record *param)
{
    if (!param)
        return 0;

    if (param->kind == GDBMI_ASYNC) {
        if (destroy_gdbmi_async_record(param->variant.async_record) == -1)
            return -1;
        param->variant.async_record = NULL;
    } else if (param->kind == GDBMI_STREAM) {
        if (destroy_gdbmi_stream_record(param->variant.stream_record) == -1)
            return -1;
        param->variant.stream_record = NULL;
    } else {
        return -1;
    }

    if (destroy_gdbmi_oob_record(param->next) == -1)
        return -1;
    param->next = NULL;

    free(param);
    param = NULL;
    return 0;
}

struct gdbmi_oob_record *
append_gdbmi_oob_record(struct gdbmi_oob_record *list,
        struct gdbmi_oob_record *item)
{
    if (!item)
        return NULL;

    if (!list)
        list = item;
    else {
        struct gdbmi_oob_record *cur = list;

        while (cur->next)
            cur = cur->next;

        cur->next = item;
    }

    return list;
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
struct gdbmi_async_record *create_gdbmi_async_record(void)
{
    return calloc(1, sizeof (struct gdbmi_async_record));
}

int destroy_gdbmi_async_record(struct gdbmi_async_record *param)
{
    if (!param)
        return 0;

    if (destroy_gdbmi_async_output(param->async_output) == -1)
        return -1;
    param->async_output = NULL;

    free(param);
    param = NULL;
    return 0;
}

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
struct gdbmi_async_output *create_gdbmi_async_output(void)
{
    return calloc(1, sizeof (struct gdbmi_async_output));
}

int destroy_gdbmi_async_output(struct gdbmi_async_output *param)
{
    if (!param)
        return 0;

    if (destroy_gdbmi_result(param->result) == -1)
        return -1;
    param->result = NULL;

    free(param);
    param = NULL;
    return 0;
}

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
        case GDBMI_STOPPED:
            printf("GDBMI_STOPPED\n");
            break;
        default:
            return -1;
    };

    return 0;
}

int print_gdbmi_value_kind(enum gdbmi_value_kind param)
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
        default:
            return -1;
    };

    return 0;
}

/* Creating, Destroying and printing value  */
struct gdbmi_value *create_gdbmi_value(void)
{
    return calloc(1, sizeof (struct gdbmi_value));
}

int destroy_gdbmi_value(struct gdbmi_value *param)
{
    if (!param)
        return 0;

    if (param->kind == GDBMI_CSTRING) {
        if (param->variant.cstring) {
            free(param->variant.cstring);
            param->variant.cstring = NULL;
        }
    } else if (param->kind == GDBMI_TUPLE) {
        if (destroy_gdbmi_tuple(param->variant.tuple) == -1)
            return -1;
        param->variant.tuple = NULL;
    } else if (param->kind == GDBMI_LIST) {
        if (destroy_gdbmi_list(param->variant.list) == -1)
            return -1;
        param->variant.list = NULL;
    } else
        return -1;

    if (destroy_gdbmi_value(param->next) == -1)
        return -1;
    param->next = NULL;

    free(param);
    param = NULL;
    return 0;
}

struct gdbmi_value *append_gdbmi_value(struct gdbmi_value *list,
        struct gdbmi_value *item)
{
    if (!item)
        return NULL;

    if (!list)
        list = item;
    else {
        struct gdbmi_value *cur = list;

        while (cur->next)
            cur = cur->next;

        cur->next = item;
    }

    return list;
}

int print_gdbmi_value(struct gdbmi_value *param)
{
    struct gdbmi_value *cur = param;
    int result;

    while (cur) {
        result = print_gdbmi_value_kind(cur->kind);
        if (result == -1)
            return -1;

        if (cur->kind == GDBMI_CSTRING) {
            printf("cstring->(%s)\n", cur->variant.cstring);
        } else if (cur->kind == GDBMI_TUPLE) {
            result = print_gdbmi_tuple(cur->variant.tuple);
            if (result == -1)
                return -1;
        } else if (cur->kind == GDBMI_LIST) {
            result = print_gdbmi_list(cur->variant.list);
            if (result == -1)
                return -1;
        } else
            return -1;

        cur = cur->next;
    }

    return 0;
}

/* Creating, Destroying and printing tuple  */
struct gdbmi_tuple *create_gdbmi_tuple(void)
{
    return calloc(1, sizeof (struct gdbmi_tuple));
}

int destroy_gdbmi_tuple(struct gdbmi_tuple *param)
{
    if (!param)
        return 0;

    if (destroy_gdbmi_result(param->result) == -1)
        return -1;
    param->result = NULL;

    free(param);
    param = NULL;
    return 0;
}

int print_gdbmi_tuple(struct gdbmi_tuple *param)
{
    int result;

    result = print_gdbmi_result(param->result);
    if (result == -1)
        return -1;

    return 0;
}

int print_gdbmi_list_kind(enum gdbmi_list_kind param)
{
    switch (param) {
        case GDBMI_VALUE:
            printf("GDBMI_VALUE\n");
            break;
        case GDBMI_RESULT:
            printf("GDBMI_RESULT\n");
            break;
        default:
            return -1;
    };

    return 0;
}

/* Creating, Destroying and printing list  */
struct gdbmi_list *create_gdbmi_list(void)
{
    return calloc(1, sizeof (struct gdbmi_list));
}

int destroy_gdbmi_list(struct gdbmi_list *param)
{
    if (!param)
        return 0;

    if (param->kind == GDBMI_VALUE) {
        if (destroy_gdbmi_value(param->variant.value) == -1)
            return -1;
        param->variant.value = NULL;
    } else if (param->kind == GDBMI_RESULT) {
        if (destroy_gdbmi_result(param->variant.result) == -1)
            return -1;
        param->variant.result = NULL;
    } else
        return -1;

    if (destroy_gdbmi_list(param->next) == -1)
        return -1;
    param->next = NULL;

    free(param);
    param = NULL;
    return 0;
}

struct gdbmi_list *append_gdbmi_list(struct gdbmi_list *list,
        struct gdbmi_list *item)
{
    if (!item)
        return NULL;

    if (!list)
        list = item;
    else {
        struct gdbmi_list *cur = list;

        while (cur->next)
            cur = cur->next;

        cur->next = item;
    }

    return list;
}

int print_gdbmi_list(struct gdbmi_list *param)
{
    struct gdbmi_list *cur = param;
    int result;

    while (cur) {
        result = print_gdbmi_list_kind(cur->kind);
        if (result == -1)
            return -1;

        if (cur->kind == GDBMI_VALUE) {
            result = print_gdbmi_value(cur->variant.value);
            if (result == -1)
                return -1;
        } else if (cur->kind == GDBMI_RESULT) {
            result = print_gdbmi_result(cur->variant.result);
            if (result == -1)
                return -1;
        } else
            return -1;

        cur = cur->next;
    }

    return 0;
}

/* Creating, Destroying and printing stream_record  */
struct gdbmi_stream_record *create_gdbmi_stream_record(void)
{
    return calloc(1, sizeof (struct gdbmi_stream_record));
}

int destroy_gdbmi_stream_record(struct gdbmi_stream_record *param)
{
    if (!param)
        return 0;

    if (param->cstring) {
        free(param->cstring);
        param->cstring = NULL;
    }

    free(param);
    param = NULL;
    return 0;
}

int print_gdbmi_stream_record(struct gdbmi_stream_record *param)
{
    int result;

    result = print_gdbmi_stream_record_kind(param->kind);
    if (result == -1)
        return -1;
    printf("cstring->(%s)\n", param->cstring);

    return 0;
}

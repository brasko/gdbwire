#include <stdio.h>
#include <stdlib.h>

#include "gdbmi_pt.h"

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

#include <stdio.h>
#include <stdlib.h>

#include "gdbwire_mi_pt.h"

struct gdbwire_mi_output *
append_gdbwire_mi_output(struct gdbwire_mi_output *list,
    struct gdbwire_mi_output *item)
{
    if (!item)
        return NULL;

    if (!list)
        list = item;
    else {
        struct gdbwire_mi_output *cur = list;

        while (cur->next)
            cur = cur->next;

        cur->next = item;
    }

    return list;
}

struct gdbwire_mi_result *
append_gdbwire_mi_result(struct gdbwire_mi_result *list,
    struct gdbwire_mi_result *item)
{
    if (!item)
        return NULL;

    if (!list)
        list = item;
    else {
        struct gdbwire_mi_result *cur = list;

        while (cur->next)
            cur = cur->next;

        cur->next = item;
    }

    return list;
}

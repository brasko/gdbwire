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

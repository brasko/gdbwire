#include <stdlib.h>

#include "gdbwire.h"

struct gdbwire
{
};

struct gdbwire *
gdbwire_create()
{
    struct gdbwire *result = malloc(sizeof(struct gdbwire));
    return result;
}

void
gdbwire_destroy(struct gdbwire *gdbwire)
{
    free(gdbwire);
}

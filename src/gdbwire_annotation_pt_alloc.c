#include <stdlib.h>

#include "gdbwire_annotation_pt.h"
#include "gdbwire_annotation_pt_alloc.h"

/* struct gdbwire_annotation_output */
struct gdbwire_annotation_output *
gdbwire_annotation_output_alloc(void)
{
    return calloc(1, sizeof (struct gdbwire_annotation_output));
}

void
gdbwire_annotation_output_free(struct gdbwire_annotation_output *param)
{
    if (param) {
        switch (param->kind) {
            case GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT:
                free(param->variant.console_output.text);
                break;
            case GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION:
                free(param->variant.annotation.text);
                break;
        }

        free(param);
        param = NULL;
    }
}

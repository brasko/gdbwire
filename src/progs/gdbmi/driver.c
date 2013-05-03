#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gdbmi/gdbmi_pt.h"
#include "gdbmi/gdbmi_parser.h"

static void usage(char *progname)
{

    printf("%s <file>\n", progname);
    exit(-1);
}

int main(int argc, char **argv)
{
    gdbmi_parser_ptr parser_ptr;
    struct gdbmi_output *output;
    int result, parse_failed;

    if (argc != 2)
        usage(argv[0]);

    parser_ptr = gdbmi_parser_create();

    result = gdbmi_parser_parse_file(parser_ptr,
            argv[1], &output, &parse_failed);

    if (result == -1) {
        fprintf(stderr, "%s:%d", __FILE__, __LINE__);
        return -1;
    }

    if (parse_failed) {
        if (result == -1) {
            fprintf(stderr, "%s:%d", __FILE__, __LINE__);
            return -1;
        }
    } else {
        print_gdbmi_output(output);
    }

    if (parse_failed) {
        output = NULL;
    } else {
        if (destroy_gdbmi_output(output) == -1) {
            fprintf(stderr, "%s:%d", __FILE__, __LINE__);
            return -1;
        }
    }

    gdbmi_parser_destroy(parser_ptr);

    return 0;
}

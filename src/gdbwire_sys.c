#include <stdlib.h>
#include <string.h>

#include "gdbwire_sys.h"

char *gdbwire_strdup(const char *str)
{
    char *result = NULL;

    if (str) {
        size_t length_to_allocate = strlen(str) + 1;
        result = malloc(length_to_allocate * sizeof(char));
        if (result) {
            strcpy(result, str);
        }
    }

    return result;
}

char *gdbwire_strndup(const char *str, size_t n)
{
    char *result = NULL;

    if (str) {
        size_t length_to_allocate = n + 1;
        result = malloc(length_to_allocate * sizeof(char));
        if (result) {
            strncpy(result, str, n);
        }
    }

    return result;
}

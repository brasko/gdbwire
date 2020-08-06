#include <string.h>
#include <stdlib.h>
#include "gdbwire_string.h"

struct gdbwire_string {
    /* The bytes that make up the string. May contain NUL characters. */
    char *data;
    /* The number of bytes occuring in data at the moment. */
    size_t size;
    /* The max capacity of the string */
    size_t capacity;
};

struct gdbwire_string *
gdbwire_string_create(void)
{
    struct gdbwire_string *string;

    string = calloc(1, sizeof (struct gdbwire_string));
    if (string) {
        if (gdbwire_string_append_cstr(string, "") == -1) {
            gdbwire_string_destroy(string);
            string = NULL;
        }
    }

    return string;
}

void
gdbwire_string_destroy(struct gdbwire_string *string)
{
    if (string) {
        if (string->data) {
            free(string->data);
            string->data = NULL;
        }
        string->size = 0;
        string->capacity = 0;
        free(string);
    }
}

void
gdbwire_string_clear(struct gdbwire_string *string)
{
    if (string) {
        string->size = 0;
        string->data[0] = '\0';
    }
}

/**
 * Increase the size of the string capacity.
 *
 * @param string
 * The string to increase the capacity.
 *
 * @return
 * 0 on success or -1 on error.
 */
static int
gdbwire_string_increase_capacity(struct gdbwire_string *string)
{
    /**
     * The algorithm chosen to increase the capacity is arbitrary.
     * It starts at 128 bytes. It then doubles it's size in bytes like this,
     *   128, 256, 512, 1024, 2048, 4096
     * After it reaches 4096 it then grows by 4096 bytes at a time.
     */
    if (string->capacity == 0) {
        string->capacity = 128;
    } else if (string->capacity < 4096) {
        string->capacity *= 2;
    } else {
        string->capacity += 4096;
    }

    /* At this point string->capacity is set to the new size, so realloc */
    string->data = (char*)realloc(string->data, string->capacity);

    return (string->data) ? 0 : -1;
}

int
gdbwire_string_append_char(struct gdbwire_string *string, char c)
{
    return gdbwire_string_append_data(string, &c, 1);
}

int
gdbwire_string_append_cstr(struct gdbwire_string *string, const char *cstr)
{
    int result;

    if (string && cstr) {
        size_t length = strlen(cstr) + 1;
        result = gdbwire_string_append_data(string, cstr, length);
        /* Do not include the NUL character in the size for NULL terminated
         * strings. This is documented in the interface. */
        if (result == 0) {
            string->size--;
        }
    } else {
        result = -1;
    }

    return result;
}

int
gdbwire_string_append_data(struct gdbwire_string *string, const char *data,
        size_t size)
{
    int result = (string && data) ? 0 : -1;
    size_t data_index = 0;

    for (; string && data && data_index < size; ++data_index, ++string->size) {
        if (string->size >= string->capacity) {
            result = gdbwire_string_increase_capacity(string);
            if (result == -1) {
                break;
            }
        }

        string->data[string->size] = data[data_index];
    }

    return result;
}

char *
gdbwire_string_data(struct gdbwire_string *string)
{
    char *result = NULL;

    if (string) {
        result = string->data;
    }

    return result;
}

size_t
gdbwire_string_size(struct gdbwire_string *string)
{
    return string->size;
}

size_t
gdbwire_string_capacity(struct gdbwire_string *string)
{
    return string->capacity;
}

size_t
gdbwire_string_find_first_of(struct gdbwire_string *string, const char *chars)
{
    size_t data_pos, data_size = 0;
    char *data_cur;
    const char *chars_cur;

    if (string && chars) {
        data_size = gdbwire_string_size(string);
        data_cur = gdbwire_string_data(string);

        for (data_pos = 0; data_pos < data_size; ++data_pos) {
            char data_c = data_cur[data_pos];
            for (chars_cur = chars; *chars_cur; ++chars_cur) {
                if (data_c == *chars_cur) {
                    return data_pos;
                }
            }
        }
    }

    return data_size;
}

int
gdbwire_string_erase(struct gdbwire_string *string, size_t pos, size_t count)
{
    int result = -1;

    if (string) {
        size_t count_erased = count;
        size_t data_size = gdbwire_string_size(string);
        char *data = gdbwire_string_data(string);

        /* The position index must be smaller than the data size to be valid */
        if (pos < data_size) {
            size_t from_pos = pos + count;

            /**
             * Check to see if anything needs to be copied.
             * If not, just null terminate the position to be erased
             * Null terminating the string ensures the c string and the data
             * string approach are both safe. In the data mode, the nul
             * character is unneeded.
             */
            if (from_pos >= data_size) {
                data[pos] = 0;
                count_erased = data_size - pos;
            /* If so, move characters from the from position
               to the to position */
            } else {
                char *to_cur = &data[pos], *from_cur = &data[from_pos];

                /* shift everything after the erase request to the left */
                for (; from_pos < data_size; ++from_pos, ++to_cur, ++from_cur) {
                    *to_cur = *from_cur;
                }
            }
            string->size -= count_erased;
            result = 0;
        }
    }

    return result;
}

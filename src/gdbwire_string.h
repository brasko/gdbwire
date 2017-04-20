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

#ifndef __GDBWIRE_STRING_H__
#define __GDBWIRE_STRING_H__

#ifdef __cplusplus 
extern "C" { 
#endif 

#include <stdlib.h>

/**
 * A dynamic string representation.
 *
 * To create and destroy a string use gdbwire_string_create() and
 * gdbwire_string_destroy() respectively.
 *
 * This string is an abstraction of a low level C string. It supports being
 * used as a NULL terminated c string and also as an arbitrary array of
 * bytes. You can append to this string in either of these modes using
 * gdbwire_string_append_cstr() or gdbwire_string_append_data(). This string
 * automatically grows as you append data to it. Please note, the size of
 * the string will not include the NULL terminated character when using
 * the gdbwire_string_append_cstr() function to append data.
 *
 * To get access to the underlying bytes associated with this string
 * call gdbwire_string_data(). It is OK to modify the result as long as
 * you are careful to stay in it's valid bounds.
 *
 * The size (or length) of the string can be accessed through the
 * gdbwire_string_size() function. The character pointer returned from
 * gdbwire_string_data() is valid from the index range of 0 to
 * gdbwire_string_size() - 1.
 */
struct gdbwire_string;

/**
 * Create a string instance.
 *
 * @return
 * A valid string instance or NULL on error.
 */
struct gdbwire_string *gdbwire_string_create(void);

/**
 * Destroy the string instance and it's resources.
 *
 * @param string
 * The string to destroy.
 */
void gdbwire_string_destroy(struct gdbwire_string *string);

/**
 * Clear the contents of a string.
 *
 * Sets the string back to an empty string which also changes it's
 * size back to zero.
 *
 * The capacity remains unchanged.
 *
 * @param string
 * The string to clear
 */
void gdbwire_string_clear(struct gdbwire_string *string);

/**
 * Append a c string to the string instance.
 *
 * @param string
 * The string instance to append the c string to.
 *
 * @param cstr
 * The c string to append to the string instance.
 *
 * @return
 * 0 on success or -1 on failure.
 */
int gdbwire_string_append_cstr(struct gdbwire_string *string, const char *cstr);

/**
 * Append a sequence of bytes to the string instance.
 *
 * @param string
 * The string instance to append the sequence of bytes to.
 *
 * @param data
 * The sequence of bytes to append to the string instance. This may
 * contain NUL characters.
 *
 * @param size
 * The number of bytes in data to append to the string instance.
 *
 * @return
 * 0 on success or -1 on failure.
 */
int gdbwire_string_append_data(struct gdbwire_string *string,
        const char *data, size_t size);

/**
 * Get the data associated with this string.
 *
 * The data could be formatted as a NULL terminated C string or
 * as an arbitrary array of bytes. Use gdbwire_string_size() to
 * determine the size (or length) of the result of this function.
 * 
 * Modifying the return value of this function is acceptable as long as you
 * stay in the string's valid bounds.
 *
 * @param string
 * The string index to get the pointer data from.
 *
 * @return
 * The data that has been added to this string instance or "" after
 * creation or clear. The result is gdbwire_string_size() bytes long.
 */
char *gdbwire_string_data(struct gdbwire_string *string);

/**
 * Determine the size (the number of bytes) this string instance represents.
 *
 * Please note, the result of this function will not include the NULL
 * terminated character when using the gdbwire_string_append_cstr() function
 * to append data.
 *
 * @param string
 * The string instance to get the size for.
 *
 * @return
 * The number of bytes contained in this string instance. To access these
 * bytes see gdbwire_string_data(). Will be 0 after creation or clear.
 */
size_t gdbwire_string_size(struct gdbwire_string *string);

/**
 * Determine the maximum capacity (number of bytes) this string may hold.
 *
 * The max capacity of the string is automatically increased when data
 * is appended to this string through the gdbwire_string_append_*()
 * family of functions.
 *
 * @param string
 * The string to determine the capacity of.
 *
 * @return
 * The max number of bytes this string may hold.
 */
size_t gdbwire_string_capacity(struct gdbwire_string *string);

/**
 * Search for the first character in chars occuring in this string.
 *
 * @param string
 * The string to search for the characters in chars in.
 *
 * @param chars
 * A null terminated string of characters. This string is not searched
 * for directly but instead each individually character in the string
 * is searched for.
 *
 * @return
 * The index position of the first matched character in chars.
 * Will return gdbwire_string_size() if not found.
 */
size_t gdbwire_string_find_first_of(struct gdbwire_string *string,
        const char *chars);

/**
 * Erase characters from this string, reducing it's size.
 *
 * @param string
 * The string to erase characters from.
 *
 * @param pos
 * The index position of the first character to be erased.
 *
 * @param count
 * The number of characters to erase starting at position pos.
 * If count goes past the end of the string it is adjusted to erase
 * until the end of the string. This allows the caller to pass in
 * gdbwire_string_size() to erase the end of the string with out
 * doing index arithmetic.
 * 
 * @return
 * On success 0 will be returned otherwise -1. The string will remain
 * unmodified when an error occurs. Success can only occur if the entire
 * requested range can be erased.
 */
int gdbwire_string_erase(struct gdbwire_string *string, size_t pos,
        size_t count);

#ifdef __cplusplus 
}
#endif 

#endif

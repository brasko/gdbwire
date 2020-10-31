#ifndef __GDBWIRE_SYS_H__
#define __GDBWIRE_SYS_H__

/**
 * Supporting system functions.
 */

#ifdef __cplusplus 
extern "C" { 
#endif 

/**
 * Duplicate a string.
 *
 * @param str
 * The string to duplicate
 *
 * @return
 * An allocated string that must be freed.
 * Null if out of memory or str is NULL.
 */
char *gdbwire_strdup(const char *str);

/**
 * Duplicate a string.
 *
 * @param str
 * The string to duplicate
 *
 * @param n
 * The number of characters to duplicate from str into the new string.
 *
 * @return
 * An allocated string that must be freed.
 * Null if out of memory or str is NULL.
 */
char *gdbwire_strndup(const char *str, size_t n);

#ifdef __cplusplus 
}
#endif 

#endif

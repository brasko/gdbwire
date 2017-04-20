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

#ifdef __cplusplus 
}
#endif 

#endif

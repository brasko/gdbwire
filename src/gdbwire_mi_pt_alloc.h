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

#ifndef GDBWIRE_MI_PT_ALLOC_H
#define GDBWIRE_MI_PT_ALLOC_H

#ifdef __cplusplus 
extern "C" { 
#endif 

/**
 * Responsible for allocating and deallocating gdbwire_mi_pt objects.
 */

/* struct gdbwire_mi_output */
struct gdbwire_mi_output *gdbwire_mi_output_alloc(void);
void gdbwire_mi_output_free(struct gdbwire_mi_output *param);

/* struct gdbwire_mi_result_record */
struct gdbwire_mi_result_record *gdbwire_mi_result_record_alloc(void);
void gdbwire_mi_result_record_free(struct gdbwire_mi_result_record *param);

/* struct gdbwire_mi_result */
struct gdbwire_mi_result *gdbwire_mi_result_alloc(void);
void gdbwire_mi_result_free(struct gdbwire_mi_result *param);

/* struct gdbwire_mi_oob_record */
struct gdbwire_mi_oob_record *gdbwire_mi_oob_record_alloc(void);
void gdbwire_mi_oob_record_free(struct gdbwire_mi_oob_record *param);

/* struct gdbwire_mi_async_record */
struct gdbwire_mi_async_record *gdbwire_mi_async_record_alloc(void);
void gdbwire_mi_async_record_free(struct gdbwire_mi_async_record *param);

/* struct gdbwire_mi_stream_record */
struct gdbwire_mi_stream_record *gdbwire_mi_stream_record_alloc(void);
void gdbwire_mi_stream_record_free(struct gdbwire_mi_stream_record *param);

#ifdef __cplusplus 
}
#endif 

#endif /* GDBWIRE_MI_PT_ALLOC_H */

#ifndef GDBMI_PT_ALLOC_H
#define GDBMI_PT_ALLOC_H

#include "gdbmi_pt.h"

#ifdef __cplusplus 
extern "C" { 
#endif 

int print_gdbmi_result_class(enum gdbmi_result_class param);
int print_gdbmi_output(struct gdbmi_output *param);
int print_gdbmi_result_record(struct gdbmi_result_record *param);
int print_gdbmi_result(struct gdbmi_result *param);
int print_gdbmi_oob_record_kind(enum gdbmi_oob_record_kind param);
int print_gdbmi_oob_record(struct gdbmi_oob_record *param);
int print_gdbmi_async_record_kind(enum gdbmi_async_record_kind param);
int print_gdbmi_stream_record_kind(enum gdbmi_stream_record_kind param);
int print_gdbmi_async_record(struct gdbmi_async_record *param);
int print_gdbmi_async_output(struct gdbmi_async_output *param);
int print_gdbmi_async_class(enum gdbmi_async_class param);
int print_gdbmi_result_kind(enum gdbmi_result_kind param);
int print_gdbmi_stream_record(struct gdbmi_stream_record *param);

#ifdef __cplusplus 
}
#endif 

#endif /* GDBMI_PT_ALLOC_H */

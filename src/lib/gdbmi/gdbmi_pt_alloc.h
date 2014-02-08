#ifndef GDBMI_PT_ALLOC_H
#define GDBMI_PT_ALLOC_H

#ifdef __cplusplus 
extern "C" { 
#endif 

/**
 * Responsible for allocating and deallocating gdbmi_pt objects.
 */

/* struct gdbmi_output */
struct gdbmi_output *gdbmi_output_alloc(void);
void gdbmi_output_free(struct gdbmi_output *param);

/* struct gdbmi_result_record */
struct gdbmi_result_record *gdbmi_result_record_alloc(void);
void gdbmi_result_record_free(struct gdbmi_result_record *param);

/* struct gdbmi_result */
struct gdbmi_result *gdbmi_result_alloc(void);
void gdbmi_result_free(struct gdbmi_result *param);

/* struct gdbmi_oob_record */
struct gdbmi_oob_record *gdbmi_oob_record_alloc(void);
void gdbmi_oob_record_free(struct gdbmi_oob_record *param);

/* struct gdbmi_async_record */
struct gdbmi_async_record *gdbmi_async_record_alloc(void);
void gdbmi_async_record_free(struct gdbmi_async_record *param);

/* struct gdbmi_async_output */
struct gdbmi_async_output *gdbmi_async_output_alloc(void);
void gdbmi_async_output_free(struct gdbmi_async_output *param);

/* struct gdbmi_value */
struct gdbmi_value *gdbmi_value_alloc(void);
void gdbmi_value_free(struct gdbmi_value *param);

/* struct gdbmi_value */
struct gdbmi_tuple *gdbmi_tuple_alloc(void);
void gdbmi_tuple_free(struct gdbmi_tuple *param);

/* struct gdbmi_list */
struct gdbmi_list *gdbmi_list_alloc(void);
void gdbmi_list_free(struct gdbmi_list *param);

/* struct gdbmi_stream_record */
struct gdbmi_stream_record *gdbmi_stream_record_alloc(void);
void gdbmi_stream_record_free(struct gdbmi_stream_record *param);

#ifdef __cplusplus 
}
#endif 

#endif /* GDBMI_PT_ALLOC_H */

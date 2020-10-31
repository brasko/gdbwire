#ifndef GDBWIRE_ANNOTATION_PT_ALLOC_H
#define GDBWIRE_ANNOTATION_PT_ALLOC_H

#ifdef __cplusplus 
extern "C" { 
#endif 

/**
 * Responsible for allocating and deallocating gdbwire_annotation_pt objects.
 */

/* struct gdbwire_annotation_output */
struct gdbwire_annotation_output *gdbwire_annotation_output_alloc(void);
void gdbwire_annotation_output_free(struct gdbwire_annotation_output *param);

#ifdef __cplusplus 
}
#endif 

#endif /* GDBWIRE_ANNOTATION_PT_ALLOC_H */

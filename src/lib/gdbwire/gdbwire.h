#ifndef GDBWIRE_H
#define GDBWIRE_H

#ifdef __cplusplus 
extern "C" { 
#endif 

/* The opaque gdbwire context */
struct gdbwire;

/**
 * Create a gdbwire context.
 *
 * @return
 * A new gdbwire instance or NULL on error.
 */
struct gdbwire *gdbwire_create(void);

/**
 * Destroy a gdbwire context.
 *
 * This function will do nothing if the instance is NULL.
 *
 * @param gdbwire
 * The instance of gdbwire to destroy
 */
void gdbwire_destroy(struct gdbwire *gdbwire);

#ifdef __cplusplus 
}
#endif 

#endif

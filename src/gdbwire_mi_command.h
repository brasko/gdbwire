#ifndef GDBWIRE_MI_COMMAND_H
#define GDBWIRE_MI_COMMAND_H

#ifdef __cplusplus 
extern "C" { 
#endif 

#include "gdbwire_result.h"
#include "gdbwire_mi_pt.h"

/**
 * An enumeration representing the supported GDB/MI commands.
 */
enum gdbwire_mi_command_kind {
    /* -file-list-exec-source-file */
    GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE,
    /* -file-list-exec-source-files */
    GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILES
};

/** A linked list of source files. */
struct gdbwire_mi_source_file {
    /** A relative path to a file, never NULL */
    char *file;
    /**An absolute path to a file, NULL if unavailable */
    char *fullname;
    /** The next file name or NULL if no more. */
    struct gdbwire_mi_source_file *next;
};

/**
 * Represents a GDB/MI command.
 */
struct gdbwire_mi_command {
    /**
     * The kind of mi command this represents.
     */
    enum gdbwire_mi_command_kind kind;

    union {
        /** When kind == GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE */
        struct {
            /**
             * The line number the inferior is currently executing at.
             */
            int line;

            /**
             * The filename the inferior is currently executing at.
             *
             * This is usually a relative path.
             */
            char *file;

            /**
             * The filename the inferior is currently executing at.
             *
             * This is an absolute path.
             *
             * This command was addd in 2004, however, it was possible
             * at the time that only the "file" field would be put out and
             * the "fullname" field would be omitted. In 2012, in git commit,
             * f35a17b5, gdb was changed to always omit the "fullname" field.
             */
            char *fullname;

            /**
             * Determines if the file includes preprocessor macro information.
             *
             * This command was added in 2004. However, the macro-info
             * field was added to the output in 2008 in git commit 17784837.
             *
             * Only check this field if macro_info_exists is true.
             */
            char macro_info:1;

            /** True if macro-info field was in mi output, otherwise false */
            char macro_info_exists:1;
        } file_list_exec_source_file;

        /** When kind == GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILES */
        struct {
            /**
             * A list of files that make up the inferior.
             *
             * When there are no files (if the gdb does not have an inferior
             * loaded) than files will be NULL.
             *
             * This command was addd in 2004, however, it was possible
             * at the time that only the "file" field would be put out and
             * the "fullname" field would be omitted. In 2012, in git commit,
             * f35a17b5, gdb was changed to always omit the "fullname" field.
             */
            struct gdbwire_mi_source_file *files;
        } file_list_exec_source_files;
        
    } variant;
};

/**
 * Get a gdbwire MI command from the result record.
 *
 * @param kind
 * The kind of command the result record is associated with.
 *
 * @param result_record
 * The result record to turn into a command.
 *
 * @param out_mi_command
 * Will return an allocated gdbwire mi command if GDBWIRE_OK is returned
 * from this function. You should free this memory with
 * gdbwire_mi_command_free when you are done with it.
 *
 * @return
 * The result of this function.
 */
enum gdbwire_result gdbwire_get_mi_command(
        enum gdbwire_mi_command_kind kind,
        struct gdbwire_mi_result_record *result_record,
        struct gdbwire_mi_command **out_mi_command);

/**
 * Free the gdbwire mi command.
 *
 * @param mi_command
 * The mi command to free.
 */
void gdbwire_mi_command_free(struct gdbwire_mi_command *mi_command);

#ifdef __cplusplus 
}
#endif 

#endif

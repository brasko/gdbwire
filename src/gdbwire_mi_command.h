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
    /* -break-info */
    GDBWIRE_MI_BREAK_INFO,

    /* -stack-info-frame */
    GDBWIRE_MI_STACK_INFO_FRAME,

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

/** The disposition of a breakpoint. What to do after hitting it. */
enum gdbwire_mi_breakpoint_disp_kind {
    GDBWIRE_MI_BP_DISP_DELETE,           /** Delete on next hit */
    GDBWIRE_MI_BP_DISP_DELETE_NEXT_STOP, /** Delete on next stop, hit or not */
    GDBWIRE_MI_BP_DISP_DISABLE,          /** Disable on next hit */
    GDBWIRE_MI_BP_DISP_KEEP,             /** Leave the breakpoint in place */
    GDBWIRE_MI_BP_DISP_UNKNOWN           /** When GDB doesn't specify */
};

/**
 * A linked list of breakpoints.
 *
 * A breakpoint is a breakpoint, a tracepoint, a watchpoing or a
 * catchpoint. The GDB breakpoint model is quite sophisticated.
 * This structure can be extended when necessary.
 */
struct gdbwire_mi_breakpoint {
    /**
     * The breakpoint number.
     *
     * An integer, however, for a breakpoint that represents one location of
     * a multiple location breakpoint, this will be a dotted pair, like ‘1.2’. 
     *
     * Never NULL.
     */
    char *number;

    /**
     * Determines if this is a multiple location breakpoint.
     *
     * True for a multi-location breakpoint, false otherwise.
     *
     * It is possible that a breakpoint corresponds to several locations in
     * your program. For example, several functions may have the same name.
     * For the following source code,
     *   int foo(int p) { return p; }
     *   double foo(double p) { return p; }
     *   int main() { int i = 1; double d = 2.3; return foo(i) + foo(d); }
     * If the user sets a breakpoint at foo by typing,
     *   b foo
     * Then gdb will create 3 breakpoints. The multiple location breakpoint,
     * which is the parent of the two breakpoints created for each foo
     * function. Here is the output of gdb from the CLI perspective,
     *   Num     Type           Disp Enb Address            What
     *   1       breakpoint     keep y   <MULTIPLE>         
     *   1.1                         y     0x4004dd in foo(int) at main.cpp:1
     *   1.2                         y     0x4004eb in foo(double) at main.cpp:2
     *
     * However, if the user created a breakpoint for main by typing,
     *   b main
     * Then gdb will only create a single breakpoint which would look like,
     *   1       breakpoint     keep y   0x4004fa in main() at main.cpp:3
     *
     * When this is true, the address field will be "<MULTIPLE>" and
     * the field multi_breakpoints will represent the breakpoints that this
     * multiple location breakpoint has created.
     */
    unsigned char multi:1;
    
    /**
     * True for breakpoints of a multi-location breakpoint, otherwise false.
     *
     * For the example above, 1.1 and 1.2 would have this field set true.
     *
     * When this is true, the field multi_breakpoint will represent
     * the multiple location breakpoint that has created this breakpoint.
     */
    unsigned char from_multi:1;

    /**
     * The breakpoint type.
     *
     * Typically "breakpoint", "watchpoint" or "catchpoint", but can be
     * a variety of different values. In gdb, see breakpoint.c:bptype_string
     * to see all the different possibilities.
     *
     * This will be NULL for breakpoints of a multiple location breakpoint.
     * In this circumstance, check the multi_breakpoint field for the
     * multiple location breakpoint type field.
     */
    char *type;

    /**
     * The type of the catchpoint or NULL if not a catch point.
     *
     * This field is only valid when the breakpoint is a catchpoint.
     * Unfortuntely, gdb says the "type" of the breakpoint in the type field
     * is "breakpoint" not "catchpoint". So if this field is non-NULL, it is
     * safe to assume that this breakpoint represents at catch point.
     */
    char *catch_type;

    /**
     * The breakpoint disposition.
     *
     * For multiple location breakpoints, this will be
     * GDBWIRE_MI_BP_DISP_UNKNOWN. In this circumstance, check the
     * multi_breakpoint field for the multiple location breakpoint
     * disposition field.
     */
    enum gdbwire_mi_breakpoint_disp_kind disposition;

    /** True if enabled or False if disabled. */
    unsigned char enabled:1;

    /**
     * The address of the breakpoint.
     *
     * This may be
     * - a hexidecimal number, representing the address
     * - the string ‘<PENDING>’ for a pending breakpoint
     * - the string ‘<MULTIPLE>’ for a breakpoint with multiple locations
     *
     * This field will be NULL if no address can be determined.
     * For example, a watchpoint does not have an address. 
     */
    char *address;

    /** 
     * The name of the function or NULL if unknown.
     */
    char *func_name;

    /**
     * A relative path to the file the breakpoint is in or NULL if unknown.
     */
    char *file;

    /**
     * An absolute path to the file the breakpoint is in or NULL if unknown.
     */
    char *fullname;

    /**
     * The line number the breakpoint is at or 0 if unkonwn.
     */
    unsigned long line;

    /**
     * The number of times this breakpoint has been hit.
     *
     * For breakpoints of multi-location breakpoints, this will be 0.
     * Look at the multi-location breakpoint field instead.
     */
    unsigned long times;

    /**
     * The location of the breakpoint as originally specified by the user. 
     *
     * This may be NULL for instance, for breakpoints for multi-breakpoints.
     */
    char *original_location;

    /**
     * True for a pending breakpoint, otherwise false.
     *
     * When this is true, the address field will be "<PENDING>".
     */
    unsigned char pending:1;

    /**
     * The breakpoints for a multi-location breakpoint.
     *
     * If multi is true, this will be the breakpoints associated with the
     * multiple location breakpoint. Otherwise will be NULL.
     */
    struct gdbwire_mi_breakpoint *multi_breakpoints;

    /**
     * A pointer to the multi location breakpoint that created this breakpoint.
     *
     * When the field from_multi is true, this will be a pointer to the
     * multi-location breakpoint that created this breakpoint. Otherwise NULL.
     *
     * For the example above in the multi field, breakpoints 1.1 and 1.2
     * would have this field pointing to the breakpoint 1.
     */
    struct gdbwire_mi_breakpoint *multi_breakpoint;


    /** The next breakpoint or NULL if no more. */
    struct gdbwire_mi_breakpoint *next;
};

struct gdbwire_mi_stack_frame {
    /**
     * The frame number.
     *
     * Where 0 is the topmost frame, i.e., the innermost function. 
     *
     * Always present.
     */
    unsigned level;

    /**
     * The address ($pc value) of the frame.
     *
     * May be NULL if GDB can not determine the frame address.
     */
    char *address;

   /**
    * The function name for the frame. May be NULL if unknown.
    */
   char *func;

   /**
    * The file name for the frame. May be NULL if unknown.
    */
   char *file;

   /**
    * The fullname name for the frame. May be NULL if unknown.
    */
   char *fullname;

   /**
    * Line number corresponding to the $pc. Maybe be 0 if unknown.
    */
   int line;

   /**
    * The shared library where this function is defined.
    *
    * This is only given if the frame's function is not known. 
    * May be NULL if unknown.
    */
   char *from;
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
        /** When kind == GDBWIRE_MI_BREAK_INFO */
        struct {
            /** The list of breakpoints, NULL if none exist.  */
            struct gdbwire_mi_breakpoint *breakpoints;
        } break_info;

        /** When kind == GDBWIRE_MI_STACK_INFO_FRAME */
        struct {
            struct gdbwire_mi_stack_frame *frame;
        } stack_info_frame;

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

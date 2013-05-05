#ifndef __GDBMI_OC_H__
#define __GDBMI_OC_H__

#include "gdbmi_pt.h"

/* The possible/implemented GDBMI commands */
enum gdbmi_input_command {
    /*  24.7 GDB/MI Program control */
    GDBMI_FILE_LIST_EXEC_SOURCE_FILE,
    GDBMI_FILE_LIST_EXEC_SOURCE_FILES,

    /*  24.5 GDB/MI Breakpoint table commands */
    GDBMI_BREAK_LIST,

    GDBMI_LAST
};

/* A cstring linked list for use by the gdbmi output commands */
struct gdbmi_oc_cstring_ll {
    /* The cstring */
    char *cstring;

    /* A pointer to the next output  */
    struct gdbmi_oc_cstring_ll *next;
};

/* A file path linked list, for use by the gdbmi output commands */
struct gdbmi_oc_file_path_info {
    /* The filename, relative path. */
    char *file;

    /* The fullname, absolute path. */
    char *fullname;

    /* A pointer to the next file path.  */
    struct gdbmi_oc_file_path_info *next;
};

enum breakpoint_type {
    GDBMI_BREAKPOINT,
    GDBMI_WATCHPOINT
};

enum breakpoint_disposition {
    GDBMI_KEEP,
    GDBMI_NOKEEP
};

struct gdbmi_oc_breakpoint {
    int number;
    enum breakpoint_type type;
    enum breakpoint_disposition disposition;
    /* 1 if enabled, otherwise 0 */
    int enabled;
    char *address;
    char *func;
    char *file;
    char *fullname;
    int line;
    int times;

    struct gdbmi_oc_breakpoint *next;
};

struct gdbmi_oc {
    /* If this is 1, then the command was asynchronous, otherwise it wasn't */
    int is_asynchronous;

    /* The console output. This is a null terminated list. */
    struct gdbmi_oc_cstring_ll *console_output;

    /* The GDBMI output command this represents. If set to GDBMI_LAST,
     * then this is an asynchronous command. */
    enum gdbmi_input_command input_command;

    union {

        /*  24.7 GDB/MI Program control */
        struct {
            int line;
            char *file;
            char *fullname;
        } file_list_exec_source_file;

        struct {
            struct gdbmi_oc_file_path_info *file_name_pair;
        } file_list_exec_source_files;

        /*  24.5 GDB/MI Breakpoint table commands */
        struct {
            struct gdbmi_oc_breakpoint *breakpoint;
        } break_list;
    } input_commands;

    /* The next MI output command */
    struct gdbmi_oc *next;
};

/**
 * This will take in a parse tree and return a list of MI output commands.
 *
 * \param output
 * The MI parse tree
 *
 * \param mi_input_cmds
 * The next MI input command.
 *
 * \param oc
 * On return, this will be the MI output commands that were derived from the 
 * parse tree.
 * 
 * \return
 * 0 on success, -1 on error.
 */
int
gdbmi_get_output_commands(struct gdbmi_output *output,
        struct gdbmi_oc_cstring_ll *mi_input_cmds, struct gdbmi_oc **oc);

/* Creating, Destroying and printing MI output commands  */
struct gdbmi_oc *create_gdbmi_oc(void);
int destroy_gdbmi_oc(struct gdbmi_oc *param);
struct gdbmi_oc *append_gdbmi_oc(struct gdbmi_oc *list, struct gdbmi_oc *item);
int print_gdbmi_oc(struct gdbmi_oc *param);

/* Creating, Destroying and printing MI cstring linked lists */
struct gdbmi_oc_cstring_ll *create_gdbmi_cstring_ll(void);
int destroy_gdbmi_cstring_ll(struct gdbmi_oc_cstring_ll *param);
struct gdbmi_oc_cstring_ll *append_gdbmi_cstring_ll(
        struct gdbmi_oc_cstring_ll *list, struct gdbmi_oc_cstring_ll *item);
int print_gdbmi_cstring_ll(struct gdbmi_oc_cstring_ll *param);

/* Creating, Destroying and printing MI file_path linked lists */
struct gdbmi_oc_file_path_info *create_gdbmi_file_path_info(void);
int destroy_gdbmi_file_path_info(struct gdbmi_oc_file_path_info *param);
struct gdbmi_oc_file_path_info *
append_gdbmi_file_path_info(struct gdbmi_oc_file_path_info *list,
        struct gdbmi_oc_file_path_info *item);
int print_gdbmi_file_path_info(struct gdbmi_oc_file_path_info *param);

/* Creating, Destroying and printing MI breakpoint_list linked lists */
struct gdbmi_oc_breakpoint *create_gdbmi_breakpoint(void);
int destroy_gdbmi_breakpoint(struct gdbmi_oc_breakpoint *param);
struct gdbmi_oc_breakpoint *append_gdbmi_breakpoint(
        struct gdbmi_oc_breakpoint *list, struct gdbmi_oc_breakpoint *item);
int print_gdbmi_breakpoint(struct gdbmi_oc_breakpoint *param);

#endif /* __GDBMI_OC_H__ */

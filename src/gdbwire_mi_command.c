#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "gdbwire_sys.h"
#include "gdbwire_assert.h"
#include "gdbwire_mi_command.h"

/**
 * Free a source file list.
 *
 * @param files
 * The source file list to free, OK to pass in NULL.
 */
static void
gdbwire_mi_source_files_free(struct gdbwire_mi_source_file *files)
{
    struct gdbwire_mi_source_file *tmp, *cur = files;
    while (cur) {
        free(cur->file);
        free(cur->fullname);
        tmp = cur;
        cur = cur->next;
        free(tmp);
    }
}

/**
 * Free a breakpoint list.
 *
 * @param breakpoints
 * The breakpoint list to free, OK to pass in NULL.
 */
static void
gdbwire_mi_breakpoints_free(struct gdbwire_mi_breakpoint *breakpoints)
{
    struct gdbwire_mi_breakpoint *tmp, *cur = breakpoints;
    while (cur) {
        free(cur->original_location);
        free(cur->fullname);
        free(cur->file);
        free(cur->func_name);
        free(cur->address);
        free(cur->catch_type);
        free(cur->type);
        free(cur->number);

        gdbwire_mi_breakpoints_free(cur->multi_breakpoints);
        cur->multi_breakpoint = 0;

        tmp = cur;
        cur = cur->next;
        free(tmp);
    }
}

/**
 * Free a stack frame.
 *
 * @param frame
 * The frame to free, OK to pass in NULL.
 */
static void
gdbwire_mi_stack_frame_free(struct gdbwire_mi_stack_frame *frame)
{
    free(frame->address);
    free(frame->func);
    free(frame->file);
    free(frame->fullname);
    free(frame->from);
    free(frame);
}

/**
 * Convert a string to an unsigned long.
 *
 * @param str
 * The string to convert.
 *
 * @param num
 * If GDBWIRE_OK is returned, this will be returned as the number.
 *
 * @return
 * GDBWIRE_OK on success, and num is valid, or GDBWIRE_LOGIC on failure.
 */
static enum gdbwire_result
gdbwire_string_to_ulong(char *str, unsigned long *num)
{
    enum gdbwire_result result = GDBWIRE_LOGIC;
    unsigned long int strtol_result;
    char *end_ptr;

    GDBWIRE_ASSERT(str);
    GDBWIRE_ASSERT(num);

    errno = 0;
    strtol_result = strtoul(str, &end_ptr, 10);
    if (errno == 0 && str != end_ptr && *end_ptr == '\0') {
        *num = strtol_result;
        result = GDBWIRE_OK;
    }

    return result;
}

/**
 * Handle breakpoints from the -break-info command.
 *
 * @param mi_result
 * The mi parse tree starting from bkpt={...}
 *
 * @param bkpt
 * Allocated breakpoint on way out on success. Otherwise NULL on way out.
 *
 * @return
 * GDBWIRE_OK on success and bkpt is an allocated breakpoint. Otherwise
 * the appropriate error code and bkpt will be NULL.
 */
static enum gdbwire_result
break_info_for_breakpoint(struct gdbwire_mi_result *mi_result,
        struct gdbwire_mi_breakpoint **bkpt)
{
    enum gdbwire_result result = GDBWIRE_OK;

    struct gdbwire_mi_breakpoint *breakpoint = 0;

    char *number = 0;
    int multi = 0;
    int from_multi = 0;
    char *catch_type = 0;
    int pending = 0;
    int enabled = 0;
    char *address = 0;
    char *type = 0;
    enum gdbwire_mi_breakpoint_disp_kind disp_kind = GDBWIRE_MI_BP_DISP_UNKNOWN;
    char *func_name = 0;
    char *file = 0;
    char *fullname = 0;
    unsigned long line = 0;
    unsigned long times = 0;
    char *original_location = 0;

    GDBWIRE_ASSERT(mi_result);
    GDBWIRE_ASSERT(bkpt);

    *bkpt = 0;

    while (mi_result) {
        if (mi_result->kind == GDBWIRE_MI_CSTRING) {
            if (strcmp(mi_result->variable, "number") == 0) {
                number = mi_result->variant.cstring;

                if (strstr(number, ".") != NULL) {
                    from_multi = 1;
                }
            } else if (strcmp(mi_result->variable, "enabled") == 0) {
                enabled = mi_result->variant.cstring[0] == 'y';
            } else if (strcmp(mi_result->variable, "addr") == 0) {
                multi = strcmp(mi_result->variant.cstring, "<MULTIPLE>") == 0;
                pending = strcmp(mi_result->variant.cstring, "<PENDING>") == 0;
                address = mi_result->variant.cstring;
            } else if (strcmp(mi_result->variable, "catch-type") == 0) {
                catch_type = mi_result->variant.cstring;
            } else if (strcmp(mi_result->variable, "type") == 0) {
                type = mi_result->variant.cstring;
            } else if (strcmp(mi_result->variable, "disp") == 0) {
                if (strcmp(mi_result->variant.cstring, "del") == 0) {
                    disp_kind = GDBWIRE_MI_BP_DISP_DELETE;
                } else if (strcmp(mi_result->variant.cstring, "dstp") == 0) {
                    disp_kind = GDBWIRE_MI_BP_DISP_DELETE_NEXT_STOP;
                } else if (strcmp(mi_result->variant.cstring, "dis") == 0) {
                    disp_kind = GDBWIRE_MI_BP_DISP_DISABLE;
                } else if (strcmp(mi_result->variant.cstring, "keep") == 0) {
                    disp_kind = GDBWIRE_MI_BP_DISP_KEEP;
                } else {
                    return GDBWIRE_LOGIC;
                }
            } else if (strcmp(mi_result->variable, "func") == 0) {
                func_name = mi_result->variant.cstring;
            } else if (strcmp(mi_result->variable, "file") == 0) {
                file = mi_result->variant.cstring;
            } else if (strcmp(mi_result->variable, "fullname") == 0) {
                fullname = mi_result->variant.cstring;
            } else if (strcmp(mi_result->variable, "line") == 0) {
                GDBWIRE_ASSERT(gdbwire_string_to_ulong(
                        mi_result->variant.cstring, &line) == GDBWIRE_OK);
            } else if (strcmp(mi_result->variable, "times") == 0) {
                GDBWIRE_ASSERT(gdbwire_string_to_ulong(
                        mi_result->variant.cstring, &times) == GDBWIRE_OK);
            } else if (strcmp(mi_result->variable, "original-location") == 0) {
                original_location = mi_result->variant.cstring;
            }
        }

        mi_result = mi_result->next;
    }

    /* Validate required fields before proceeding. */
    GDBWIRE_ASSERT(number);

    /* At this point, allocate a breakpoint */
    breakpoint = calloc(1, sizeof(struct gdbwire_mi_breakpoint));
    if (!breakpoint) {
        return GDBWIRE_NOMEM;
    }

    breakpoint->multi = multi;
    breakpoint->from_multi = from_multi;
    breakpoint->number = gdbwire_strdup(number);
    breakpoint->type = (type)?gdbwire_strdup(type):0;
    breakpoint->catch_type = (catch_type)?gdbwire_strdup(catch_type):0;
    breakpoint->disposition = disp_kind;
    breakpoint->enabled = enabled;
    breakpoint->address = (address)?gdbwire_strdup(address):0;
    breakpoint->func_name = (func_name)?gdbwire_strdup(func_name):0;
    breakpoint->file = (file)?gdbwire_strdup(file):0;
    breakpoint->fullname = (fullname)?gdbwire_strdup(fullname):0;
    breakpoint->line = line;
    breakpoint->times = times;
    breakpoint->original_location =
        (original_location)?gdbwire_strdup(original_location):0;
    breakpoint->pending = pending;

    /* Handle the out of memory situation */
    if (!breakpoint->number ||
        (type && !breakpoint->type) ||
        (catch_type && !breakpoint->catch_type) ||
        (address && !breakpoint->address) ||
        (func_name && !breakpoint->func_name) ||
        (file && !breakpoint->file) ||
        (fullname && !breakpoint->fullname) ||
        (original_location && !breakpoint->original_location)) {
        gdbwire_mi_breakpoints_free(breakpoint);
        breakpoint = 0;
        result = GDBWIRE_NOMEM;
    }

    *bkpt = breakpoint;

    return result;
}

/**
 * Handle the -break-info command.
 *
 * @param result_record
 * The mi result record that makes up the command output from gdb.
 *
 * @param out
 * The output command, null on error.
 *
 * @return
 * GDBWIRE_OK on success, otherwise failure and out is NULL.
 */
static enum gdbwire_result
break_info(
    struct gdbwire_mi_result_record *result_record,
    struct gdbwire_mi_command **out)
{
    enum gdbwire_result result = GDBWIRE_OK;
    struct gdbwire_mi_result *mi_result;
    struct gdbwire_mi_command *mi_command = 0;
    struct gdbwire_mi_breakpoint *breakpoints = 0, *cur_bkpt;
    int found_body = 0;

    GDBWIRE_ASSERT(result_record);
    GDBWIRE_ASSERT(out);

    *out = 0;

    GDBWIRE_ASSERT(result_record->result_class == GDBWIRE_MI_DONE);
    GDBWIRE_ASSERT(result_record->result);

    mi_result = result_record->result;

    GDBWIRE_ASSERT(mi_result->kind == GDBWIRE_MI_TUPLE);
    GDBWIRE_ASSERT(strcmp(mi_result->variable, "BreakpointTable") == 0);
    GDBWIRE_ASSERT(mi_result->variant.result);
    GDBWIRE_ASSERT(!mi_result->next);
    mi_result = mi_result->variant.result;

    /* Fast forward to the body */
    while (mi_result) {
        if (mi_result->kind == GDBWIRE_MI_LIST &&
            strcmp(mi_result->variable, "body") == 0) {
            found_body = 1;
            break;
        } else {
            mi_result = mi_result->next;
        }
    }

    GDBWIRE_ASSERT(found_body);
    GDBWIRE_ASSERT(!mi_result->next);
    mi_result = mi_result->variant.result;

    while (mi_result) {
        struct gdbwire_mi_breakpoint *bkpt;
        GDBWIRE_ASSERT_GOTO(
            mi_result->kind == GDBWIRE_MI_TUPLE, result, cleanup);

        /**
         * GDB emits non-compliant MI when sending breakpoint information.
         *   https://sourceware.org/bugzilla/show_bug.cgi?id=9659
         * In particular, instead of saying
         *   bkpt={...},bkpt={...}
         * it puts out,
         *   bkpt={...},{...}
         * skipping the additional bkpt for subsequent breakpoints. I've seen
         * this output for multiple location breakpoints as the bug points to.
         *
         * For this reason, only check bkpt for the first breakpoint and
         * assume it is true for the remaining.
         */
        if (mi_result->variable) {
            GDBWIRE_ASSERT_GOTO(
                strcmp(mi_result->variable, "bkpt") == 0, result, cleanup);
        }

        result = break_info_for_breakpoint(mi_result->variant.result, &bkpt);
        if (result != GDBWIRE_OK) {
            goto cleanup;
        }

        if (bkpt->from_multi) {

            bkpt->multi_breakpoint = cur_bkpt;

            /* Append breakpoint to the multiple location breakpoints */
            if (cur_bkpt->multi_breakpoints) {
                struct gdbwire_mi_breakpoint *multi =
                    cur_bkpt->multi_breakpoints;
                while (multi->next) {
                    multi = multi->next;
                }
                multi->next = bkpt;
            } else {
                cur_bkpt->multi_breakpoints = bkpt;
            }
        } else {
            /* Append breakpoint to the list of breakpoints */
            if (breakpoints) {
                cur_bkpt->next = bkpt;
                cur_bkpt = cur_bkpt->next;
            } else {
                breakpoints = cur_bkpt = bkpt;
            }
        }

        mi_result = mi_result->next;
    }

    mi_command = calloc(1, sizeof(struct gdbwire_mi_command));
    if (!mi_command) {
        result = GDBWIRE_NOMEM;
        goto cleanup;
    }
    mi_command->variant.break_info.breakpoints = breakpoints;

    *out = mi_command;

    return result;

cleanup:
    gdbwire_mi_breakpoints_free(breakpoints);
    return result;
}

/**
 * Handle the -stack-info-frame command.
 *
 * @param result_record
 * The mi result record that makes up the command output from gdb.
 *
 * @param out
 * The output command, null on error.
 *
 * @return
 * GDBWIRE_OK on success, otherwise failure and out is NULL.
 */
static enum gdbwire_result
stack_info_frame(
    struct gdbwire_mi_result_record *result_record,
    struct gdbwire_mi_command **out)
{
    struct gdbwire_mi_stack_frame *frame;
    struct gdbwire_mi_result *mi_result;
    struct gdbwire_mi_command *mi_command = 0;

    char *level = 0, *address = 0;
    char *func = 0, *file = 0, *fullname = 0, *line = 0, *from = 0;

    *out = 0;

    GDBWIRE_ASSERT(result_record->result_class == GDBWIRE_MI_DONE);
    GDBWIRE_ASSERT(result_record->result);

    mi_result = result_record->result;

    GDBWIRE_ASSERT(mi_result->kind == GDBWIRE_MI_TUPLE);
    GDBWIRE_ASSERT(strcmp(mi_result->variable, "frame") == 0);
    GDBWIRE_ASSERT(mi_result->variant.result);
    GDBWIRE_ASSERT(!mi_result->next);
    mi_result = mi_result->variant.result;

    while (mi_result) {
        if (mi_result->kind == GDBWIRE_MI_CSTRING) {
            if (strcmp(mi_result->variable, "level") == 0) {
                level = mi_result->variant.cstring;
            } else if (strcmp(mi_result->variable, "addr") == 0) {
                address = mi_result->variant.cstring;
            } else if (strcmp(mi_result->variable, "func") == 0) {
                func = mi_result->variant.cstring;
            } else if (strcmp(mi_result->variable, "file") == 0) {
                file = mi_result->variant.cstring;
            } else if (strcmp(mi_result->variable, "fullname") == 0) {
                fullname = mi_result->variant.cstring;
            } else if (strcmp(mi_result->variable, "line") == 0) {
                line = mi_result->variant.cstring;
            } else if (strcmp(mi_result->variable, "from") == 0) {
                from = mi_result->variant.cstring;
            }
        }

        mi_result = mi_result->next;
    }

    GDBWIRE_ASSERT(level && address);

    if (strcmp(address, "<unavailable>") == 0) {
        address = 0;
    }

    frame = calloc(1, sizeof(struct gdbwire_mi_stack_frame));
    if (!frame) {
        return GDBWIRE_NOMEM;
    }

    frame->level = atoi(level);
    frame->address = (address)?gdbwire_strdup(address):0;
    frame->func = (func)?gdbwire_strdup(func):0;
    frame->file = (file)?gdbwire_strdup(file):0;
    frame->fullname = (fullname)?gdbwire_strdup(fullname):0;
    frame->line = (line)?atoi(line):0;
    frame->from = (from)?gdbwire_strdup(from):0;

    /* Handle the out of memory situation */
    if ((address && !frame->address) ||
        (func && !frame->func) ||
        (file && !frame->file) ||
        (fullname && !frame->fullname) ||
        (from && !frame->from)) {
        gdbwire_mi_stack_frame_free(frame);
        return GDBWIRE_NOMEM;
    }

    mi_command = calloc(1, sizeof(struct gdbwire_mi_command));
    if (!mi_command) {
        gdbwire_mi_stack_frame_free(frame);
        return GDBWIRE_NOMEM;
    }
    mi_command->kind = GDBWIRE_MI_STACK_INFO_FRAME;
    mi_command->variant.stack_info_frame.frame = frame;

    *out = mi_command;

    return GDBWIRE_OK;
}

/**
 * Handle the -file-list-exec-source-file command.
 *
 * @param result_record
 * The mi result record that makes up the command output from gdb.
 *
 * @param out
 * The output command, null on error.
 *
 * @return
 * GDBWIRE_OK on success, otherwise failure and out is NULL.
 */
static enum gdbwire_result
file_list_exec_source_file(
    struct gdbwire_mi_result_record *result_record,
    struct gdbwire_mi_command **out)
{
    struct gdbwire_mi_result *mi_result;
    struct gdbwire_mi_command *mi_command = 0;

    char *line = 0, *file = 0, *fullname = 0, *macro_info = 0;

    *out = 0;

    GDBWIRE_ASSERT(result_record->result_class == GDBWIRE_MI_DONE);
    GDBWIRE_ASSERT(result_record->result);

    mi_result = result_record->result;

    while (mi_result) {
        if (mi_result->kind == GDBWIRE_MI_CSTRING) {
            if (strcmp(mi_result->variable, "line") == 0) {
                line = mi_result->variant.cstring;
            } else if (strcmp(mi_result->variable, "file") == 0) {
                file = mi_result->variant.cstring;
            } else if (strcmp(mi_result->variable, "fullname") == 0) {
                fullname = mi_result->variant.cstring;
            } else if (strcmp(mi_result->variable, "macro-info") == 0) {
                macro_info = mi_result->variant.cstring;
                GDBWIRE_ASSERT(strlen(macro_info) == 1);
                GDBWIRE_ASSERT(macro_info[0] == '0' || macro_info[0] == '1');
            }
        }

        mi_result = mi_result->next;
    }

    GDBWIRE_ASSERT(line && file);

    mi_command = calloc(1, sizeof(struct gdbwire_mi_command));
    if (!mi_command) {
        return GDBWIRE_NOMEM;
    }

    mi_command->kind = GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE;
    mi_command->variant.file_list_exec_source_file.line = atoi(line);
    mi_command->variant.file_list_exec_source_file.file = gdbwire_strdup(file);
    if (!mi_command->variant.file_list_exec_source_file.file) {
        gdbwire_mi_command_free(mi_command);
        return GDBWIRE_NOMEM;
    }
    mi_command->variant.file_list_exec_source_file.fullname =
        (fullname)?gdbwire_strdup(fullname):0;
    if (fullname &&
        !mi_command->variant.file_list_exec_source_file.fullname) {
        gdbwire_mi_command_free(mi_command);
        return GDBWIRE_NOMEM;
    }
    mi_command->variant.file_list_exec_source_file.macro_info_exists =
        macro_info != 0;
    if (macro_info) {
        mi_command->variant.file_list_exec_source_file.macro_info =
            atoi(macro_info);
    }

    *out = mi_command;

    return GDBWIRE_OK;
}

/**
 * Handle the -file-list-exec-source-files command.
 *
 * @param result_record
 * The mi result record that makes up the command output from gdb.
 *
 * @param out
 * The output command, null on error.
 *
 * @return
 * GDBWIRE_OK on success, otherwise failure and out is NULL.
 */
static enum gdbwire_result
file_list_exec_source_files(
    struct gdbwire_mi_result_record *result_record,
    struct gdbwire_mi_command **out)
{
    enum gdbwire_result result = GDBWIRE_OK;
    struct gdbwire_mi_result *mi_result;
    struct gdbwire_mi_source_file *files = 0, *cur_node, *new_node;

    GDBWIRE_ASSERT(result_record->result_class == GDBWIRE_MI_DONE);
    GDBWIRE_ASSERT(result_record->result);

    mi_result = result_record->result;

    GDBWIRE_ASSERT(mi_result->kind == GDBWIRE_MI_LIST);
    GDBWIRE_ASSERT(strcmp(mi_result->variable, "files") == 0);
    GDBWIRE_ASSERT(!mi_result->next);

    mi_result = mi_result->variant.result;

    while (mi_result) {
        struct gdbwire_mi_result *tuple;
        char *file = 0, *fullname = 0;
        GDBWIRE_ASSERT_GOTO(mi_result->kind == GDBWIRE_MI_TUPLE, result, err);
        tuple = mi_result->variant.result;

        /* file field */
        GDBWIRE_ASSERT_GOTO(tuple->kind == GDBWIRE_MI_CSTRING, result, err);
        GDBWIRE_ASSERT_GOTO(strcmp(tuple->variable, "file") == 0, result, err);
        file = tuple->variant.cstring;

        if (tuple->next) {
            tuple = tuple->next;

            /* fullname field */
            GDBWIRE_ASSERT_GOTO(tuple->kind == GDBWIRE_MI_CSTRING, result, err);
            GDBWIRE_ASSERT_GOTO(strcmp(tuple->variable, "fullname") == 0,
                result, err);
            fullname = tuple->variant.cstring;
        }

        GDBWIRE_ASSERT(!tuple->next);

        /* Create the new */
        new_node = calloc(1, sizeof(struct gdbwire_mi_source_file));
        GDBWIRE_ASSERT_GOTO(new_node, result, err);

        new_node->file = gdbwire_strdup(file);
        new_node->fullname = (fullname)?gdbwire_strdup(fullname):0;
        new_node->next = 0;

        /* Append the node to the list */
        if (files) {
            cur_node->next = new_node;
            cur_node = cur_node->next;
        } else {
            files = cur_node = new_node;
        }

        GDBWIRE_ASSERT_GOTO(new_node->file && (new_node->fullname || !fullname),
            result, err);

        mi_result = mi_result->next;
    }

    *out = calloc(1, sizeof(struct gdbwire_mi_command));
    GDBWIRE_ASSERT_GOTO(*out, result, err);
    (*out)->kind = GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILES;
    (*out)->variant.file_list_exec_source_files.files = files;

    return result;

err:
    gdbwire_mi_source_files_free(files); 

    return result;
}

enum gdbwire_result
gdbwire_get_mi_command(enum gdbwire_mi_command_kind kind,
        struct gdbwire_mi_result_record *result_record,
        struct gdbwire_mi_command **out)
{
    enum gdbwire_result result = GDBWIRE_OK;

    GDBWIRE_ASSERT(result_record);
    GDBWIRE_ASSERT(out);

    *out = 0;

    switch (kind) {
        case GDBWIRE_MI_BREAK_INFO:
            result = break_info(result_record, out);
            break;
        case GDBWIRE_MI_STACK_INFO_FRAME:
            result = stack_info_frame(result_record, out);
            break;
        case GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE:
            result = file_list_exec_source_file(result_record, out);
            break;
        case GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILES:
            result = file_list_exec_source_files(result_record, out);
            break;
    }
    
    return result;
}

void gdbwire_mi_command_free(struct gdbwire_mi_command *mi_command)
{
    if (mi_command) {
        switch (mi_command->kind) {
            case GDBWIRE_MI_BREAK_INFO:
                gdbwire_mi_breakpoints_free(
                    mi_command->variant.break_info.breakpoints);
                break;
            case GDBWIRE_MI_STACK_INFO_FRAME:
                gdbwire_mi_stack_frame_free(
                    mi_command->variant.stack_info_frame.frame);
                break;
            case GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE:
                free(mi_command->variant.file_list_exec_source_file.file);
                free(mi_command->variant.file_list_exec_source_file.fullname);
                break;
            case GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILES:
                gdbwire_mi_source_files_free(
                    mi_command->variant.file_list_exec_source_files.files);
                break;
        }

        free(mi_command);
    }
}

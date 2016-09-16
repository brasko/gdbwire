#include <stdlib.h>
#include <string.h>

#include "gdbwire_assert.h"
#include "gdbwire_mi_command.h"

enum gdbwire_result
gdbwire_get_mi_command(enum gdbwire_mi_command_kind kind,
        struct gdbwire_mi_result_record *result_record,
        struct gdbwire_mi_command **out)
{
    enum gdbwire_result result = GDBWIRE_OK;
    struct gdbwire_mi_command *mi_command = 0;

    GDBWIRE_ASSERT(result_record);
    GDBWIRE_ASSERT(out);

    *out = 0;

    switch (kind) {
        case GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE: {
            struct gdbwire_mi_result *mi_result;

            char *line, *file, *fullname, *macro_info;
            int macro_info_exists = 0;
            int fullname_exists = 0;

            GDBWIRE_ASSERT(result_record->result_class == GDBWIRE_MI_DONE);
            GDBWIRE_ASSERT(result_record->result);

            mi_result = result_record->result;

            GDBWIRE_ASSERT(mi_result->kind == GDBWIRE_MI_CSTRING);
            GDBWIRE_ASSERT(strcmp(mi_result->variable, "line") == 0);
            line = mi_result->variant.cstring;

            GDBWIRE_ASSERT(mi_result->next);
            mi_result = mi_result->next;

            GDBWIRE_ASSERT(mi_result->kind == GDBWIRE_MI_CSTRING);
            GDBWIRE_ASSERT(strcmp(mi_result->variable, "file") == 0);
            file = mi_result->variant.cstring;

            if (mi_result->next) {
                mi_result = mi_result->next;

                GDBWIRE_ASSERT(mi_result->kind == GDBWIRE_MI_CSTRING);
                GDBWIRE_ASSERT(strcmp(mi_result->variable, "fullname") == 0);
                fullname = mi_result->variant.cstring;
                fullname_exists = 1;
            }

            if (mi_result->next) {
                GDBWIRE_ASSERT(mi_result->next);
                mi_result = mi_result->next;

                GDBWIRE_ASSERT(mi_result->kind == GDBWIRE_MI_CSTRING);
                GDBWIRE_ASSERT(strcmp(mi_result->variable, "macro-info") == 0);
                macro_info = mi_result->variant.cstring;
                GDBWIRE_ASSERT(strlen(macro_info) == 1);
                GDBWIRE_ASSERT(macro_info[0] == '0' || macro_info[0] == '1');
                macro_info_exists = 1;
            }

            GDBWIRE_ASSERT(!mi_result->next);

            mi_command = calloc(1, sizeof(struct gdbwire_mi_command));
            mi_command->kind = kind;
            mi_command->variant.file_list_exec_source_file.line = atoi(line);
            mi_command->variant.file_list_exec_source_file.file = strdup(file);
            mi_command->variant.file_list_exec_source_file.fullname =
                (fullname_exists)?strdup(fullname):0;
            mi_command->variant.file_list_exec_source_file.macro_info_exists =
                macro_info_exists;
            if (macro_info_exists) {
                mi_command->variant.file_list_exec_source_file.macro_info =
                    atoi(macro_info);
            }
            break;
        }
        case GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILES: {
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
                GDBWIRE_ASSERT(mi_result->kind == GDBWIRE_MI_TUPLE);
                tuple = mi_result->variant.result;

                // file field
                GDBWIRE_ASSERT(tuple->kind == GDBWIRE_MI_CSTRING);
                GDBWIRE_ASSERT(strcmp(tuple->variable, "file") == 0);
                file = tuple->variant.cstring;

                if (tuple->next) {
                    tuple = tuple->next;

                    // fullname field
                    GDBWIRE_ASSERT(tuple->kind == GDBWIRE_MI_CSTRING);
                    GDBWIRE_ASSERT(strcmp(tuple->variable, "fullname") == 0);
                    fullname = tuple->variant.cstring;
                }

                GDBWIRE_ASSERT(!tuple->next);

                // Create the new 
                new_node = calloc(1, sizeof(struct gdbwire_mi_source_file));
                new_node->file = strdup(file);
                new_node->fullname = (fullname)?strdup(fullname):0;
                new_node->next = 0;

                // Append the node to the list
                if (files) {
                    cur_node->next = new_node;
                    cur_node = cur_node->next;
                } else {
                    files = cur_node = new_node;
                }

                mi_result = mi_result->next;
            }

            mi_command = calloc(1, sizeof(struct gdbwire_mi_command));
            mi_command->kind = kind;
            mi_command->variant.file_list_exec_source_files.files = files;
            break;
        }
    }
    
    *out = mi_command;

    return result;
}

void gdbwire_mi_command_free(struct gdbwire_mi_command *mi_command)
{
    if (mi_command) {
        switch (mi_command->kind) {
            case GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE:
                free(mi_command->variant.file_list_exec_source_file.file);
                free(mi_command->variant.file_list_exec_source_file.fullname);
                break;
            case GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILES: {
                struct gdbwire_mi_source_file *tmp, *cur = 
                    mi_command->variant.file_list_exec_source_files.files;
                while (cur) {
                    free (cur->file);
                    free (cur->fullname);
                    tmp = cur;
                    cur = cur->next;
                    free(tmp);
                }
                break;
            }
        }

        free(mi_command);
    }
}

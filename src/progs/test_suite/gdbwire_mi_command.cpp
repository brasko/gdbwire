#include <stdio.h>
#include "catch.hpp"
#include "fixture.h"
#include "gdbwire_mi_command.h"
#include "gdbwire_mi_parser.h"

/**
 * The GDB/MI command unit tests.
 *
 * The goal of the following unit tests is to test each kind of mi command,
 * with each variant of output gdb might emit. Including with different
 * versions of gdb.
 */

namespace {
    struct GdbwireMiCommandCallback {
        GdbwireMiCommandCallback() : m_output(0) {
            callbacks.context = (void*)this;
            callbacks.gdbwire_mi_output_callback =
                    GdbwireMiCommandCallback::gdbwire_mi_output_callback;
        }

        ~GdbwireMiCommandCallback() {
            gdbwire_mi_output_free(m_output);
        }

        static void gdbwire_mi_output_callback(void *context,
            gdbwire_mi_output *output) {
            GdbwireMiCommandCallback *callback =
                (GdbwireMiCommandCallback *)context;
            callback->gdbwire_mi_output_callback(output);
        }

        void gdbwire_mi_output_callback(gdbwire_mi_output *output) {
            m_output = append_gdbwire_mi_output(m_output, output);
        }

        gdbwire_mi_parser_callbacks callbacks;
        gdbwire_mi_output *m_output;
    };

    struct GdbwireMiCommandTest : public Fixture {
        GdbwireMiCommandTest() {
            parser = gdbwire_mi_parser_create(parserCallback.callbacks);
            REQUIRE(parser);
            output = parse(parser, sourceTestPath());
            REQUIRE(output);
            REQUIRE(output->line);
            result_record = get_mi_result_record(output);
            REQUIRE(result_record);
        }
        
        ~GdbwireMiCommandTest() {
            gdbwire_mi_parser_destroy(parser);
        }

        /**
         * Parse a GDB/MI file and return a gdbwire_mi_output structure.
         *
         * @param parser
         * The gdb mi parser to do the parsing
         *
         * @param input
         * The input file to parse
         *
         * @return
         * A gdbwire_mi_output structure representing the input file.
         * You are responsible for destroying this memory.
         */
        gdbwire_mi_output *parse(gdbwire_mi_parser *parser,
            const std::string &input) {
            FILE *fd;
            int c;

            fd = fopen(input.c_str(), "r");
            REQUIRE(fd);

            while ((c = fgetc(fd)) != EOF) {
                char ch = c;
                REQUIRE(gdbwire_mi_parser_push_data(
                    parser, &ch, 1) == GDBWIRE_OK);
            }
            fclose(fd);

            return parserCallback.m_output;
        }

        gdbwire_mi_result_record *get_mi_result_record(
            gdbwire_mi_output *output) {
            REQUIRE(output);
            REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_RESULT);
            return output->variant.result_record;
        }

        GdbwireMiCommandCallback parserCallback;
        gdbwire_mi_parser *parser;
        gdbwire_mi_output *output;
        gdbwire_mi_result_record *result_record;
    };
}

/**
 * The file list exec source file command.
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, file_list_exec_source_file/basic.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;
    std::string file = "test.cpp", fullname = "/home/foo/test.cpp";

    result = gdbwire_get_mi_command(GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE,
        result_record, &com);
    REQUIRE(result == GDBWIRE_OK);

    REQUIRE(com);
    REQUIRE(com->kind == GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE);
    REQUIRE(com->variant.file_list_exec_source_file.line == 33);
    REQUIRE(com->variant.file_list_exec_source_file.file == file);
    REQUIRE(com->variant.file_list_exec_source_file.fullname == fullname);
    REQUIRE(com->variant.file_list_exec_source_file.macro_info_exists);
    REQUIRE(!com->variant.file_list_exec_source_file.macro_info);

    gdbwire_mi_command_free(com);
}

/**
 * The file list exec source file command.
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, file_list_exec_source_file/no_fullname.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;
    std::string file = "test.cpp";

    result = gdbwire_get_mi_command(GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE,
        result_record, &com);
    REQUIRE(result == GDBWIRE_OK);

    REQUIRE(com);
    REQUIRE(com->kind == GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE);
    REQUIRE(com->variant.file_list_exec_source_file.line == 33);
    REQUIRE(com->variant.file_list_exec_source_file.file == file);
    REQUIRE(!com->variant.file_list_exec_source_file.fullname);
    REQUIRE(!com->variant.file_list_exec_source_file.macro_info_exists);
    REQUIRE(!com->variant.file_list_exec_source_file.macro_info);

    gdbwire_mi_command_free(com);
}

/**
 * The file list exec source file command with no macro info in it.
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, file_list_exec_source_file/no_macro_info.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;
    std::string file = "test.cpp", fullname = "/home/foo/test.cpp";

    result = gdbwire_get_mi_command(GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE,
        result_record, &com);
    REQUIRE(result == GDBWIRE_OK);

    REQUIRE(com);
    REQUIRE(com->kind == GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE);
    REQUIRE(com->variant.file_list_exec_source_file.line == 33);
    REQUIRE(com->variant.file_list_exec_source_file.file == file);
    REQUIRE(com->variant.file_list_exec_source_file.fullname == fullname);
    REQUIRE(!com->variant.file_list_exec_source_file.macro_info_exists);

    gdbwire_mi_command_free(com);
}

/**
 * The file list exec source file command.
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, file_list_exec_source_file/fail_line.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;

    result = gdbwire_get_mi_command(GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE,
        result_record, &com);
    REQUIRE(result == GDBWIRE_ASSERT);
    REQUIRE(!com);
}

/**
 * The file list exec source file command.
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, file_list_exec_source_file/fail_file.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;

    result = gdbwire_get_mi_command(GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE,
        result_record, &com);
    REQUIRE(result == GDBWIRE_ASSERT);
    REQUIRE(!com);
}

/**
 * The file list exec source file command.
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, file_list_exec_source_file/fail_fullname.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;

    result = gdbwire_get_mi_command(GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE,
        result_record, &com);
    REQUIRE(result == GDBWIRE_ASSERT);
    REQUIRE(!com);

    gdbwire_mi_command_free(com);
}

/**
 * The file list exec source file command.
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, file_list_exec_source_file/fail_macro_info.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;

    result = gdbwire_get_mi_command(GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE,
        result_record, &com);
    REQUIRE(result == GDBWIRE_ASSERT);
    REQUIRE(!com);

    gdbwire_mi_command_free(com);
}

/**
 * The file list exec source files command. Check the case with no files.
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, file_list_exec_source_files/empty.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;

    result = gdbwire_get_mi_command(GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILES,
        result_record, &com);
    REQUIRE(result == GDBWIRE_OK);

    REQUIRE(com);
    REQUIRE(com->kind == GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILES);
    REQUIRE(!com->variant.file_list_exec_source_files.files);

    gdbwire_mi_command_free(com);
}

/**
 * The file list exec source files command.
 * Check the case with 1 pair of files.
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, file_list_exec_source_files/1_pair.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;
    gdbwire_mi_source_file *files;
    std::string file = "test.cpp", fullname = "/home/foo/test.cpp";

    result = gdbwire_get_mi_command(GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILES,
        result_record, &com);
    REQUIRE(result == GDBWIRE_OK);

    REQUIRE(com);
    REQUIRE(com->kind == GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILES);
    REQUIRE(com->variant.file_list_exec_source_files.files);

    files = com->variant.file_list_exec_source_files.files;

    REQUIRE(files->file == file);
    REQUIRE(files->fullname == fullname);
    REQUIRE(!files->next);

    gdbwire_mi_command_free(com);
}

/**
 * The file list exec source files command.
 * Check the case with 2 pair of files.
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, file_list_exec_source_files/2_pair.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;
    gdbwire_mi_source_file *files;
    std::string file1= "a.cpp", fullname1= "/tmp/a.cpp";
    std::string file2= "b.cpp", fullname2= "/tmp/b.cpp";

    result = gdbwire_get_mi_command(GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILES,
        result_record, &com);
    REQUIRE(result == GDBWIRE_OK);

    REQUIRE(com);
    REQUIRE(com->kind == GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILES);
    REQUIRE(com->variant.file_list_exec_source_files.files);

    files = com->variant.file_list_exec_source_files.files;

    REQUIRE(files->file == file1);
    REQUIRE(files->fullname == fullname1);
    REQUIRE(files->next);
    files = files->next;

    REQUIRE(files->file == file2);
    REQUIRE(files->fullname == fullname2);
    REQUIRE(!files->next);

    gdbwire_mi_command_free(com);
}

/**
 * The file list exec source files command.
 * Ensure that fullname is not required. (for legacy versions of gdb)
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, file_list_exec_source_files/no_full.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;
    gdbwire_mi_source_file *files;
    std::string file = "test.cpp";

    result = gdbwire_get_mi_command(GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILES,
        result_record, &com);
    REQUIRE(result == GDBWIRE_OK);

    REQUIRE(com);
    REQUIRE(com->kind == GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILES);
    REQUIRE(com->variant.file_list_exec_source_files.files);

    files = com->variant.file_list_exec_source_files.files;

    REQUIRE(files->file == file);
    REQUIRE(!files->fullname);
    REQUIRE(!files->next);

    gdbwire_mi_command_free(com);
}

/**
 * The file list exec source files command.
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, file_list_exec_source_files/1_pair_fail_file.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;
    gdbwire_mi_source_file *files;

    result = gdbwire_get_mi_command(GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILES,
        result_record, &com);
    REQUIRE(result == GDBWIRE_ASSERT);
    REQUIRE(!com);
}

/**
 * The file list exec source files command.
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, file_list_exec_source_files/1_pair_fail_fullname.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;
    gdbwire_mi_source_file *files;

    result = gdbwire_get_mi_command(GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILES,
        result_record, &com);
    REQUIRE(result == GDBWIRE_ASSERT);
    REQUIRE(!com);
}

/**
 * The file list exec source files command.
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, file_list_exec_source_files/2_pair_fail_fullname.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;
    gdbwire_mi_source_file *files;

    result = gdbwire_get_mi_command(GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILES,
        result_record, &com);
    REQUIRE(result == GDBWIRE_ASSERT);
    REQUIRE(!com);
}

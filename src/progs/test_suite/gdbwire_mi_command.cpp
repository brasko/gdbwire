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
    std::string typeBreakpoint = "breakpoint";

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
 * The -break-info command. No breakpoints.
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, break_info/no_bkpt.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;

    result = gdbwire_get_mi_command(GDBWIRE_MI_BREAK_INFO, result_record, &com);
    REQUIRE(result == GDBWIRE_OK);

    REQUIRE(com);
    REQUIRE(com->kind == GDBWIRE_MI_BREAK_INFO);
    REQUIRE(!com->variant.break_info.breakpoints);

    gdbwire_mi_command_free(com);
}

/**
 * The -break-info command.
 *
 * A normal breakpoint at main.
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, break_info/one_bkpt.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;
    gdbwire_mi_breakpoint *breakpoint;

    result = gdbwire_get_mi_command(GDBWIRE_MI_BREAK_INFO, result_record, &com);
    REQUIRE(result == GDBWIRE_OK);

    REQUIRE(com);
    REQUIRE(com->kind == GDBWIRE_MI_BREAK_INFO);
    REQUIRE(com->variant.break_info.breakpoints);

    breakpoint = com->variant.break_info.breakpoints;
    REQUIRE(breakpoint->number);
    REQUIRE(breakpoint->number == std::string("1"));
    REQUIRE(!breakpoint->multi);
    REQUIRE(!breakpoint->from_multi);
    REQUIRE(breakpoint->type);
    REQUIRE(breakpoint->type == typeBreakpoint);
    REQUIRE(!breakpoint->catch_type);
    REQUIRE(breakpoint->disposition == GDBWIRE_MI_BP_DISP_KEEP);
    REQUIRE(breakpoint->enabled);
    REQUIRE(breakpoint->address);
    REQUIRE(breakpoint->address == std::string("0x0000000000400501"));
    REQUIRE(breakpoint->func_name);
    REQUIRE(breakpoint->func_name == std::string("main(int, char**)"));
    REQUIRE(breakpoint->file);
    REQUIRE(breakpoint->file == std::string("main.cpp"));
    REQUIRE(breakpoint->fullname);
    REQUIRE(breakpoint->fullname == std::string("/home/foo/main.cpp"));
    REQUIRE(breakpoint->line == 10);
    REQUIRE(breakpoint->times == 0);
    REQUIRE(breakpoint->original_location == std::string("main"));
    REQUIRE(!breakpoint->pending);
    REQUIRE(!breakpoint->multi_breakpoints);
    REQUIRE(!breakpoint->multi_breakpoint);
    REQUIRE(!breakpoint->next);
    
    gdbwire_mi_command_free(com);
}

/**
 * The -break-info command.
 *
 * A multiple location breakpoint.
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, break_info/multi_bkpt.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;
    gdbwire_mi_breakpoint *breakpoint;

    result = gdbwire_get_mi_command(GDBWIRE_MI_BREAK_INFO, result_record, &com);
    REQUIRE(result == GDBWIRE_OK);

    REQUIRE(com);
    REQUIRE(com->kind == GDBWIRE_MI_BREAK_INFO);
    REQUIRE(com->variant.break_info.breakpoints);

    breakpoint = com->variant.break_info.breakpoints;
    REQUIRE(breakpoint->number);
    REQUIRE(breakpoint->number == std::string("1"));
    REQUIRE(breakpoint->multi);
    REQUIRE(!breakpoint->from_multi);
    REQUIRE(breakpoint->type);
    REQUIRE(breakpoint->type == typeBreakpoint);
    REQUIRE(!breakpoint->catch_type);
    REQUIRE(breakpoint->disposition == GDBWIRE_MI_BP_DISP_KEEP);
    REQUIRE(breakpoint->enabled);
    REQUIRE(breakpoint->address);
    REQUIRE(breakpoint->address == std::string("<MULTIPLE>"));
    REQUIRE(!breakpoint->func_name);
    REQUIRE(!breakpoint->file);
    REQUIRE(!breakpoint->fullname);
    REQUIRE(breakpoint->line == 0);
    REQUIRE(breakpoint->times == 3);
    REQUIRE(breakpoint->original_location);
    REQUIRE(breakpoint->original_location == std::string("foo"));
    REQUIRE(!breakpoint->pending);
    REQUIRE(breakpoint->multi_breakpoints);
    REQUIRE(!breakpoint->multi_breakpoint);
    REQUIRE(!breakpoint->next);

    /* Multi breakpoint first child */
    breakpoint = breakpoint->multi_breakpoints;
    REQUIRE(breakpoint->number);
    REQUIRE(breakpoint->number == std::string("1.1"));
    REQUIRE(!breakpoint->multi);
    REQUIRE(breakpoint->from_multi);
    REQUIRE(!breakpoint->type);
    REQUIRE(!breakpoint->catch_type);
    REQUIRE(breakpoint->disposition == GDBWIRE_MI_BP_DISP_UNKNOWN);
    REQUIRE(breakpoint->enabled);
    REQUIRE(breakpoint->address);
    REQUIRE(breakpoint->address == std::string("0x00000000004004dd"));
    REQUIRE(breakpoint->func_name);
    REQUIRE(breakpoint->func_name == std::string("foo(int)"));
    REQUIRE(breakpoint->file);
    REQUIRE(breakpoint->file == std::string("main.cpp"));
    REQUIRE(breakpoint->fullname);
    REQUIRE(breakpoint->fullname == std::string("/home/foo/main.cpp"));
    REQUIRE(breakpoint->line == 2);
    REQUIRE(breakpoint->times == 0);
    REQUIRE(!breakpoint->original_location);
    REQUIRE(!breakpoint->pending);
    REQUIRE(!breakpoint->multi_breakpoints);
    REQUIRE(breakpoint->multi_breakpoint);
    REQUIRE(breakpoint->multi_breakpoint->number);
    REQUIRE(breakpoint->multi_breakpoint->number == std::string("1"));
    REQUIRE(breakpoint->next);

    /* Multi breakpoint second child */
    breakpoint = breakpoint->next;
    REQUIRE(breakpoint->number);
    REQUIRE(breakpoint->number == std::string("1.2"));
    REQUIRE(!breakpoint->multi);
    REQUIRE(breakpoint->from_multi);
    REQUIRE(!breakpoint->type);
    REQUIRE(!breakpoint->catch_type);
    REQUIRE(breakpoint->disposition == GDBWIRE_MI_BP_DISP_UNKNOWN);
    REQUIRE(breakpoint->enabled);
    REQUIRE(breakpoint->address);
    REQUIRE(breakpoint->address == std::string("0x00000000004004eb"));
    REQUIRE(breakpoint->func_name);
    REQUIRE(breakpoint->func_name == std::string("foo(double)"));
    REQUIRE(breakpoint->file);
    REQUIRE(breakpoint->file == std::string("main.cpp"));
    REQUIRE(breakpoint->fullname);
    REQUIRE(breakpoint->fullname == std::string("/home/foo/main.cpp"));
    REQUIRE(breakpoint->line == 6);
    REQUIRE(breakpoint->times == 0);
    REQUIRE(!breakpoint->original_location);
    REQUIRE(!breakpoint->pending);
    REQUIRE(!breakpoint->multi_breakpoints);
    REQUIRE(breakpoint->multi_breakpoint);
    REQUIRE(breakpoint->multi_breakpoint->number);
    REQUIRE(breakpoint->multi_breakpoint->number == std::string("1"));
    REQUIRE(!breakpoint->next);
    
    gdbwire_mi_command_free(com);
}

/**
 * The -break-info command.
 *
 * Two normal breakpoints.
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, break_info/two_bkpts.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;
    gdbwire_mi_breakpoint *breakpoint;

    result = gdbwire_get_mi_command(GDBWIRE_MI_BREAK_INFO, result_record, &com);
    REQUIRE(result == GDBWIRE_OK);

    REQUIRE(com);
    REQUIRE(com->kind == GDBWIRE_MI_BREAK_INFO);
    REQUIRE(com->variant.break_info.breakpoints);

    breakpoint = com->variant.break_info.breakpoints;
    REQUIRE(breakpoint->number);
    REQUIRE(breakpoint->number == std::string("1"));
    REQUIRE(!breakpoint->multi);
    REQUIRE(!breakpoint->from_multi);
    REQUIRE(breakpoint->type);
    REQUIRE(breakpoint->type == typeBreakpoint);
    REQUIRE(breakpoint->disposition == GDBWIRE_MI_BP_DISP_KEEP);
    REQUIRE(!breakpoint->catch_type);
    REQUIRE(breakpoint->enabled);
    REQUIRE(breakpoint->address);
    REQUIRE(breakpoint->address == std::string("0x0000000000400501"));
    REQUIRE(breakpoint->func_name);
    REQUIRE(breakpoint->func_name == std::string("main(int, char**)"));
    REQUIRE(breakpoint->file);
    REQUIRE(breakpoint->file == std::string("main.cpp"));
    REQUIRE(breakpoint->fullname);
    REQUIRE(breakpoint->fullname == std::string("/home/foo/main.cpp"));
    REQUIRE(breakpoint->line == 10);
    REQUIRE(breakpoint->times == 0);
    REQUIRE(breakpoint->original_location);
    REQUIRE(breakpoint->original_location == std::string("main"));
    REQUIRE(!breakpoint->pending);
    REQUIRE(!breakpoint->multi_breakpoints);
    REQUIRE(!breakpoint->multi_breakpoint);
    REQUIRE(breakpoint->next);

    breakpoint = breakpoint->next;
    REQUIRE(breakpoint->number);
    REQUIRE(breakpoint->number == std::string("2"));
    REQUIRE(!breakpoint->multi);
    REQUIRE(!breakpoint->from_multi);
    REQUIRE(breakpoint->type);
    REQUIRE(breakpoint->type == typeBreakpoint);
    REQUIRE(!breakpoint->catch_type);
    REQUIRE(breakpoint->disposition == GDBWIRE_MI_BP_DISP_KEEP);
    REQUIRE(breakpoint->enabled);
    REQUIRE(breakpoint->address);
    REQUIRE(breakpoint->address == std::string("0x00000000004004eb"));
    REQUIRE(breakpoint->func_name);
    REQUIRE(breakpoint->func_name == std::string("foo(double)"));
    REQUIRE(breakpoint->file);
    REQUIRE(breakpoint->file == std::string("main.cpp"));
    REQUIRE(breakpoint->fullname);
    REQUIRE(breakpoint->fullname == std::string("/home/foo/main.cpp"));
    REQUIRE(breakpoint->line == 6);
    REQUIRE(breakpoint->times == 0);
    REQUIRE(breakpoint->original_location);
    REQUIRE(breakpoint->original_location == std::string("main.cpp:6"));
    REQUIRE(!breakpoint->pending);
    REQUIRE(!breakpoint->multi_breakpoints);
    REQUIRE(!breakpoint->multi_breakpoint);
    REQUIRE(!breakpoint->next);
    
    gdbwire_mi_command_free(com);
}

/**
 * The -break-info command.
 *
 * The enable field. Show a breakpoint enabled and disabled.
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, break_info/enable.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;
    gdbwire_mi_breakpoint *breakpoint;

    result = gdbwire_get_mi_command(GDBWIRE_MI_BREAK_INFO, result_record, &com);
    REQUIRE(result == GDBWIRE_OK);

    REQUIRE(com);
    REQUIRE(com->kind == GDBWIRE_MI_BREAK_INFO);
    REQUIRE(com->variant.break_info.breakpoints);

    breakpoint = com->variant.break_info.breakpoints;
    REQUIRE(breakpoint->number);
    REQUIRE(breakpoint->number == std::string("1"));
    REQUIRE(breakpoint->enabled);
    REQUIRE(breakpoint->next);

    breakpoint = breakpoint->next;
    REQUIRE(breakpoint->number);
    REQUIRE(breakpoint->number == std::string("2"));
    REQUIRE(!breakpoint->enabled);
    REQUIRE(!breakpoint->next);
    
    gdbwire_mi_command_free(com);
}

/**
 * The -break-info command.
 *
 * The enable field for multiple location breakpoints.
 *
 * Show that the multiple location breakpoint can be enabled but a
 * breakpoint created from the multiple location breakpoint can be disabled.
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, break_info/enable_multi_loc.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;
    gdbwire_mi_breakpoint *breakpoint;

    result = gdbwire_get_mi_command(GDBWIRE_MI_BREAK_INFO, result_record, &com);
    REQUIRE(result == GDBWIRE_OK);

    REQUIRE(com);
    REQUIRE(com->kind == GDBWIRE_MI_BREAK_INFO);
    REQUIRE(com->variant.break_info.breakpoints);

    /* The first multi-location breakpoint is enabled
     * - the first child is enabled
     * - the second child is disabled
     */
    breakpoint = com->variant.break_info.breakpoints;
    REQUIRE(breakpoint->number);
    REQUIRE(breakpoint->number == std::string("1"));
    REQUIRE(breakpoint->multi);
    REQUIRE(breakpoint->enabled);

    REQUIRE(breakpoint->multi_breakpoints);
    REQUIRE(breakpoint->multi_breakpoints->enabled);

    REQUIRE(breakpoint->multi_breakpoints->next);
    REQUIRE(!breakpoint->multi_breakpoints->next->enabled);

    REQUIRE(breakpoint->next);

    
    /* The first multi-location breakpoint is disabled
     * - the first child is disabled
     * - the second child is enabled
     */
    breakpoint = breakpoint->next;
    REQUIRE(breakpoint->number);
    REQUIRE(breakpoint->number == std::string("2"));
    REQUIRE(!breakpoint->enabled);

    REQUIRE(breakpoint->multi_breakpoints);
    REQUIRE(!breakpoint->multi_breakpoints->enabled);

    REQUIRE(breakpoint->multi_breakpoints->next);
    REQUIRE(breakpoint->multi_breakpoints->next->enabled);

    REQUIRE(!breakpoint->next);
    
    gdbwire_mi_command_free(com);
}

/**
 * The -break-info command.
 *
 * The address field. Show the cases that we know about,
 * - a hexidecimal address
 * - <MULTIPLE>
 * - <PENDING>
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, break_info/address.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;
    gdbwire_mi_breakpoint *breakpoint;

    result = gdbwire_get_mi_command(GDBWIRE_MI_BREAK_INFO, result_record, &com);
    REQUIRE(result == GDBWIRE_OK);

    REQUIRE(com);
    REQUIRE(com->kind == GDBWIRE_MI_BREAK_INFO);
    REQUIRE(com->variant.break_info.breakpoints);

    breakpoint = com->variant.break_info.breakpoints;
    REQUIRE(breakpoint->number);
    REQUIRE(breakpoint->number == std::string("1"));
    REQUIRE(breakpoint->address);
    REQUIRE(breakpoint->address == std::string("0x0000000000400501"));
    REQUIRE(breakpoint->next);

    breakpoint = breakpoint->next;
    REQUIRE(breakpoint->number);
    REQUIRE(breakpoint->number == std::string("2"));
    REQUIRE(breakpoint->address);
    REQUIRE(breakpoint->address == std::string("<MULTIPLE>"));
    REQUIRE(breakpoint->multi);
    REQUIRE(breakpoint->next);

    breakpoint = breakpoint->next;
    REQUIRE(breakpoint->number);
    REQUIRE(breakpoint->number == std::string("3"));
    REQUIRE(breakpoint->address);
    REQUIRE(breakpoint->address == std::string("<PENDING>"));
    REQUIRE(breakpoint->pending);
    REQUIRE(!breakpoint->next);
    
    gdbwire_mi_command_free(com);
}

/**
 * The -break-info command.
 *
 * A normal watchpoint (ie. watch argv[0]).
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, break_info/watchpoint.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;
    gdbwire_mi_breakpoint *breakpoint;

    result = gdbwire_get_mi_command(GDBWIRE_MI_BREAK_INFO, result_record, &com);
    REQUIRE(result == GDBWIRE_OK);

    REQUIRE(com);
    REQUIRE(com->kind == GDBWIRE_MI_BREAK_INFO);
    REQUIRE(com->variant.break_info.breakpoints);

    breakpoint = com->variant.break_info.breakpoints;
    REQUIRE(breakpoint->number);
    REQUIRE(breakpoint->number == std::string("2"));
    REQUIRE(!breakpoint->multi);
    REQUIRE(!breakpoint->from_multi);
    REQUIRE(breakpoint->type);
    REQUIRE(breakpoint->type == std::string("hw watchpoint"));
    REQUIRE(!breakpoint->catch_type);
    REQUIRE(breakpoint->disposition == GDBWIRE_MI_BP_DISP_KEEP);
    REQUIRE(breakpoint->enabled);
    REQUIRE(!breakpoint->address);
    REQUIRE(!breakpoint->func_name);
    REQUIRE(!breakpoint->file);
    REQUIRE(!breakpoint->fullname);
    REQUIRE(breakpoint->line == 0);
    REQUIRE(breakpoint->times == 0);
    REQUIRE(breakpoint->original_location);
    REQUIRE(breakpoint->original_location == std::string("argv[0]"));
    REQUIRE(!breakpoint->pending);
    REQUIRE(!breakpoint->multi_breakpoints);
    REQUIRE(!breakpoint->multi_breakpoint);
    REQUIRE(!breakpoint->next);
    
    gdbwire_mi_command_free(com);
}

/**
 * The -break-info command.
 *
 * A normal catch point (ie. catch throw).
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, break_info/catchpoint.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;
    gdbwire_mi_breakpoint *breakpoint;

    result = gdbwire_get_mi_command(GDBWIRE_MI_BREAK_INFO, result_record, &com);
    REQUIRE(result == GDBWIRE_OK);

    REQUIRE(com);
    REQUIRE(com->kind == GDBWIRE_MI_BREAK_INFO);
    REQUIRE(com->variant.break_info.breakpoints);

    breakpoint = com->variant.break_info.breakpoints;
    REQUIRE(breakpoint->number);
    REQUIRE(breakpoint->number == std::string("1"));
    REQUIRE(!breakpoint->multi);
    REQUIRE(!breakpoint->from_multi);
    REQUIRE(breakpoint->type);
    REQUIRE(breakpoint->type == typeBreakpoint);
    REQUIRE(breakpoint->catch_type);
    REQUIRE(breakpoint->catch_type == std::string("throw"));
    REQUIRE(breakpoint->disposition == GDBWIRE_MI_BP_DISP_KEEP);
    REQUIRE(breakpoint->enabled);
    REQUIRE(breakpoint->address);
    REQUIRE(breakpoint->address == std::string("<PENDING>"));
    REQUIRE(!breakpoint->func_name);
    REQUIRE(!breakpoint->file);
    REQUIRE(!breakpoint->fullname);
    REQUIRE(breakpoint->line == 0);
    REQUIRE(breakpoint->times == 0);
    REQUIRE(!breakpoint->original_location);
    REQUIRE(breakpoint->pending);
    REQUIRE(!breakpoint->multi_breakpoints);
    REQUIRE(!breakpoint->multi_breakpoint);
    REQUIRE(!breakpoint->next);
    
    gdbwire_mi_command_free(com);
}

/**
 * The -break-info command.
 *
 * Fail, no number field.
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, break_info/no_number.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;

    result = gdbwire_get_mi_command(GDBWIRE_MI_BREAK_INFO, result_record, &com);
    REQUIRE(result == GDBWIRE_ASSERT);

    REQUIRE(!com);
}

/**
 * The -stack-info-frame command.
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, stack_info_frame/basic.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;
    gdbwire_mi_stack_frame *frame;

    result = gdbwire_get_mi_command(GDBWIRE_MI_STACK_INFO_FRAME,
        result_record, &com);
    REQUIRE(result == GDBWIRE_OK);

    REQUIRE(com);
    REQUIRE(com->kind == GDBWIRE_MI_STACK_INFO_FRAME);
    REQUIRE(com->variant.stack_info_frame.frame);
    frame = com->variant.stack_info_frame.frame;

    REQUIRE(frame->level == 0);
    REQUIRE(frame->address);
    REQUIRE(frame->address == std::string("0x0000000000400501"));
    REQUIRE(frame->func);
    REQUIRE(frame->func == std::string("main"));
    REQUIRE(frame->file);
    REQUIRE(frame->file == std::string("main.cpp"));
    REQUIRE(frame->fullname);
    REQUIRE(frame->fullname == std::string("/home/foo/main.cpp"));
    REQUIRE(frame->line == 10);
    REQUIRE(!frame->from);

    gdbwire_mi_command_free(com);
}

/**
 * The -stack-info-frame command.
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, stack_info_frame/extra_field.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;
    gdbwire_mi_stack_frame *frame;

    result = gdbwire_get_mi_command(GDBWIRE_MI_STACK_INFO_FRAME,
        result_record, &com);
    REQUIRE(result == GDBWIRE_OK);

    REQUIRE(com);
    REQUIRE(com->kind == GDBWIRE_MI_STACK_INFO_FRAME);
    REQUIRE(com->variant.stack_info_frame.frame);
    frame = com->variant.stack_info_frame.frame;

    REQUIRE(frame->level == 0);
    REQUIRE(frame->address);
    REQUIRE(frame->address == std::string("0x0000000000400501"));
    REQUIRE(frame->func);
    REQUIRE(frame->func == std::string("main"));
    REQUIRE(frame->file);
    REQUIRE(frame->file == std::string("main.cpp"));
    REQUIRE(frame->fullname);
    REQUIRE(frame->fullname == std::string("/home/foo/main.cpp"));
    REQUIRE(frame->line == 10);
    REQUIRE(!frame->from);

    gdbwire_mi_command_free(com);
}

/**
 * The -stack-info-frame command.
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, stack_info_frame/minimal.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;
    gdbwire_mi_stack_frame *frame;

    result = gdbwire_get_mi_command(GDBWIRE_MI_STACK_INFO_FRAME,
        result_record, &com);
    REQUIRE(result == GDBWIRE_OK);

    REQUIRE(com);
    REQUIRE(com->kind == GDBWIRE_MI_STACK_INFO_FRAME);
    REQUIRE(com->variant.stack_info_frame.frame);
    frame = com->variant.stack_info_frame.frame;

    REQUIRE(frame->level == 2);
    REQUIRE(frame->address);
    REQUIRE(frame->address == std::string("0x0000000000400501"));
    REQUIRE(!frame->func);
    REQUIRE(!frame->file);
    REQUIRE(!frame->fullname);
    REQUIRE(frame->line == 0);
    REQUIRE(!frame->from);

    gdbwire_mi_command_free(com);
}

/**
 * The -stack-info-frame command.
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, stack_info_frame/from_field.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;
    gdbwire_mi_stack_frame *frame;

    result = gdbwire_get_mi_command(GDBWIRE_MI_STACK_INFO_FRAME,
        result_record, &com);
    REQUIRE(result == GDBWIRE_OK);

    REQUIRE(com);
    REQUIRE(com->kind == GDBWIRE_MI_STACK_INFO_FRAME);
    REQUIRE(com->variant.stack_info_frame.frame);
    frame = com->variant.stack_info_frame.frame;

    REQUIRE(frame->level == 2);
    REQUIRE(frame->address);
    REQUIRE(frame->address == std::string("0x0000000000400501"));
    REQUIRE(!frame->func);
    REQUIRE(!frame->file);
    REQUIRE(!frame->fullname);
    REQUIRE(frame->line == 0);
    REQUIRE(frame->from);
    REQUIRE(frame->from == std::string("From!"));

    gdbwire_mi_command_free(com);
}

/**
 * The -stack-info-frame command.
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, stack_info_frame/no_level.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;
    gdbwire_mi_stack_frame *frame;

    result = gdbwire_get_mi_command(GDBWIRE_MI_STACK_INFO_FRAME,
        result_record, &com);
    REQUIRE(result == GDBWIRE_ASSERT);
    REQUIRE(!com);
}

/**
 * The -stack-info-frame command.
 */
TEST_CASE_METHOD_N(GdbwireMiCommandTest, stack_info_frame/no_address.mi)
{
    gdbwire_result result;
    gdbwire_mi_command *com = 0;
    gdbwire_mi_stack_frame *frame;

    result = gdbwire_get_mi_command(GDBWIRE_MI_STACK_INFO_FRAME,
        result_record, &com);
    REQUIRE(result == GDBWIRE_ASSERT);
    REQUIRE(!com);
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
TEST_CASE_METHOD_N(GdbwireMiCommandTest, file_list_exec_source_file/extra_field.mi)
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

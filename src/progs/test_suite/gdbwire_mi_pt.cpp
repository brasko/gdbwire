#include <stdio.h>
#include "catch.hpp"
#include "fixture.h"
#include "gdbwire_mi_pt.h"
#include "gdbwire_mi_parser.h"

/**
 * The GDB/MI parse tree unit tests.
 *
 * Unit testing a parse tree is a non trivial task. A parse tree can have
 * many variations and it's often difficult to test them all in isolation.
 *
 * The goal of the following unit tests is to isolate testing as many
 * combinations of GDB/MI parse trees as possible. Hoping to achieve 100%
 * coverage on the gdbwire_mi_grammar.y file.
 *
 * These unit tests will not be concerned with the semantics of the parse
 * tree, but simply validating that all combinations of GDB/MI output
 * commands can be parsed and turned into an appropriate parse tree.
 */

namespace {
    struct GdbwireMiParserCallback {
        GdbwireMiParserCallback() : m_output(0) {
            callbacks.context = (void*)this;
            callbacks.gdbwire_mi_output_callback =
                    GdbwireMiParserCallback::gdbwire_mi_output_callback;
        }

        ~GdbwireMiParserCallback() {
            gdbwire_mi_output_free(m_output);
        }

        static void gdbwire_mi_output_callback(void *context,
            gdbwire_mi_output *output) {
            GdbwireMiParserCallback *callback =
                (GdbwireMiParserCallback *)context;
            callback->gdbwire_mi_output_callback(output);
        }

        void gdbwire_mi_output_callback(gdbwire_mi_output *output) {
            m_output = append_gdbwire_mi_output(m_output, output);
        }

        gdbwire_mi_parser_callbacks callbacks;
        gdbwire_mi_output *m_output;
    };

    struct GdbwireMiPtTest : public Fixture {
        GdbwireMiPtTest() {
            parser = gdbwire_mi_parser_create(parserCallback.callbacks);
            REQUIRE(parser);
            output = parse(parser, sourceTestPath());
            REQUIRE(output);
            REQUIRE(output->line);
        }
        
        ~GdbwireMiPtTest() {
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

        /**
         * Checks a result record in an output command.
         *
         * @param output
         * The output command to check the result record in.
         *
         * @param result_class
         * The result class to compare in the result record.
         *
         * @param token
         * The token to compare in the result record.
         *
         * @return
         * The gdbwire_mi result or NULL if none in the result record.
         */
        gdbwire_mi_result *CHECK_OUTPUT_RESULT_RECORD(gdbwire_mi_output *output,
            gdbwire_mi_result_class result_class,
            const std::string &token = "") {
            REQUIRE(output);
            REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_RESULT);
            gdbwire_mi_result_record *result_record =
                output->variant.result_record;
            REQUIRE(result_record);
            if (token.empty()) {
                REQUIRE(!result_record->token);
            } else {
                REQUIRE(result_record->token);
                REQUIRE(token == result_record->token);
            }
            REQUIRE(result_record->result_class == result_class);
            return result_record->result;
        }

        gdbwire_mi_oob_record *CHECK_OUTPUT_OOB_RECORD(
            gdbwire_mi_output *output) {
            REQUIRE(output);
            REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_OOB);
            REQUIRE(output->variant.oob_record);
            return output->variant.oob_record;
        }

        gdbwire_mi_output *CHECK_OUTPUT_PROMPT(gdbwire_mi_output *output) {
            REQUIRE(output);
            REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_PROMPT);
            return output->next;
        }

        void CHECK_OUTPUT_AT_FINAL_PROMPT(gdbwire_mi_output *output) {
            REQUIRE(output);
            output = CHECK_OUTPUT_PROMPT(output);
            REQUIRE(!output);
        }

        /**
         * Checks an out-of-band record to ensure it's an stream record.
         *
         * @param oob
         * The out of band record to check.
         *
         * @return
         * The stream record found inside the async out-of-band record.
         */
        gdbwire_mi_stream_record *CHECK_OOB_RECORD_STREAM(
            gdbwire_mi_oob_record *oob) {
            REQUIRE(oob);
            REQUIRE(oob->kind == GDBWIRE_MI_STREAM);
            REQUIRE(oob->variant.stream_record);
            return oob->variant.stream_record;
        }

        /**
         * Checks an out-of-band record to ensure it's an async record.
         *
         * @param oob
         * The out of band record to check.
         *
         * @return
         * The async record found inside the async out-of-band record.
         */
        gdbwire_mi_async_record *CHECK_OOB_RECORD_ASYNC(
            gdbwire_mi_oob_record *oob) {
            REQUIRE(oob);
            REQUIRE(oob->kind == GDBWIRE_MI_ASYNC);
            REQUIRE(oob->variant.async_record);
            return oob->variant.async_record;
        }

        /**
         * A utility function for checking the values in a
         * gdbwire_mi_stream_record.
         *
         * If the values do not match the record, an assertion failure is made.
         *
         * @param record
         * The gdbwire_mi stream record to check the values of.
         *
         * @param kind
         * The kind of record to check for.
         *
         * @param expected
         * The expected cstring value to check for.
         */
        void CHECK_STREAM_RECORD(gdbwire_mi_stream_record *record,
            gdbwire_mi_stream_record_kind kind, const std::string &expected) {
            REQUIRE(record);
            REQUIRE(record->kind == kind);
            REQUIRE(expected == record->cstring);
        }

        /**
         * Check the async record matches the corresponding parameters.
         *
         * @param async_record
         * The async record to check.
         *
         * @param kind
         * The expected async record kind.
         *
         * @param async_class
         * The expected async class kind.
         *
         * @param token
         * The token to compare in the result record.
         *
         * @return
         * The gdbwire_mi result of the async record (may be NULL);
         */
        gdbwire_mi_result *CHECK_ASYNC_RECORD(
            gdbwire_mi_async_record *async_record,
            gdbwire_mi_async_record_kind kind,
            gdbwire_mi_async_class async_class,
            const std::string &token = "") {
            REQUIRE(async_record);

            if (token.empty()) {
                REQUIRE(!async_record->token);
            } else {
                REQUIRE(async_record->token);
                REQUIRE(token == async_record->token);
            }

            REQUIRE(async_record->kind == kind);
            REQUIRE(async_record->async_class == async_class);

            return async_record->result;
        }

        /** 
         * Check that the result variable has the expected value.
         *
         * If the values do not match, an assertion failure is made.
         *
         * @param result
         * The gdbwire_mi result to check the variable value of.
         *
         * @param value
         * The expected variable value. If empty, the result's variable
         * should be NULL.
         */
        void CHECK_RESULT_VARIABLE(gdbwire_mi_result *result,
            const std::string &value = "") {
            REQUIRE(result);

            if (value.empty()) {
                REQUIRE(!result->variable);
            } else {
                REQUIRE(result->variable);
                REQUIRE(value == result->variable);
            }
        }

        /**
         * Check that the cstring result matches the corresponding parameters.
         *
         * If the values do not match the result, an assertion failure is made.
         *
         * @param result
         * The gdbwire_mi result to check the values of.
         *
         * @param variable
         * The result variable name or empty string if none.
         *
         * @param expected
         * The expected cstring value.
         *
         * @return
         * Returns the next gdbwire_mi_result pointer.
         */
        gdbwire_mi_result *CHECK_RESULT_CSTRING(gdbwire_mi_result *result,
            const std::string &variable, const std::string &expected) {

            CHECK_RESULT_VARIABLE(result, variable);

            REQUIRE(result->kind == GDBWIRE_MI_CSTRING);
            REQUIRE(expected == result->variant.cstring);

            return result->next;
        }

        /**
         * Check the tuple or list result matches the corresponding parameters.
         *
         * If the values do not match the result, an assertion failure is made.
         *
         * @param result
         * The gdbwire_mi result to check the values of.
         *
         * @param kind
         * A tuple or list is allowed to be checked for.
         *
         * @param variable
         * The result variable name or empty string if none.
         *
         * @return
         * Returns the next gdbwire_mi_result pointer.
         */
        gdbwire_mi_result *CHECK_RESULT_VARIANT(gdbwire_mi_result *result,
            enum gdbwire_mi_result_kind kind,
            const std::string &variable = "") {
            REQUIRE((kind == GDBWIRE_MI_TUPLE || kind == GDBWIRE_MI_LIST));

            CHECK_RESULT_VARIABLE(result, variable);

            REQUIRE(result->kind == kind);
            return result->variant.result;
        }

        /** 
         * A utility function to get a result for the result unit tests.
         *
         * Each result unit test has to get a result from a gdb mi
         * output rule. Each result comes from an output command like this,
         *   *stopped,{}
         *   (gdb)
         * The output command is parsed and the result is retrieved so that
         * it can be properly unit tested. This functionality was placed
         * into a convience function to keep the size of the unit tests down.
         *
         * @param output
         * The output command containing an async output record, containing
         * a result.
         *
         * @return
         * The result associated with the async output record.
         */
        gdbwire_mi_result *GET_RESULT(gdbwire_mi_output *output) {
            gdbwire_mi_oob_record *oob;
            gdbwire_mi_async_record *async;
            gdbwire_mi_result *result;

            oob = CHECK_OUTPUT_OOB_RECORD(output);
            async = CHECK_OOB_RECORD_ASYNC(oob);

            result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_EXEC,
                GDBWIRE_MI_ASYNC_STOPPED);
            REQUIRE(result);
            return result;
        }

        GdbwireMiParserCallback parserCallback;
        gdbwire_mi_parser *parser;
        gdbwire_mi_output *output;
    };
}

/**
 * A simple console output parse tree.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, oob_record/stream/console/basic.mi)
{
    std::string expected = "Hello World console output";
    struct gdbwire_mi_oob_record *oob;
    struct gdbwire_mi_stream_record *stream;

    oob = CHECK_OUTPUT_OOB_RECORD(output);
    stream = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream, GDBWIRE_MI_CONSOLE, expected);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * A single console output with many newlines in it.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, oob_record/stream/console/manylines.mi)
{
    std::string expected =
        "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n"
        "This is free software: you are free to change and redistribute it.\n"
        "There is NO WARRANTY, to the extent permitted by law.  Type \"show copying\"\n"
        "and \"show warranty\" for details.\n";

    struct gdbwire_mi_oob_record *oob;
    struct gdbwire_mi_stream_record *stream;

    oob = CHECK_OUTPUT_OOB_RECORD(output);
    stream = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream, GDBWIRE_MI_CONSOLE, expected);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * All possible characters in the console output stream.
 *
 * The basic idea behind this test is to print a character array with
 * every possible value. For example,
 *   char chars[256];
 *   for (i = 0; i < 256; ++i) {
 *     chars[i] = (char)i;
 *   }
 * The MI output converts the char value 0 to \\000 and 1 to \\001, etc.
 * So it essentially escapes the backslashes. In C, I have to escape them
 * again to compare the values.
 * 
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, oob_record/stream/console/characters.mi)
{
    std::string expected =
        "$1 = "
        "\"\\000\\001\\002\\003\\004\\005\\006\\a"
        "\\b\t\n\\v\\f\r\\016\\017"
        "\\020\\021\\022\\023\\024\\025\\026\\027"
        "\\030\\031\\032\\033\\034\\035\\036\\037"
        " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\\177"
        "\\200\\201\\202\\203\\204\\205\\206\\207"
        "\\210\\211\\212\\213\\214\\215\\216\\217"
        "\\220\\221\\222\\223\\224\\225\\226\\227"
        "\\230\\231\\232\\233\\234\\235\\236\\237"
        "\\240\\241\\242\\243\\244\\245\\246\\247"
        "\\250\\251\\252\\253\\254\\255\\256\\257"
        "\\260\\261\\262\\263\\264\\265\\266\\267"
        "\\270\\271\\272\\273\\274\\275\\276\\277"
        "\\300\\301\\302\\303\\304\\305\\306\\307"
        "\\310\\311\\312\\313\\314\\315\\316\\317"
        "\\320\\321\\322\\323\\324\\325\\326\\327"
        "\\330\\331\\332\\333\\334\\335\\336\\337"
        "\\340\\341\\342\\343\\344\\345\\346\\347"
        "\\350\\351\\352\\353\\354\\355\\356\\357"
        "\\360\\361\\362\\363\\364\\365\\366\\367"
        "\\370\\371\\372\\373\\374\\375\\376\\377\"";
    struct gdbwire_mi_oob_record *oob;
    struct gdbwire_mi_stream_record *stream;

    oob = CHECK_OUTPUT_OOB_RECORD(output);
    stream = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream, GDBWIRE_MI_CONSOLE, expected);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * A simple target output parse tree.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, oob_record/stream/target/basic.mi)
{
    std::string expected = "Hello World target output";
    struct gdbwire_mi_oob_record *oob;
    struct gdbwire_mi_stream_record *stream;

    oob = CHECK_OUTPUT_OOB_RECORD(output);
    stream = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream, GDBWIRE_MI_TARGET, expected);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * A simple log output parse tree.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, oob_record/stream/log/basic.mi)
{
    std::string expected = "Hello World log output";
    struct gdbwire_mi_oob_record *oob;
    struct gdbwire_mi_stream_record *stream;

    oob = CHECK_OUTPUT_OOB_RECORD(output);
    stream = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream, GDBWIRE_MI_LOG, expected);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * A simple out of band record with multiple streams of different kinds.
 *
 * This test is intended to show that multiple different stream records (in
 * any order) can be contained in a single out of band record.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, oob_record/stream/combo/basic.mi)
{
    std::string console1 = "console line 1";
    std::string console2 = "console line 2";
    std::string target1 = "target line 1";
    std::string log1 = "log line 1";
    std::string target2 = "target line 2";
    std::string log2 = "log line 2";
    std::string console3 = "console line 3";

    struct gdbwire_mi_oob_record *oob;
    struct gdbwire_mi_stream_record *stream;

    oob = CHECK_OUTPUT_OOB_RECORD(output);
    stream = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream, GDBWIRE_MI_CONSOLE, console1);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    stream = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream, GDBWIRE_MI_CONSOLE, console2);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    stream = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream, GDBWIRE_MI_TARGET, target1);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    stream = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream, GDBWIRE_MI_LOG, log1);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    stream = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream, GDBWIRE_MI_TARGET, target2);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    stream = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream, GDBWIRE_MI_LOG, log2);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    stream = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream, GDBWIRE_MI_CONSOLE, console3);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test the token field of an async record.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, oob_record/async/token/basic.mi)
{
    gdbwire_mi_oob_record *oob;
    gdbwire_mi_async_record *async;
    gdbwire_mi_result *result;

    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_EXEC,
        GDBWIRE_MI_ASYNC_STOPPED, "111");
    REQUIRE(result);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * A simple status output parse tree.
 *
 * The MI status output was actually hard to make GDB produce.
 * The help I got on the mailing list that worked for me at the time of
 * this writing is,
 *   1. Build hello-world 'main' test program
 *   2. Start gdbserver as: gdbserver :1234 ./main
 *   3. Start gdb as: gdb -i mi ./main
 *   4. Within gdb:
 *   (gdb) -target-select remote :1234
 *   (gdb) -target-download
 *   # Bunch of +download lines
 *   # Single ^done line.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, oob_record/async/status/basic.mi)
{
    gdbwire_mi_oob_record *oob;
    gdbwire_mi_async_record *async;
    gdbwire_mi_result *result;

    oob = CHECK_OUTPUT_OOB_RECORD(output);

    async = CHECK_OOB_RECORD_ASYNC(oob);

    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_STATUS,
        GDBWIRE_MI_ASYNC_DOWNLOAD);
    REQUIRE(result);
    REQUIRE(!result->next);

    result = CHECK_RESULT_VARIANT(result, GDBWIRE_MI_TUPLE);
    REQUIRE(result);
    result = CHECK_RESULT_CSTRING(result, "section", ".interp");
    result = CHECK_RESULT_CSTRING(result, "section-size", "28");
    result = CHECK_RESULT_CSTRING(result, "total-size", "2466");
    REQUIRE(!result);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * All of the supported async class's for the status kind.
 *
 * Currently, +download is the only known async class for async status
 * records. This particular class is not documented in the latest manual.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, oob_record/async/status/async_class.mi)
{
    gdbwire_mi_oob_record *oob;
    gdbwire_mi_async_record *async;
    gdbwire_mi_result *result;

    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_STATUS,
        GDBWIRE_MI_ASYNC_DOWNLOAD);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_STATUS,
        GDBWIRE_MI_ASYNC_UNSUPPORTED);
    REQUIRE(result);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * A simple async exec output tree.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, oob_record/async/exec/basic.mi)
{
    gdbwire_mi_oob_record *oob;
    gdbwire_mi_async_record *async;
    gdbwire_mi_result *result;

    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);

    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_EXEC,
        GDBWIRE_MI_ASYNC_RUNNING);
    result = CHECK_RESULT_CSTRING(result, "thread-id", "all");

    REQUIRE(!result);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * All of the supported async class's for the exec kind.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, oob_record/async/exec/async_class.mi)
{
    gdbwire_mi_oob_record *oob;
    gdbwire_mi_async_record *async;
    gdbwire_mi_result *result;

    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_EXEC,
        GDBWIRE_MI_ASYNC_STOPPED);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_EXEC,
        GDBWIRE_MI_ASYNC_RUNNING);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_EXEC,
        GDBWIRE_MI_ASYNC_UNSUPPORTED);
    REQUIRE(result);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * A simple async notify output tree.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, oob_record/async/notify/basic.mi)
{
    gdbwire_mi_oob_record *oob;
    gdbwire_mi_async_record *async;
    gdbwire_mi_result *result;

    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);

    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_NOTIFY,
            GDBWIRE_MI_ASYNC_BREAKPOINT_CREATED);
    REQUIRE(result);
    REQUIRE(!result->next);

    result = CHECK_RESULT_VARIANT(result, GDBWIRE_MI_TUPLE, "bkpt");
    REQUIRE(result);
    result = CHECK_RESULT_CSTRING(result, "number", "2");
    result = CHECK_RESULT_CSTRING(result, "type", "breakpoint");
    result = CHECK_RESULT_CSTRING(result, "line", "9");

    REQUIRE(!result);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * All of the supported async class's for the notify kind.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, oob_record/async/notify/async_class.mi)
{
    gdbwire_mi_oob_record *oob;
    gdbwire_mi_async_record *async;
    gdbwire_mi_result *result;

    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_NOTIFY,
            GDBWIRE_MI_ASYNC_THREAD_GROUP_ADDED);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_NOTIFY,
            GDBWIRE_MI_ASYNC_THREAD_GROUP_REMOVED);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_NOTIFY,
            GDBWIRE_MI_ASYNC_THREAD_GROUP_STARTED);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_NOTIFY,
            GDBWIRE_MI_ASYNC_THREAD_GROUP_EXITED);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_NOTIFY,
            GDBWIRE_MI_ASYNC_THREAD_CREATED);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_NOTIFY,
            GDBWIRE_MI_ASYNC_THREAD_EXITED);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_NOTIFY,
            GDBWIRE_MI_ASYNC_THREAD_SELECTED);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_NOTIFY,
            GDBWIRE_MI_ASYNC_LIBRARY_LOADED);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_NOTIFY,
            GDBWIRE_MI_ASYNC_LIBRARY_UNLOADED);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_NOTIFY,
            GDBWIRE_MI_ASYNC_TRACEFRAME_CHANGED);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_NOTIFY,
        GDBWIRE_MI_ASYNC_TSV_CREATED);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_NOTIFY,
        GDBWIRE_MI_ASYNC_TSV_MODIFIED);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_NOTIFY,
        GDBWIRE_MI_ASYNC_TSV_DELETED);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_NOTIFY,
            GDBWIRE_MI_ASYNC_BREAKPOINT_CREATED);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_NOTIFY,
            GDBWIRE_MI_ASYNC_BREAKPOINT_MODIFIED);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_NOTIFY,
            GDBWIRE_MI_ASYNC_BREAKPOINT_DELETED);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_NOTIFY,
            GDBWIRE_MI_ASYNC_RECORD_STARTED);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_NOTIFY,
            GDBWIRE_MI_ASYNC_RECORD_STOPPED);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_NOTIFY,
            GDBWIRE_MI_ASYNC_CMD_PARAM_CHANGED);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_NOTIFY,
            GDBWIRE_MI_ASYNC_MEMORY_CHANGED);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_NOTIFY,
            GDBWIRE_MI_ASYNC_UNSUPPORTED);
    REQUIRE(result);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * A simple out of band record with multiple async records of different kinds.
 *
 * This test is intended to show that multiple different async records (in
 * any order) can be contained in a single out of band record.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, oob_record/async/combo/basic.mi)
{
    gdbwire_mi_oob_record *oob;
    gdbwire_mi_async_record *async;
    gdbwire_mi_result *result;

    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_EXEC,
        GDBWIRE_MI_ASYNC_RUNNING);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_NOTIFY,
            GDBWIRE_MI_ASYNC_BREAKPOINT_CREATED);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_STATUS,
        GDBWIRE_MI_ASYNC_DOWNLOAD);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_NOTIFY,
            GDBWIRE_MI_ASYNC_BREAKPOINT_CREATED);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_EXEC,
        GDBWIRE_MI_ASYNC_STOPPED);
    REQUIRE(result);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test the result record can have a NULL result field.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, oob_record/async/result/null.mi)
{
    gdbwire_mi_oob_record *oob;
    gdbwire_mi_async_record *async;
    gdbwire_mi_result *result;

    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_NOTIFY,
        GDBWIRE_MI_ASYNC_TSV_DELETED);
    REQUIRE(!result);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * A simple out of band record with multiple stream and async records.
 *
 * This test is intended to show that multiple different stream and async
 * records can be contained in a single out of band record.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, oob_record/combo/basic.mi)
{
    std::string console1 = "console line 1";
    std::string console2 = "console line 2";
    std::string target1 = "target line 1";
    std::string log1 = "log line 1";
    std::string log2 = "log line 2";

    gdbwire_mi_oob_record *oob;
    gdbwire_mi_stream_record *stream_record;
    gdbwire_mi_async_record *async;
    gdbwire_mi_result *result;

    oob = CHECK_OUTPUT_OOB_RECORD(output);
    stream_record = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream_record, GDBWIRE_MI_CONSOLE, console1);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_EXEC,
        GDBWIRE_MI_ASYNC_RUNNING);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    stream_record = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream_record, GDBWIRE_MI_CONSOLE, console2);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    stream_record = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream_record, GDBWIRE_MI_TARGET, target1);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_STATUS,
        GDBWIRE_MI_ASYNC_DOWNLOAD);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_NOTIFY,
            GDBWIRE_MI_ASYNC_BREAKPOINT_CREATED);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    stream_record = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream_record, GDBWIRE_MI_LOG, log1);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBWIRE_MI_EXEC,
        GDBWIRE_MI_ASYNC_STOPPED);
    REQUIRE(result);

    output = output->next;
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    stream_record = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream_record, GDBWIRE_MI_LOG, log2);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test the token field of a result record.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, result_record/token/basic.mi)
{
    gdbwire_mi_result *result;

    result = CHECK_OUTPUT_RESULT_RECORD(output, GDBWIRE_MI_ERROR, "512");
    REQUIRE(result);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test the done result class of a result record.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, result_record/result_class/done.mi)
{
    gdbwire_mi_result *result;

    result = CHECK_OUTPUT_RESULT_RECORD(output, GDBWIRE_MI_DONE);
    REQUIRE(result);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test the running result class of a result record.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, result_record/result_class/running.mi)
{
    gdbwire_mi_result *result;

    result = CHECK_OUTPUT_RESULT_RECORD(output, GDBWIRE_MI_RUNNING);
    REQUIRE(!result);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test the connected result class of a result record.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, result_record/result_class/connected.mi)
{
    gdbwire_mi_result *result;

    result = CHECK_OUTPUT_RESULT_RECORD(output, GDBWIRE_MI_CONNECTED);
    REQUIRE(!result);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test the error result class of a result record.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, result_record/result_class/error.mi)
{
    std::string expected =
        "Undefined command: \"null\".  Try \"help\".";
    gdbwire_mi_result *result;

    result = CHECK_OUTPUT_RESULT_RECORD(output, GDBWIRE_MI_ERROR);
    result = CHECK_RESULT_CSTRING(result, "msg", expected);
    REQUIRE(!result);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test the exit result class of a result record.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, result_record/result_class/exit.mi)
{
    gdbwire_mi_result *result;

    result = CHECK_OUTPUT_RESULT_RECORD(output, GDBWIRE_MI_EXIT);
    REQUIRE(!result);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test the unsupported result class of a result record.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, result_record/result_class/unsupported.mi)
{
    std::string expected = "An unsupported result class can have a result!";
    gdbwire_mi_result *result;

    result = CHECK_OUTPUT_RESULT_RECORD(output, GDBWIRE_MI_UNSUPPORTED);
    result = CHECK_RESULT_CSTRING(result, "msg", expected);
    REQUIRE(!result);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test the result record can have a NULL result field.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, result_record/result/null.mi)
{
    gdbwire_mi_result *result;

    result = CHECK_OUTPUT_RESULT_RECORD(output, GDBWIRE_MI_EXIT);
    REQUIRE(!result);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test a value only cstring in a result record.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, result/cstring/value.mi)
{
    gdbwire_mi_result *result = GET_RESULT(output);

    result = CHECK_RESULT_CSTRING(result, "", "value");
    REQUIRE(!result);
    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test a key/value cstring in a result record.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, result/cstring/key_value.mi)
{
    gdbwire_mi_result *result = GET_RESULT(output);

    result = CHECK_RESULT_CSTRING(result, "key", "value");
    REQUIRE(!result);
    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test a key/value cstring in a result record with whitespace added.
 *
 * Try spaces and tabs between the key, the equal sign and the value.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, result/cstring/key_value_whitespace.mi)
{
    gdbwire_mi_result *result = GET_RESULT(output);

    result = CHECK_RESULT_CSTRING(result, "key", "value");
    REQUIRE(!result);
    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test a null tuple result record, ie. {}.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, result/tuple/null.mi)
{
    gdbwire_mi_result *result = GET_RESULT(output);
    REQUIRE(!result->next);

    result = CHECK_RESULT_VARIANT(result, GDBWIRE_MI_TUPLE);
    REQUIRE(!result);
    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test a null tuple result record with a key, ie. {}.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, result/tuple/key_null.mi)
{
    gdbwire_mi_result *result = GET_RESULT(output);
    REQUIRE(!result->next);

    result = CHECK_RESULT_VARIANT(result, GDBWIRE_MI_TUPLE, "key");
    REQUIRE(!result);
    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test a tuple result record with no key and only a value, ie. {"value"}.
 *
 * Tuples are required to have keys, although gdbwire allows them
 * to not have keys as this has been reported in the field as
 * being possible.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, result/tuple/no_key.mi)
{
    gdbwire_mi_result *result = GET_RESULT(output);
    REQUIRE(!result->next);

    result = CHECK_RESULT_VARIANT(result, GDBWIRE_MI_TUPLE);
    result = CHECK_RESULT_CSTRING(result, "", "value");
    REQUIRE(!result);
    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test a tuple result record with a cstring element
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, result/tuple/of_cstring.mi)
{
    gdbwire_mi_result *result = GET_RESULT(output);
    REQUIRE(!result->next);

    result = CHECK_RESULT_VARIANT(result, GDBWIRE_MI_TUPLE);
    result = CHECK_RESULT_CSTRING(result, "key", "value");
    REQUIRE(!result);
    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test a tuple result record with two cstring elements
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, result/tuple/of_2_cstring.mi)
{
    gdbwire_mi_result *result = GET_RESULT(output);
    REQUIRE(!result->next);

    result = CHECK_RESULT_VARIANT(result, GDBWIRE_MI_TUPLE);
    result = CHECK_RESULT_CSTRING(result, "key", "value");
    result = CHECK_RESULT_CSTRING(result, "key2", "value2");
    REQUIRE(!result);
    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test a tuple result record with three cstring elements
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, result/tuple/of_3_cstring.mi)
{
    gdbwire_mi_result *result = GET_RESULT(output);
    REQUIRE(!result->next);

    result = CHECK_RESULT_VARIANT(result, GDBWIRE_MI_TUPLE);
    result = CHECK_RESULT_CSTRING(result, "key", "value");
    result = CHECK_RESULT_CSTRING(result, "key2", "value2");
    result = CHECK_RESULT_CSTRING(result, "key3", "value3");
    REQUIRE(!result);
    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test a tuple result record of a null tuple.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, result/tuple/of_null_tuple.mi)
{
    gdbwire_mi_result *result = GET_RESULT(output);
    REQUIRE(!result->next);

    result = CHECK_RESULT_VARIANT(result, GDBWIRE_MI_TUPLE);
    REQUIRE(!result->next);

    result = CHECK_RESULT_VARIANT(result, GDBWIRE_MI_TUPLE, "key");
    REQUIRE(!result);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test a null list result record, ie. [].
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, result/list/null.mi)
{
    gdbwire_mi_result *result = GET_RESULT(output);
    REQUIRE(!result->next);

    result = CHECK_RESULT_VARIANT(result, GDBWIRE_MI_LIST);
    REQUIRE(!result);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test a null list result record with a key, ie. [].
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, result/list/key_null.mi)
{
    gdbwire_mi_result *result = GET_RESULT(output);
    REQUIRE(!result->next);

    result = CHECK_RESULT_VARIANT(result, GDBWIRE_MI_LIST, "key");
    REQUIRE(!result);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test a list result record with a cstring element
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, result/list/of_cstring.mi)
{
    gdbwire_mi_result *result = GET_RESULT(output);
    REQUIRE(!result->next);

    result = CHECK_RESULT_VARIANT(result, GDBWIRE_MI_LIST);
    result = CHECK_RESULT_CSTRING(result, "key", "value");
    REQUIRE(!result);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test a list result record with two cstring elements
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, result/list/of_2_cstring.mi)
{
    gdbwire_mi_result *result = GET_RESULT(output);
    REQUIRE(!result->next);

    result = CHECK_RESULT_VARIANT(result, GDBWIRE_MI_LIST);
    result = CHECK_RESULT_CSTRING(result, "key", "value");
    result = CHECK_RESULT_CSTRING(result, "key2", "value2");
    REQUIRE(!result);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test a list result record with three cstring elements
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, result/list/of_3_cstring.mi)
{
    gdbwire_mi_result *result = GET_RESULT(output);
    REQUIRE(!result->next);

    result = CHECK_RESULT_VARIANT(result, GDBWIRE_MI_LIST);
    result = CHECK_RESULT_CSTRING(result, "key", "value");
    result = CHECK_RESULT_CSTRING(result, "", "value2");
    result = CHECK_RESULT_CSTRING(result, "key3", "value3");
    REQUIRE(!result);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test a list result record of a null list.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, result/list/of_null_list.mi)
{
    gdbwire_mi_result *result = GET_RESULT(output);
    REQUIRE(!result->next);

    result = CHECK_RESULT_VARIANT(result, GDBWIRE_MI_LIST);
    REQUIRE(!result->next);

    result = CHECK_RESULT_VARIANT(result, GDBWIRE_MI_LIST, "key");
    REQUIRE(!result);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test a result record with many next pointers.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, result/mixed/next.mi)
{
    gdbwire_mi_result *top_result = GET_RESULT(output), *result;

    result = CHECK_RESULT_VARIANT(top_result, GDBWIRE_MI_LIST, "key");
    result = CHECK_RESULT_CSTRING(result, "key2", "value2");
    REQUIRE(!result);

    REQUIRE(top_result->next);
    top_result = top_result->next;

    result = CHECK_RESULT_VARIANT(top_result, GDBWIRE_MI_TUPLE, "key3");
    result = CHECK_RESULT_CSTRING(result, "key4", "value4");
    result = CHECK_RESULT_CSTRING(result, "key5", "value5");
    REQUIRE(!result);

    REQUIRE(top_result->next);
    top_result = top_result->next;

    result = CHECK_RESULT_VARIANT(top_result, GDBWIRE_MI_LIST);
    result = CHECK_RESULT_CSTRING(result, "key6", "value6");
    result = CHECK_RESULT_CSTRING(result, "", "value7");
    REQUIRE(!result);

    REQUIRE(!top_result->next);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test a recursive result record.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, result/mixed/recursive.mi)
{
    gdbwire_mi_result *top_result = GET_RESULT(output),
        *result, *inside_result;
    REQUIRE(!top_result->next);

    result = CHECK_RESULT_VARIANT(top_result, GDBWIRE_MI_TUPLE);
    result = CHECK_RESULT_CSTRING(result, "key", "value");
    REQUIRE(!result->next);

    result = CHECK_RESULT_VARIANT(result, GDBWIRE_MI_TUPLE, "key2");

    inside_result = CHECK_RESULT_VARIANT(result, GDBWIRE_MI_LIST, "key3");
    inside_result = CHECK_RESULT_CSTRING(inside_result, "", "value3");
    inside_result = CHECK_RESULT_CSTRING(inside_result, "", "value4");
    REQUIRE(!inside_result);

    REQUIRE(result->next);
    result = result->next;

    inside_result = CHECK_RESULT_VARIANT(result, GDBWIRE_MI_TUPLE, "key5");
    inside_result = CHECK_RESULT_CSTRING(inside_result, "key6", "value6");
    inside_result = CHECK_RESULT_CSTRING(inside_result, "key7", "value7");
    REQUIRE(!inside_result);

    REQUIRE(!result->next);

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test the line field when the output kind is GDBWIRE_MI_OUTPUT_OOB.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, line/oob.mi)
{
    std::string expected = "Hello World console output";
    struct gdbwire_mi_oob_record *oob;
    struct gdbwire_mi_stream_record *stream;

    REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_OOB);
    oob = CHECK_OUTPUT_OOB_RECORD(output);
    stream = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream, GDBWIRE_MI_CONSOLE, expected);

    REQUIRE(std::string(output->line) == "~\"Hello World console output\"\n");

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test the line field when the output kind is GDBWIRE_MI_OUTPUT_RESULT.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, line/result.mi)
{
    gdbwire_mi_result *result;

    REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_RESULT);
    result = CHECK_OUTPUT_RESULT_RECORD(output, GDBWIRE_MI_EXIT);
    REQUIRE(!result);

    REQUIRE(std::string(output->line) == "^exit\n");

    CHECK_OUTPUT_AT_FINAL_PROMPT(output->next);
}

/**
 * Test the line field when the output kind is GDBWIRE_MI_OUTPUT_PROMPT.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, line/prompt.mi)
{
    REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_PROMPT);
    REQUIRE(std::string(output->line) == "(gdb)\n");
    CHECK_OUTPUT_AT_FINAL_PROMPT(output);
}

/**
 * Test that an empty MI command is an error.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, parse_error/syntax/empty.mi)
{
    REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_PARSE_ERROR);
    REQUIRE(std::string(output->line) == "\n");
    REQUIRE(output->variant.error.token);
    REQUIRE(std::string(output->variant.error.token) == "\n");
    REQUIRE(output->variant.error.pos.start_column == 1);
    REQUIRE(output->variant.error.pos.end_column == 1);
}

/**
 * Test the error at the front of the line.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, parse_error/syntax/front.mi)
{
    REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_PARSE_ERROR);
    REQUIRE(std::string(output->line) == "$error\n");
    REQUIRE(output->variant.error.token);
    REQUIRE(std::string(output->variant.error.token) == "$");
    REQUIRE(output->variant.error.pos.start_column == 1);
    REQUIRE(output->variant.error.pos.end_column == 1);
}

/**
 * Test the error in the middle of the line.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, parse_error/syntax/middle.mi)
{
    REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_PARSE_ERROR);
    REQUIRE(std::string(output->line) == "*running, abc {}\n");
    REQUIRE(std::string(output->variant.error.token) == "{");
    REQUIRE(output->variant.error.pos.start_column == 15);
    REQUIRE(output->variant.error.pos.end_column == 15);
}

/**
 * Test the error at the end of the line.
 *
 * Extra tokens exist on the line at the end in this case.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, parse_error/syntax/end.mi)
{
    REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_PARSE_ERROR);
    REQUIRE(std::string(output->line) == "^error abc\n");
    REQUIRE(output->variant.error.token);
    REQUIRE(std::string(output->variant.error.token) == "abc");
    REQUIRE(output->variant.error.pos.start_column == 8);
    REQUIRE(output->variant.error.pos.end_column == 10);
}

/**
 * Test the error when there is missing information on a line.
 *
 * There are missing tokens on the line at the end.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, parse_error/syntax/end_missing.mi)
{
    REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_PARSE_ERROR);
    REQUIRE(std::string(output->line) == "^\n");
    REQUIRE(std::string(output->variant.error.token) == "\n");
    REQUIRE(output->variant.error.pos.start_column == 2);
    REQUIRE(output->variant.error.pos.end_column == 2);
}

/**
 * Test the error in a list grammar rule.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, parse_error/syntax/list_of_2_cstring.mi)
{
    REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_PARSE_ERROR);
    REQUIRE(std::string(output->line) ==
        "*stopped,[key=\"value\", key2= \" \"value2\"]\n");
    REQUIRE(std::string(output->variant.error.token) == "value2");
    REQUIRE(output->variant.error.pos.start_column == 33);
    REQUIRE(output->variant.error.pos.end_column == 38);
}

/**
 * Test that the same parser can handle errors on many lines.
 *
 * This test proves the parser can have errors on some lines and
 * then recover on other lines after that successfully.
 *
 * I've added an error which is forced manually in the bison grammar
 * and then expecting the next line to parse successfully. The error
 * I'm talking about is '(not_gdb)' which is a gdb prompt line.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, parse_error/syntax/multi_line_error.mi)
{
    REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_PARSE_ERROR);
    REQUIRE(std::string(output->line) == "^error^\n");
    REQUIRE(std::string(output->variant.error.token) == "^");
    REQUIRE(output->variant.error.pos.start_column == 7);
    REQUIRE(output->variant.error.pos.end_column == 7);

    REQUIRE(output->next);
    output = output->next;

    REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_PROMPT);
    REQUIRE(output->next);
    output = output->next;

    REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_PARSE_ERROR);
    REQUIRE(std::string(output->line) == "^\n");
    REQUIRE(std::string(output->variant.error.token) == "\n");
    REQUIRE(output->variant.error.pos.start_column == 2);
    REQUIRE(output->variant.error.pos.end_column == 2);

    REQUIRE(output->next);
    output = output->next;

    REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_PARSE_ERROR);
    REQUIRE(std::string(output->line) == "^error abc\n");
    REQUIRE(std::string(output->variant.error.token) == "abc");
    REQUIRE(output->variant.error.pos.start_column == 8);
    REQUIRE(output->variant.error.pos.end_column == 10);

    REQUIRE(output->next);
    output = output->next;

    REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_PROMPT);

    REQUIRE(output->next);
    output = output->next;

    {
        gdbwire_mi_result *result =
            CHECK_OUTPUT_RESULT_RECORD(output, GDBWIRE_MI_ERROR);
        result = CHECK_RESULT_CSTRING(result, "msg", "bogus");
        REQUIRE(!result);
    }

    REQUIRE(output->next);
    output = output->next;

    REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_PROMPT);

    REQUIRE(output->next);
    output = output->next;

    REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_PARSE_ERROR);
    REQUIRE(std::string(output->line) == "(not_gdb)\n");
    REQUIRE(std::string(output->variant.error.token) == "not_gdb");
    REQUIRE(output->variant.error.pos.start_column == 2);
    REQUIRE(output->variant.error.pos.end_column == 8);

    REQUIRE(output->next);
    output = output->next;

    REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_PROMPT);
    REQUIRE(!output->next);
}

/**
 * Test that the GDB prompt says 'gdb'.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, parse_error/syntax/prompt.mi)
{
    REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_PARSE_ERROR);
    REQUIRE(std::string(output->line) == "(not_gdb)\n");
    REQUIRE(output->variant.error.token);
    REQUIRE(std::string(output->variant.error.token) == "not_gdb");
    REQUIRE(output->variant.error.pos.start_column == 2);
    REQUIRE(output->variant.error.pos.end_column == 8);
}

/**
 * Test the token destructor functionality (verify with gcov).
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, parse_error/syntax/dtor/token.mi)
{
    REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_PARSE_ERROR);
    REQUIRE(std::string(output->line) == "543#\n");
    REQUIRE(output->variant.error.token);
    REQUIRE(std::string(output->variant.error.token) == "#");
    REQUIRE(output->variant.error.pos.start_column == 4);
    REQUIRE(output->variant.error.pos.end_column == 4);
}

/**
 * Test the variable destructor functionality (verify with gcov).
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, parse_error/syntax/dtor/variable.mi)
{
    REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_PARSE_ERROR);
    REQUIRE(std::string(output->line) == "(gdb#\n");
    REQUIRE(output->variant.error.token);
    REQUIRE(std::string(output->variant.error.token) == "#");
    REQUIRE(output->variant.error.pos.start_column == 5);
    REQUIRE(output->variant.error.pos.end_column == 5);
}

/**
 * Test the variable destructor functionality (verify with gcov).
 *
 * This is testing an edge case in the parser. The gdb prompt rule looks
 * like this: ( variable ). The rule in the grammar has a mid rule action
 * that asserts that the variable is "gdb". If it is not "gdb" it manually
 * raises an error. This ensures that the manual error is handled by the
 * destructor to ensure no memory is lost.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, parse_error/syntax/dtor/mid_action_var.mi)
{
    REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_PARSE_ERROR);
    REQUIRE(std::string(output->line) == "(not_gdb)\n");
    REQUIRE(output->variant.error.token);
    REQUIRE(std::string(output->variant.error.token) == "not_gdb");
    REQUIRE(output->variant.error.pos.start_column == 2);
    REQUIRE(output->variant.error.pos.end_column == 8);
}

/**
 * Test the opt_variable destructor functionality (verify with gcov).
 *
 * This also triggers result_list. There is no way to isolate these two
 * at this point in time.
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, parse_error/syntax/dtor/opt_variable.mi)
{
    REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_PARSE_ERROR);
    REQUIRE(std::string(output->line) == "*stopped,reason=#\n");
    REQUIRE(output->variant.error.token);
    REQUIRE(std::string(output->variant.error.token) == "#");
    REQUIRE(output->variant.error.pos.start_column == 17);
    REQUIRE(output->variant.error.pos.end_column == 17);
}

/**
 * Test the result destructor functionality (verify with gcov).
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, parse_error/syntax/dtor/result.mi)
{
    REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_PARSE_ERROR);
    REQUIRE(std::string(output->line) == "*stopped,{\"abc\",^\n");
    REQUIRE(output->variant.error.token);
    REQUIRE(std::string(output->variant.error.token) == "^");
    REQUIRE(output->variant.error.pos.start_column == 17);
    REQUIRE(output->variant.error.pos.end_column == 17);
}

/**
 * Test the result_list destructor functionality (verify with gcov).
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, parse_error/syntax/dtor/result_list.mi)
{
    REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_PARSE_ERROR);
    REQUIRE(std::string(output->line) == "*stopped,{\"abc\",\"def\"^\n");
    REQUIRE(output->variant.error.token);
    REQUIRE(std::string(output->variant.error.token) == "^");
    REQUIRE(output->variant.error.pos.start_column == 22);
    REQUIRE(output->variant.error.pos.end_column == 22);
}

/**
 * Test the output_variant destructor functionality (verify with gcov).
 */
TEST_CASE_METHOD_N(GdbwireMiPtTest, parse_error/syntax/dtor/output_variant.mi)
{
    REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_PARSE_ERROR);
    REQUIRE(std::string(output->line) == "^error^\n");
    REQUIRE(output->variant.error.token);
    REQUIRE(std::string(output->variant.error.token) == "^");
    REQUIRE(output->variant.error.pos.start_column == 7);
    REQUIRE(output->variant.error.pos.end_column == 7);
}

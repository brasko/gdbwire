#include <errno.h>
#include <stdio.h>
#include "catch.hpp"
#include "fixture.h"
#include "gdbmi/gdbmi_pt.h"
#include "gdbmi/gdbmi_pt_print.h"
#include "gdbmi/gdbmi_parser.h"

/**
 * The GDB/MI parse tree unit tests.
 *
 * Unit testing a parse tree is a non trivial task. A parse tree can have
 * many variations and it's often difficult to test them all in isolation.
 *
 * The goal of the following unit tests is to isolate testing as many
 * combinations of GDB/MI parse trees as possible. Hoping to achieve 100%
 * coverage on the gdbmi_grammar.y file.
 *
 * These unit tests will not be concerned with the semantics of the parse
 * tree, but simply validating that all combinations of GDB/MI output
 * commands can be parsed and turned into an appropriate parse tree.
 */

namespace {
    struct GdbmiParserCallback {
        GdbmiParserCallback() : m_output(0) {
            callbacks.context = (void*)this;
            callbacks.gdbmi_output_callback =
                    GdbmiParserCallback::gdbmi_output_callback;
        }

        ~GdbmiParserCallback() {
            gdbmi_output_free(m_output);
        }

        static void gdbmi_output_callback(void *context, gdbmi_output *output) {
            GdbmiParserCallback *callback = (GdbmiParserCallback *)context;
            callback->gdbmi_output_callback(output);
        }

        void gdbmi_output_callback(gdbmi_output *output) {
            m_output = append_gdbmi_output(m_output, output);
        }

        gdbmi_parser_callbacks callbacks;
        gdbmi_output *m_output;
    };

    struct GdbmiPtTest : public Fixture {
        GdbmiPtTest() {
            input = sourceTestDir() + "/input.mi";
            parser = gdbmi_parser_create(parserCallback.callbacks);
            REQUIRE(parser);
            output = parse(parser, input);
            REQUIRE(output);
        }
        
        ~GdbmiPtTest() {
            gdbmi_parser_destroy(parser);
        }

        /**
         * Parse a GDB/MI file and return a gdbmi_output structure.
         *
         * @param parser
         * The gdb mi parser to do the parsing
         *
         * @param input
         * The input file to parse
         *
         * @return
         * A gdbmi_output structure representing the input file.
         * You are responsible for destroying this memory.
         */
        gdbmi_output *parse(gdbmi_parser *parser, const std::string &input) {
            FILE *fd;
            int c;

            fd = fopen(input.c_str(), "r");
            REQUIRE(fd);

            while ((c = fgetc(fd)) != EOF) {
                char data[2] = { (char)c, 0 };
                REQUIRE(gdbmi_parser_push(parser, data) == GDBWIRE_OK);
            }
            fclose(fd);

            return parserCallback.m_output;
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
        gdbmi_stream_record *CHECK_OOB_RECORD_STREAM(gdbmi_oob_record *oob) {
            REQUIRE(oob);
            REQUIRE(oob->kind == GDBMI_STREAM);
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
        gdbmi_async_record *CHECK_OOB_RECORD_ASYNC(gdbmi_oob_record *oob) {
            REQUIRE(oob);
            REQUIRE(oob->kind == GDBMI_ASYNC);
            REQUIRE(oob->variant.async_record);
            return oob->variant.async_record;
        }

        /**
         * A utility function for checking the values in a gdbmi_stream_record.
         *
         * If the values do not match the record, an assertion failure is made.
         *
         * @param record
         * The gdbmi stream record to check the values of.
         *
         * @param kind
         * The kind of record to check for.
         *
         * @param expected
         * The expected cstring value to check for.
         */
        void CHECK_STREAM_RECORD(gdbmi_stream_record *record,
            gdbmi_stream_record_kind kind, const std::string &expected) {
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
         * The expected token value.
         *
         * @return
         * The gdbmi result of the async record (may be NULL);
         */
        gdbmi_result *CHECK_ASYNC_RECORD(
            gdbmi_async_record *async_record,
            gdbmi_async_record_kind kind, gdbmi_async_class async_class,
            gdbmi_token_t token = -1) {
            REQUIRE(async_record);
            REQUIRE(async_record->token == token);
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
         * The gdbmi result to check the variable value of.
         *
         * @param value
         * The expected variable value. If empty, the result's variable
         * should be NULL.
         */
        void CHECK_RESULT_VARIABLE(gdbmi_result *result,
            const std::string &value = "") {
            REQUIRE(result);

            if (value.empty()) {
                REQUIRE(!result->variable);
            } else {
                REQUIRE(value == result->variable);
            }
        }

        /**
         * Check that the cstring result matches the corresponding parameters.
         *
         * If the values do not match the result, an assertion failure is made.
         *
         * @param result
         * The gdbmi result to check the values of.
         *
         * @param variable
         * The result variable name or empty string if none.
         *
         * @param expected
         * The expected cstring value.
         *
         * @return
         * Returns the next gdbmi_result pointer.
         */
        gdbmi_result *CHECK_RESULT_CSTRING(gdbmi_result *result,
            const std::string &variable, const std::string &expected) {

            CHECK_RESULT_VARIABLE(result, variable);

            REQUIRE(result->kind == GDBMI_CSTRING);
            REQUIRE(expected == result->variant.cstring);

            return result->next;
        }

        /**
         * Check the tuple result matches the corresponding parameters.
         *
         * If the values do not match the result, an assertion failure is made.
         *
         * @param result
         * The gdbmi result to check the values of.
         *
         * @param variable
         * The result variable name or empty string if none.
         *
         * @return
         * Returns the next gdbmi_result pointer.
         */
        gdbmi_result *CHECK_RESULT_TUPLE(gdbmi_result *result,
            const std::string &variable = "") {

            CHECK_RESULT_VARIABLE(result, variable);

            REQUIRE(result->kind == GDBMI_TUPLE);
            REQUIRE(result->variant.result);
            return result->variant.result;
        }

        GdbmiParserCallback parserCallback;
        std::string input;
        gdbmi_parser *parser;
        gdbmi_output *output;
    };
}

TEST_CASE_METHOD_N(GdbmiPtTest, basic)
{
    //REQUIRE(print_gdbmi_output(output) == 0);
}

/**
 * A simple console output parse tree.
 */
TEST_CASE_METHOD_N(GdbmiPtTest, oob_record/stream/console/basic)
{
    std::string expected = "\"Hello World console output\"";
    struct gdbmi_oob_record *oob = output->oob_record;
    struct gdbmi_stream_record *stream;

    stream = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream, GDBMI_CONSOLE, expected);

    REQUIRE(!oob->next);

    REQUIRE(!output->result_record);
    REQUIRE(!output->next);
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
TEST_CASE_METHOD_N(GdbmiPtTest, oob_record/stream/console/characters)
{
    std::string expected =
        "\"$1 = "
        "\\\"\\\\000\\\\001\\\\002\\\\003\\\\004\\\\005\\\\006\\\\a"
        "\\\\b\\\\t\\\\n\\\\v\\\\f\\\\r\\\\016\\\\017"
        "\\\\020\\\\021\\\\022\\\\023\\\\024\\\\025\\\\026\\\\027"
        "\\\\030\\\\031\\\\032\\\\033\\\\034\\\\035\\\\036\\\\037"
        " !\\\\\\\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "[\\\\\\\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\\\\177"
        "\\\\200\\\\201\\\\202\\\\203\\\\204\\\\205\\\\206\\\\207"
        "\\\\210\\\\211\\\\212\\\\213\\\\214\\\\215\\\\216\\\\217"
        "\\\\220\\\\221\\\\222\\\\223\\\\224\\\\225\\\\226\\\\227"
        "\\\\230\\\\231\\\\232\\\\233\\\\234\\\\235\\\\236\\\\237"
        "\\\\240\\\\241\\\\242\\\\243\\\\244\\\\245\\\\246\\\\247"
        "\\\\250\\\\251\\\\252\\\\253\\\\254\\\\255\\\\256\\\\257"
        "\\\\260\\\\261\\\\262\\\\263\\\\264\\\\265\\\\266\\\\267"
        "\\\\270\\\\271\\\\272\\\\273\\\\274\\\\275\\\\276\\\\277"
        "\\\\300\\\\301\\\\302\\\\303\\\\304\\\\305\\\\306\\\\307"
        "\\\\310\\\\311\\\\312\\\\313\\\\314\\\\315\\\\316\\\\317"
        "\\\\320\\\\321\\\\322\\\\323\\\\324\\\\325\\\\326\\\\327"
        "\\\\330\\\\331\\\\332\\\\333\\\\334\\\\335\\\\336\\\\337"
        "\\\\340\\\\341\\\\342\\\\343\\\\344\\\\345\\\\346\\\\347"
        "\\\\350\\\\351\\\\352\\\\353\\\\354\\\\355\\\\356\\\\357"
        "\\\\360\\\\361\\\\362\\\\363\\\\364\\\\365\\\\366\\\\367"
        "\\\\370\\\\371\\\\372\\\\373\\\\374\\\\375\\\\376\\\\377\\\"\"";
    struct gdbmi_oob_record *oob = output->oob_record;
    struct gdbmi_stream_record *stream;

    stream = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream, GDBMI_CONSOLE, expected);

    REQUIRE(!oob->next);

    REQUIRE(!output->result_record);
    REQUIRE(!output->next);
}

/**
 * A simple target output parse tree.
 */
TEST_CASE_METHOD_N(GdbmiPtTest, oob_record/stream/target/basic)
{
    std::string expected = "\"Hello World target output\"";
    struct gdbmi_oob_record *oob = output->oob_record;
    struct gdbmi_stream_record *stream;

    stream = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream, GDBMI_TARGET, expected);

    REQUIRE(!oob->next);

    REQUIRE(!output->result_record);
    REQUIRE(!output->next);
}

/**
 * A simple log output parse tree.
 */
TEST_CASE_METHOD_N(GdbmiPtTest, oob_record/stream/log/basic)
{
    std::string expected = "\"Hello World log output\"";
    struct gdbmi_oob_record *oob = output->oob_record;
    struct gdbmi_stream_record *stream;

    stream = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream, GDBMI_LOG, expected);
    REQUIRE(!oob->next);

    REQUIRE(!output->result_record);
    REQUIRE(!output->next);
}

/**
 * A simple out of band record with multiple streams of different kinds.
 *
 * This test is intended to show that multiple different stream records (in
 * any order) can be contained in a single out of band record.
 */
TEST_CASE_METHOD_N(GdbmiPtTest, oob_record/stream/combo/basic)
{
    std::string console1 = "\"console line 1\"";
    std::string console2 = "\"console line 2\"";
    std::string target1 = "\"target line 1\"";
    std::string log1 = "\"log line 1\"";
    std::string target2 = "\"target line 2\"";
    std::string log2 = "\"log line 2\"";
    std::string console3 = "\"console line 3\"";

    struct gdbmi_oob_record *oob = output->oob_record;
    struct gdbmi_stream_record *stream;

    stream = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream, GDBMI_CONSOLE, console1);
    oob = oob->next;

    stream = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream, GDBMI_CONSOLE, console2);
    oob = oob->next;

    stream = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream, GDBMI_TARGET, target1);
    oob = oob->next;

    stream = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream, GDBMI_LOG, log1);
    oob = oob->next;

    stream = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream, GDBMI_TARGET, target2);
    oob = oob->next;

    stream = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream, GDBMI_LOG, log2);
    oob = oob->next;

    stream = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream, GDBMI_CONSOLE, console3);
    REQUIRE(!oob->next);

    REQUIRE(!output->result_record);
    REQUIRE(!output->next);
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
TEST_CASE_METHOD_N(GdbmiPtTest, oob_record/async/status/basic)
{
    gdbmi_oob_record *oob;
    gdbmi_async_record *async;
    gdbmi_result *result;

    oob = output->oob_record;
    async = CHECK_OOB_RECORD_ASYNC(oob);
    REQUIRE(!oob->next);

    result = CHECK_ASYNC_RECORD(async, GDBMI_STATUS, GDBMI_ASYNC_DOWNLOAD);
    REQUIRE(result);
    REQUIRE(!result->next);

    result = CHECK_RESULT_TUPLE(result);
    result = CHECK_RESULT_CSTRING(result, "section", "\".interp\"");
    result = CHECK_RESULT_CSTRING(result, "section-size", "\"28\"");
    result = CHECK_RESULT_CSTRING(result, "total-size", "\"2466\"");

    REQUIRE(!result);

    REQUIRE(!output->result_record);
    REQUIRE(!output->next);
}

/**
 * All of the supported async class's for the status kind.
 *
 * Currently, +download is the only known async class for async status
 * records. This particular class is not documented in the latest manual.
 */
TEST_CASE_METHOD_N(GdbmiPtTest, oob_record/async/status/async_class)
{
    gdbmi_oob_record *oob;
    gdbmi_async_record *async;
    gdbmi_result *result;

    oob = output->oob_record;
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBMI_STATUS, GDBMI_ASYNC_DOWNLOAD);
    REQUIRE(result);

    REQUIRE(!oob->next);

    REQUIRE(!output->result_record);
    REQUIRE(!output->next);
}

/**
 * A simple async exec output tree.
 */
TEST_CASE_METHOD_N(GdbmiPtTest, oob_record/async/exec/basic)
{
    gdbmi_oob_record *oob;
    gdbmi_async_record *async;
    gdbmi_result *result;

    oob = output->oob_record;
    async = CHECK_OOB_RECORD_ASYNC(oob);
    REQUIRE(!oob->next);

    result = CHECK_ASYNC_RECORD(async, GDBMI_EXEC, GDBMI_ASYNC_RUNNING);
    result = CHECK_RESULT_CSTRING(result, "thread-id", "\"all\"");

    REQUIRE(!result);

    REQUIRE(!output->result_record);
    REQUIRE(!output->next);
}

/**
 * A simple async notify output tree.
 */
TEST_CASE_METHOD_N(GdbmiPtTest, oob_record/async/notify/basic)
{
    gdbmi_oob_record *oob;
    gdbmi_async_record *async;
    gdbmi_result *result;

    oob = output->oob_record;
    async = CHECK_OOB_RECORD_ASYNC(oob);
    REQUIRE(!oob->next);

    result = CHECK_ASYNC_RECORD(async, GDBMI_NOTIFY,
            GDBMI_ASYNC_BREAKPOINT_CREATED);
    REQUIRE(result);
    REQUIRE(!result->next);

    result = CHECK_RESULT_TUPLE(result, "bkpt");
    result = CHECK_RESULT_CSTRING(result, "number", "\"2\"");
    result = CHECK_RESULT_CSTRING(result, "type", "\"breakpoint\"");
    result = CHECK_RESULT_CSTRING(result, "line", "\"9\"");

    REQUIRE(!result);

    REQUIRE(!output->result_record);
    REQUIRE(!output->next);
}

/**
 * A simple out of band record with multiple async records of different kinds.
 *
 * This test is intended to show that multiple different async records (in
 * any order) can be contained in a single out of band record.
 */
TEST_CASE_METHOD_N(GdbmiPtTest, oob_record/async/combo/basic)
{
    gdbmi_oob_record *oob;
    gdbmi_async_record *async;
    gdbmi_result *result;

    oob = output->oob_record;
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBMI_EXEC, GDBMI_ASYNC_RUNNING);
    REQUIRE(result);

    REQUIRE(oob->next);
    oob = oob->next;
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBMI_NOTIFY,
            GDBMI_ASYNC_BREAKPOINT_CREATED);
    REQUIRE(result);

    REQUIRE(oob->next);
    oob = oob->next;
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBMI_STATUS, GDBMI_ASYNC_DOWNLOAD);
    REQUIRE(result);

    REQUIRE(oob->next);
    oob = oob->next;
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBMI_NOTIFY,
            GDBMI_ASYNC_BREAKPOINT_CREATED);
    REQUIRE(result);

    REQUIRE(oob->next);
    oob = oob->next;
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBMI_EXEC, GDBMI_ASYNC_STOPPED);
    REQUIRE(result);

    REQUIRE(!oob->next);

    REQUIRE(!output->result_record);
    REQUIRE(!output->next);
}

/**
 * A simple out of band record with multiple stream and async records.
 *
 * This test is intended to show that multiple different stream and async
 * records can be contained in a single out of band record.
 */
TEST_CASE_METHOD_N(GdbmiPtTest, oob_record/combo/basic)
{
    std::string console1 = "\"console line 1\"";
    std::string console2 = "\"console line 2\"";
    std::string target1 = "\"target line 1\"";
    std::string log1 = "\"log line 1\"";
    std::string log2 = "\"log line 2\"";

    gdbmi_oob_record *oob;
    gdbmi_stream_record *stream_record;
    gdbmi_async_record *async;
    gdbmi_result *result;

    REQUIRE(!output->result_record);
    REQUIRE(!output->next);

    oob = output->oob_record;

    stream_record = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream_record, GDBMI_CONSOLE, console1);

    REQUIRE(oob->next);
    oob = oob->next;
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBMI_EXEC, GDBMI_ASYNC_RUNNING);
    REQUIRE(result);

    REQUIRE(oob->next);
    oob = oob->next;
    stream_record = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream_record, GDBMI_CONSOLE, console2);

    REQUIRE(oob->next);
    oob = oob->next;
    stream_record = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream_record, GDBMI_TARGET, target1);

    REQUIRE(oob->next);
    oob = oob->next;
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBMI_STATUS, GDBMI_ASYNC_DOWNLOAD);
    REQUIRE(result);

    REQUIRE(oob->next);
    oob = oob->next;
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBMI_NOTIFY,
            GDBMI_ASYNC_BREAKPOINT_CREATED);
    REQUIRE(result);

    REQUIRE(oob->next);
    oob = oob->next;
    stream_record = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream_record, GDBMI_LOG, log1);

    REQUIRE(oob->next);
    oob = oob->next;
    async = CHECK_OOB_RECORD_ASYNC(oob);
    result = CHECK_ASYNC_RECORD(async, GDBMI_EXEC, GDBMI_ASYNC_STOPPED);
    REQUIRE(result);

    REQUIRE(oob->next);
    oob = oob->next;
    stream_record = CHECK_OOB_RECORD_STREAM(oob);
    CHECK_STREAM_RECORD(stream_record, GDBMI_LOG, log2);

    REQUIRE(!oob->next);
}

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
                char data[2] = { c, 0 };
                REQUIRE(gdbmi_parser_push(parser, data) == GDBWIRE_OK);
            }
            fclose(fd);

            return parserCallback.m_output;
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
    REQUIRE(output->oob_record);
        struct gdbmi_oob_record *oob = output->oob_record;
        REQUIRE(oob->kind == GDBMI_STREAM);
            struct gdbmi_stream_record *stream = oob->variant.stream_record;
            REQUIRE(stream);
            REQUIRE(stream->kind == GDBMI_CONSOLE);
            REQUIRE(expected == stream->cstring);
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

    REQUIRE(output->oob_record);
        struct gdbmi_oob_record *oob = output->oob_record;
        REQUIRE(oob->kind == GDBMI_STREAM);
            struct gdbmi_stream_record *stream = oob->variant.stream_record;
            REQUIRE(stream);
            REQUIRE(stream->kind == GDBMI_CONSOLE);
            REQUIRE(expected == stream->cstring);
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
    REQUIRE(output->oob_record);
        struct gdbmi_oob_record *oob = output->oob_record;
        REQUIRE(oob->kind == GDBMI_STREAM);
            struct gdbmi_stream_record *stream = oob->variant.stream_record;
            REQUIRE(stream);
            REQUIRE(stream->kind == GDBMI_TARGET);
            REQUIRE(expected == stream->cstring);
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
    REQUIRE(output->oob_record);
        struct gdbmi_oob_record *oob = output->oob_record;
        REQUIRE(oob->kind == GDBMI_STREAM);
            struct gdbmi_stream_record *stream = oob->variant.stream_record;
            REQUIRE(stream);
            REQUIRE(stream->kind == GDBMI_LOG);
            REQUIRE(expected == stream->cstring);
        REQUIRE(!oob->next);
    REQUIRE(!output->result_record);
    REQUIRE(!output->next);
}

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
 * A simple console parse tree.
 */
TEST_CASE_METHOD_N(GdbmiPtTest, console/basic)
{
    REQUIRE(output->oob_record);
        struct gdbmi_oob_record *oob = output->oob_record;
        REQUIRE(oob->kind == GDBMI_STREAM);
            struct gdbmi_stream_record *stream = oob->variant.stream_record;
            REQUIRE(stream);
            REQUIRE(stream->kind == GDBMI_CONSOLE);
            std::string expected =
                "\"GNU gdb (Ubuntu/Linaro) 7.4-2012.04\\n\"";
            REQUIRE(expected == stream->cstring);
        REQUIRE(!oob->next);
    REQUIRE(!output->result_record);
    REQUIRE(!output->next);
}

#include <errno.h>
#include <stdio.h>
#include "catch.hpp"
#include "fixture.h"
#include "gdbwire_mi_pt.h"
#include "gdbwire_mi_parser.h"

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

    struct GdbwireMiParserTest : public Fixture {
        GdbwireMiParserTest() {
            parser = gdbwire_mi_parser_create(parserCallback.callbacks);
            REQUIRE(parser);
        }
        
        ~GdbwireMiParserTest() {
            gdbwire_mi_parser_destroy(parser);
        }

        void test_two_output_item(const char *data) {
            gdbwire_mi_output *output; gdbwire_result result;
            result = gdbwire_mi_parser_push(parser, data);
            REQUIRE(result == GDBWIRE_OK);

            output = parserCallback.m_output;
            REQUIRE(output);
            REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_RESULT);

            output = output->next;
            REQUIRE(output);
            REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_RESULT);

            REQUIRE(!output->next);
        }

        std::string oneHundredPrompts() {
            int i;
            std::string data;
            std::string::iterator it;
            for (i = 0; i < 100; ++i) {
                data += "(gdb)\n";
            }
            return data;
        }

        GdbwireMiParserCallback parserCallback;
        gdbwire_mi_parser *parser;
    };
}

/**
 * Ensure create function fails with null callbacks.
 */
TEST_CASE("GdbwireMiParserTest/create/null_callbacks")
{
    gdbwire_mi_parser *parser;
    struct gdbwire_mi_parser_callbacks callbacks = { 0, 0 };
    parser = gdbwire_mi_parser_create(callbacks);
    REQUIRE(!parser);
}

/**
 * Ensure create returns a non null pointer on success.
 */
TEST_CASE("GdbwireMiParserTest/create/success")
{
    gdbwire_mi_parser *parser;
    struct gdbwire_mi_parser_callbacks callbacks =
        { 0, GdbwireMiParserCallback::gdbwire_mi_output_callback };
    parser = gdbwire_mi_parser_create(callbacks);
    REQUIRE(parser);
    gdbwire_mi_parser_destroy(parser);
}

/**
 * Ensure the callback isn't triggered until a newline is reached.
 */
TEST_CASE_METHOD_N(GdbwireMiParserTest, push/output_waits_for_newline)
{
    gdbwire_result result;

    /* Ensure call back does not occur until newline */
    result = gdbwire_mi_parser_push(parser, "^error");
    REQUIRE(result == GDBWIRE_OK);
    REQUIRE(!parserCallback.m_output);

    result = gdbwire_mi_parser_push(parser, "\n");
    REQUIRE(result == GDBWIRE_OK);
    REQUIRE(parserCallback.m_output);
    REQUIRE(parserCallback.m_output->kind == GDBWIRE_MI_OUTPUT_RESULT);
    REQUIRE(!parserCallback.m_output->next);

    result = gdbwire_mi_parser_push(parser, "(gdb)");
    REQUIRE(result == GDBWIRE_OK);
    REQUIRE(parserCallback.m_output);
    REQUIRE(!parserCallback.m_output->next);

    result = gdbwire_mi_parser_push(parser, "\n");
    REQUIRE(result == GDBWIRE_OK);
    REQUIRE(parserCallback.m_output);
    REQUIRE(parserCallback.m_output->next);
    REQUIRE(parserCallback.m_output->next->kind == GDBWIRE_MI_OUTPUT_PROMPT);
    REQUIRE(!parserCallback.m_output->next->next);
}

/**
 * Ensure the push function fails with a NULL parser parameter.
 */
TEST_CASE_METHOD_N(GdbwireMiParserTest, push/null_parser_parameter)
{
    gdbwire_result result;
    result = gdbwire_mi_parser_push(0, "^error");
    REQUIRE(result == GDBWIRE_ASSERT);
}

/**
 * Ensure parser works when one large package is sent to it.
 *
 * Each line must be handled individually. Internal to the code in the
 * parser, each line must be handled individually. The parser only sets
 * a single gdbwire_mi_output for every string it is provided. If a string
 * containing two newlines was passed to the parser, only the last
 * gdbwire_mi output would be captured.
 */
TEST_CASE_METHOD_N(GdbwireMiParserTest, push/entire_string_at_once)
{
    int i;
    gdbwire_mi_output *output;
    gdbwire_result result;
    std::string data = oneHundredPrompts();
    std::string::iterator it;

    result = gdbwire_mi_parser_push(parser, data.c_str());
    REQUIRE(result == GDBWIRE_OK);

    output = parserCallback.m_output;
    for (i = 0; i < 100; ++i) {
        REQUIRE(output);
        REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_PROMPT);
        output = output->next;
    }

    REQUIRE(!output);
}

/**
 * Ensure parser works with arbitrary sized packets sent to it.
 */
TEST_CASE_METHOD_N(GdbwireMiParserTest, push/packets_at_a_time)
{
    /* I send 5 characters at a time, over a repeating 6 character
     * gdbwire_mi output command. This will ensure that each edge case
     * is hit, due to the overlap that occurs.
     * 
     * (gdb)\n(gdb)\n(gdb)\n(gdb)\n(gdb)\n(gdb)\n
     *     ^     ^     ^     ^     ^    ^     ^
     */
    int i;
    gdbwire_mi_output *output;
    gdbwire_result result;
    std::string data = oneHundredPrompts();
    std::string::size_type count;

    for (count = 0; count < data.size(); count += 5) {
        std::string sub = data.substr(count, 5);
        result = gdbwire_mi_parser_push(parser, sub.c_str());
        REQUIRE(result == GDBWIRE_OK);
    }

    output = parserCallback.m_output;
    for (i = 0; i < 100; ++i) {
        REQUIRE(output);
        REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_PROMPT);
        output = output->next;
    }

    REQUIRE(!output);
}

/**
 * If the push parser is given a single char at a time, it must work.
 */
TEST_CASE_METHOD_N(GdbwireMiParserTest, push/char_at_a_time)
{
    int i;
    gdbwire_mi_output *output;
    gdbwire_result result;
    std::string data = oneHundredPrompts();
    std::string::iterator it;

    for (it = data.begin(); it != data.end(); ++it) {
        result = gdbwire_mi_parser_push_data(parser, &*it, 1);
        REQUIRE(result == GDBWIRE_OK);
    }

    output = parserCallback.m_output;
    for (i = 0; i < 100; ++i) {
        REQUIRE(output);
        REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_PROMPT);
        output = output->next;
    }

    REQUIRE(!output);
}

/**
 * Ensure that \n is supported as a newline.
 */
TEST_CASE_METHOD_N(GdbwireMiParserTest, push/nl_supported_as_newline)
{
    test_two_output_item("^error\n^error\n");
}

/**
 * Ensure that \r is supported as a newline.
 */
TEST_CASE_METHOD_N(GdbwireMiParserTest, push/cr_supported_as_newline)
{
    test_two_output_item("^error\r^error\r");
}

/**
 * Ensure that \r\n is supported as a newline.
 */
TEST_CASE_METHOD_N(GdbwireMiParserTest, push/crnl_supported_as_newline)
{
    test_two_output_item("^error\r\n^error\r\n");
}

/**
 * Ensure that a syntax error is handled successfully.
 *
 * The parser itself shouldn't fail on a syntax error, instead let
 * the gdbwire_mi output command show the error.
 */
TEST_CASE_METHOD_N(GdbwireMiParserTest, push/syntax_error)
{
    gdbwire_mi_output *output;
    gdbwire_result result;
    result = gdbwire_mi_parser_push(parser, "error\n");
    REQUIRE(result == GDBWIRE_OK);
    output = parserCallback.m_output;
    REQUIRE(output);
    REQUIRE(output->kind == GDBWIRE_MI_OUTPUT_PARSE_ERROR);
}

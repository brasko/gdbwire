#include <stdio.h>
#include <list>
#include <string>
#include <iostream>
#include "catch.hpp"
#include "fixture.h"
#include "gdbwire_annotation_pt.h"
#include "gdbwire_annotation_parser.h"

/**
 * The GDB/Annotation parse tree unit tests.
 */

namespace {
    typedef std::list<gdbwire_annotation_output*> gao_list;

    struct GdbwireAnnotationParserCallback {
        GdbwireAnnotationParserCallback() {
            callbacks.context = (void*)this;
            callbacks.gdbwire_annotation_output_callback =
                    GdbwireAnnotationParserCallback::gdbwire_annotation_output_callback;
        }

        ~GdbwireAnnotationParserCallback() {
            gao_list::iterator iter = m_output_list.begin();
            for (; iter != m_output_list.end(); ++iter) {
                gdbwire_annotation_output_free(*iter);
            }
            m_output_list.clear();
        }

        static void gdbwire_annotation_output_callback(void *context,
            gdbwire_annotation_output *output) {
            GdbwireAnnotationParserCallback *callback =
                (GdbwireAnnotationParserCallback *)context;
            callback->gdbwire_annotation_output_callback(output);
        }

        void gdbwire_annotation_output_callback(
                gdbwire_annotation_output *output) {
            m_output_list.push_back(output);
        }

        gdbwire_annotation_parser_callbacks callbacks;
        gao_list m_output_list;
    };

    struct GdbwireAnnotationPtTest : public Fixture {
        GdbwireAnnotationPtTest() {
            parser = gdbwire_annotation_parser_create(parserCallback.callbacks);
            REQUIRE(parser);
            output = parse(parser, sourceTestPath());
            REQUIRE(!output.empty());
        }
        
        ~GdbwireAnnotationPtTest() {
            gdbwire_annotation_parser_destroy(parser);
        }

        /**
         * Parse a GDB/Annotation file and return a gao_list structure.
         *
         * This parser intentionally reads the entire file into memory
         * before sending it to be parsed by the annotation parser.
         * That's due to the fact that if you fed the parser a char at a
         * time, it would create output console events, one char at a time,
         * which is not what we want for testing, and not a realistic
         * scenario for people to run into when using this api.
         *
         * @param parser
         * The gdb annotation parser to do the parsing
         *
         * @param input
         * The input file to parse
         *
         * @return
         * A gdbwire_annotation_event_output structure representing the input file.
         * You are responsible for destroying this memory.
         */
        gao_list parse(gdbwire_annotation_parser *parser,
            const std::string &input) {
            FILE *fd;
            int c;
            std::string file_contents;

            fd = fopen(input.c_str(), "r");
            REQUIRE(fd);

            while ((c = fgetc(fd)) != EOF) {
                char ch = c;
                file_contents.push_back(ch);
            }
            fclose(fd);

            REQUIRE(gdbwire_annotation_parser_push_data(parser,
                file_contents.data(), file_contents.size()) == GDBWIRE_OK);

            return parserCallback.m_output_list;
        }

        GdbwireAnnotationParserCallback parserCallback;
        gdbwire_annotation_parser *parser;
        gao_list output;
    };
}

/**
 * A simple console output parse tree.
 */
TEST_CASE_METHOD_N(GdbwireAnnotationPtTest, console_output/basic.txt)
{
    std::string expected = "Hello World console output";
    gdbwire_annotation_output *o = output.front();
    REQUIRE(output.size() == 1);
    REQUIRE(o->kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT);
    REQUIRE(o->variant.console_output);
    REQUIRE(expected == o->variant.console_output);
}

/**
 * All possible characters in the console output stream.
 *
 * Please see the test,
 *   oob_record/stream/console/characters.mi
 * in gdbwire_mi_pt.cpp for additional comments.
 */
TEST_CASE_METHOD_N(GdbwireAnnotationPtTest, console_output/characters.txt)
{
    std::string expected =
        "$1 = "
        "\"\\000\\001\\002\\003\\004\\005\\006\\a"
        "\\b\\t\\n\\v\\f\\r\\016\\017"
        "\\020\\021\\022\\023\\024\\025\\026\\027"
        "\\030\\031\\032\\033\\034\\035\\036\\037"
        " !\\\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "[\\\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\\177"
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
    gdbwire_annotation_output *o = output.front();
    REQUIRE(output.size() == 1);
    REQUIRE(o->kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT);
    REQUIRE(o->variant.console_output);
    REQUIRE(expected == o->variant.console_output);
}

/// Test the breakpoints-invalid annotations
TEST_CASE_METHOD_N(GdbwireAnnotationPtTest, annotation/breakpoints-invalid.txt)
{
    gao_list::const_iterator o;

    REQUIRE(output.size() == 1);

    o = output.begin();
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind ==
        GDBWIRE_ANNOTATION_BREAKPOINTS_INVALID);
    REQUIRE(std::string("breakpoints-invalid") ==
            (*o)->variant.annotation.text);

    ++o;
    REQUIRE(o == output.end());
}

/// Test the source annotations
TEST_CASE_METHOD_N(GdbwireAnnotationPtTest, annotation/source.txt)
{
    gao_list::const_iterator o;
    std::string expected = "source /home/foo/o.cpp:7:129:beg:0x4004f6";

    REQUIRE(output.size() == 1);

    o = output.begin();
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind == GDBWIRE_ANNOTATION_SOURCE);
    REQUIRE(expected == (*o)->variant.annotation.text);

    ++o;
    REQUIRE(o == output.end());
}

/// Test the frame end annotations
TEST_CASE_METHOD_N(GdbwireAnnotationPtTest, annotation/frame-end.txt)
{
    gao_list::const_iterator o;
    std::string expected = "frame-end";

    REQUIRE(output.size() == 1);

    o = output.begin();
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind == GDBWIRE_ANNOTATION_FRAME_END);
    REQUIRE(expected == (*o)->variant.annotation.text);

    ++o;
    REQUIRE(o == output.end());
}

/// Test the frames invalid annotations
TEST_CASE_METHOD_N(GdbwireAnnotationPtTest, annotation/frames-invalid.txt)
{
    gao_list::const_iterator o;
    std::string expected = "frames-invalid";

    REQUIRE(output.size() == 1);

    o = output.begin();
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind == GDBWIRE_ANNOTATION_FRAMES_INVALID);
    REQUIRE(expected == (*o)->variant.annotation.text);

    ++o;
    REQUIRE(o == output.end());
}

/// Test the commands annotations
TEST_CASE_METHOD_N(GdbwireAnnotationPtTest, annotation/commands.txt)
{
    gao_list::const_iterator o;

    REQUIRE(output.size() == 10);

    o = output.begin();
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind == GDBWIRE_ANNOTATION_PRE_COMMANDS);
    REQUIRE(std::string("pre-commands") == (*o)->variant.annotation.text);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT);
    REQUIRE(std::string(">") == (*o)->variant.console_output);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind == GDBWIRE_ANNOTATION_COMMANDS);
    REQUIRE(std::string("commands") == (*o)->variant.annotation.text);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT);
    REQUIRE(std::string("print argc\n") == (*o)->variant.console_output);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind == GDBWIRE_ANNOTATION_POST_COMMANDS);
    REQUIRE(std::string("post-commands") == (*o)->variant.annotation.text);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind == GDBWIRE_ANNOTATION_PRE_COMMANDS);
    REQUIRE(std::string("pre-commands") == (*o)->variant.annotation.text);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT);
    REQUIRE(std::string(">") == (*o)->variant.console_output);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind == GDBWIRE_ANNOTATION_COMMANDS);
    REQUIRE(std::string("commands") == (*o)->variant.annotation.text);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT);
    REQUIRE(std::string("end\n") == (*o)->variant.console_output);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind == GDBWIRE_ANNOTATION_POST_COMMANDS);
    REQUIRE(std::string("post-commands") == (*o)->variant.annotation.text);

    ++o;
    REQUIRE(o == output.end());
}

/**
 * Test the overload-choice annotations
 *
 * To reproduce this annotations file, I created a source file as follows,
 *   int identity(int p) { return p; }
 *   float identity(float p) { return p; }
 *   int main() {
 *     identity(1);
 *     identity(1.f);
 *     return 0;
 *   }
 * Then started gdb as follows,
 *   gdb --annotate=2 ./main
 * and typed the following command,
 *   b identity
 */
TEST_CASE_METHOD_N(GdbwireAnnotationPtTest, annotation/overload-choice.txt)
{
    gao_list::const_iterator o;

    REQUIRE(output.size() == 9);

    // the standard beginning prompt
    o = output.begin();

    // the choices are printed as console output
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT);
    REQUIRE(std::string("[0] cancel\n") == (*o)->variant.console_output);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT);
    REQUIRE(std::string("[1] all\n") == (*o)->variant.console_output);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT);
    REQUIRE(std::string("[2] o.cpp:identity(float)\n") ==
        (*o)->variant.console_output);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT);
    REQUIRE(std::string("[3] o.cpp:identity(int)\n") ==
        (*o)->variant.console_output);

    // the overload-choice prompt
    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind ==
        GDBWIRE_ANNOTATION_PRE_OVERLOAD_CHOICE);
    REQUIRE(std::string("pre-overload-choice") ==
        (*o)->variant.annotation.text);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT);
    REQUIRE(std::string("> ") == (*o)->variant.console_output);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind ==
        GDBWIRE_ANNOTATION_OVERLOAD_CHOICE);
    REQUIRE(std::string("overload-choice") == (*o)->variant.annotation.text);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT);
    REQUIRE(std::string("2\n") == (*o)->variant.console_output);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind ==
        GDBWIRE_ANNOTATION_POST_OVERLOAD_CHOICE);
    REQUIRE(std::string("post-overload-choice") ==
       (*o)->variant.annotation.text);

    ++o;
    REQUIRE(o == output.end());
}

/**
 * Test the instance-choice annotations
 *
 * This is a GNAT extension for choosing a specific instance of
 * a generic I believe. I have no actual documentation or examples of
 * what this looks like, so I've fabricated a test case to the best of my
 * ability based on the overload-choice annotation.
 */
TEST_CASE_METHOD_N(GdbwireAnnotationPtTest, annotation/instance-choice.txt)
{
    gao_list::const_iterator o;

    REQUIRE(output.size() == 9);

    // the standard beginning prompt
    o = output.begin();

    // the choices are printed as console output
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT);
    REQUIRE(std::string("[0] cancel\n") == (*o)->variant.console_output);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT);
    REQUIRE(std::string("[1] all\n") == (*o)->variant.console_output);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT);
    REQUIRE(std::string("[2] o.adb:identity(float)\n") ==
        (*o)->variant.console_output);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT);
    REQUIRE(std::string("[3] o.adb:identity(int)\n") ==
        (*o)->variant.console_output);

    // the instance-choice prompt
    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind ==
        GDBWIRE_ANNOTATION_PRE_INSTANCE_CHOICE);
    REQUIRE(std::string("pre-instance-choice") ==
        (*o)->variant.annotation.text);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT);
    REQUIRE(std::string("> ") == (*o)->variant.console_output);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind ==
        GDBWIRE_ANNOTATION_INSTANCE_CHOICE);
    REQUIRE(std::string("instance-choice") == (*o)->variant.annotation.text);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT);
    REQUIRE(std::string("2\n") == (*o)->variant.console_output);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind ==
        GDBWIRE_ANNOTATION_POST_INSTANCE_CHOICE);
    REQUIRE(std::string("post-instance-choice") ==
       (*o)->variant.annotation.text);

    ++o;
    REQUIRE(o == output.end());
}

/**
 * Test the query annotations
 */
TEST_CASE_METHOD_N(GdbwireAnnotationPtTest, annotation/query.txt)
{
    gao_list::const_iterator o;

    REQUIRE(output.size() == 9);

    // the standard beginning prompt
    o = output.begin();

    // the query-choice prompt
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind ==
        GDBWIRE_ANNOTATION_PRE_QUERY);
    REQUIRE(std::string("pre-query") == (*o)->variant.annotation.text);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT);
    REQUIRE(std::string("A debugging session is active.\n") ==
            (*o)->variant.console_output);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT);
    REQUIRE(std::string("\n") ==
            (*o)->variant.console_output);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT);
    REQUIRE(std::string("    Inferior 1 [process 15150] will be killed.\n") ==
            (*o)->variant.console_output);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT);
    REQUIRE(std::string("\n") ==
            (*o)->variant.console_output);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT);
    REQUIRE(std::string("Quit anyway? (y or n) ") ==
            (*o)->variant.console_output);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind ==
        GDBWIRE_ANNOTATION_QUERY);
    REQUIRE(std::string("query") == (*o)->variant.annotation.text);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT);
    REQUIRE(std::string("y\n") == (*o)->variant.console_output);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind ==
        GDBWIRE_ANNOTATION_POST_QUERY);
    REQUIRE(std::string("post-query") ==
       (*o)->variant.annotation.text);

    ++o;
    REQUIRE(o == output.end());
}

/**
 * Test the prompt annotations
 */
TEST_CASE_METHOD_N(GdbwireAnnotationPtTest, annotation/prompt.txt)
{
    gao_list::const_iterator o;

    REQUIRE(output.size() == 5);

    // the standard beginning prompt
    o = output.begin();
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind == GDBWIRE_ANNOTATION_PRE_PROMPT);
    REQUIRE(std::string("pre-prompt") == (*o)->variant.annotation.text);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT);
    REQUIRE(std::string("(gdb)") == (*o)->variant.console_output);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind == GDBWIRE_ANNOTATION_PROMPT);
    REQUIRE(std::string("prompt") == (*o)->variant.annotation.text);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT);
    REQUIRE(std::string("b identity\n") == (*o)->variant.console_output);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind == GDBWIRE_ANNOTATION_POST_PROMPT);
    REQUIRE(std::string("post-prompt") == (*o)->variant.annotation.text);

    ++o;
    REQUIRE(o == output.end());
}

/**
 * Test the error begin annotations
 * 
 * To get this output, I started gdb with --annotate=2, typed ctrl-c and
 * then ctrl-d and redirected that output to a file.
 *
 * I then filtered that file by deleting everything before the first prompt
 * annotation and after the last pre-prompt annotation.
 */
TEST_CASE_METHOD_N(GdbwireAnnotationPtTest, annotation/error-begin.txt)
{
    gao_list::const_iterator o;

    REQUIRE(output.size() == 4);

    // the standard beginning prompt
    o = output.begin();
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind == GDBWIRE_ANNOTATION_ERROR_BEGIN);
    REQUIRE(std::string("error-begin") == (*o)->variant.annotation.text);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT);
    REQUIRE(std::string("./fake: No such file or directory.\n") == 
            (*o)->variant.console_output);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind == GDBWIRE_ANNOTATION_ERROR);
    REQUIRE(std::string("error") == (*o)->variant.annotation.text);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind == GDBWIRE_ANNOTATION_PRE_PROMPT);
    REQUIRE(std::string("pre-prompt") == (*o)->variant.annotation.text);

    ++o;
    REQUIRE(o == output.end());
}

/**
 * Test the error annotations
 * 
 * To get this output, I started gdb with --annotate=2 and set a breakpoint
 * at a file that does not exist.
 */
TEST_CASE_METHOD_N(GdbwireAnnotationPtTest, annotation/error.txt)
{
    gao_list::const_iterator o;

    REQUIRE(output.size() == 5);

    // the standard beginning prompt
    o = output.begin();
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind == GDBWIRE_ANNOTATION_POST_PROMPT);
    REQUIRE(std::string("post-prompt") == (*o)->variant.annotation.text);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind == GDBWIRE_ANNOTATION_ERROR_BEGIN);
    REQUIRE(std::string("error-begin") == (*o)->variant.annotation.text);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT);
    REQUIRE(std::string("No source file named file1.c.\n") ==
            (*o)->variant.console_output);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind == GDBWIRE_ANNOTATION_ERROR);
    REQUIRE(std::string("error") == (*o)->variant.annotation.text);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind == GDBWIRE_ANNOTATION_PRE_QUERY);
    REQUIRE(std::string("pre-query") == (*o)->variant.annotation.text);

    ++o;
    REQUIRE(o == output.end());
}

/**
 * Test the quit annotations
 * 
 * This is currently the same test as error-begin. However, this test
 * represents the "Quit" annotation, and could change as necessary.
 */
TEST_CASE_METHOD_N(GdbwireAnnotationPtTest, annotation/quit.txt)
{
    gao_list::const_iterator o;

    REQUIRE(output.size() == 5);

    // the standard beginning prompt
    o = output.begin();
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind == GDBWIRE_ANNOTATION_PROMPT);
    REQUIRE(std::string("prompt") == (*o)->variant.annotation.text);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind == GDBWIRE_ANNOTATION_ERROR_BEGIN);
    REQUIRE(std::string("error-begin") == (*o)->variant.annotation.text);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_CONSOLE_OUTPUT);
    REQUIRE(std::string("Quit\n") == (*o)->variant.console_output);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind == GDBWIRE_ANNOTATION_QUIT);
    REQUIRE(std::string("quit") == (*o)->variant.annotation.text);

    ++o;
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind == GDBWIRE_ANNOTATION_PRE_PROMPT);
    REQUIRE(std::string("pre-prompt") == (*o)->variant.annotation.text);

    ++o;
    REQUIRE(o == output.end());
}

/**
 * Test the exited annotations
 */
TEST_CASE_METHOD_N(GdbwireAnnotationPtTest, annotation/exited.txt)
{
    gao_list::const_iterator o;

    REQUIRE(output.size() == 1);

    // the standard beginning prompt
    o = output.begin();
    REQUIRE((*o)->kind == GDBWIRE_ANNOTATION_OUTPUT_ANNOTATION);
    REQUIRE((*o)->variant.annotation.kind == GDBWIRE_ANNOTATION_EXITED);
    REQUIRE(std::string("exited 0") == (*o)->variant.annotation.text);

    ++o;
    REQUIRE(o == output.end());
}

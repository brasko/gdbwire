#include <stdio.h>
#include <string.h>
#include "catch.hpp"
#include "fixture.h"
#include "gdbwire.h"

/**
 * These tests test that the callbacks work.
 *
 * They do not verify the data in the callbacks. That testing is
 * done elsewhere.
 */

namespace {
    struct GdbwireCallbacks {
        GdbwireCallbacks() {
            const static gdbwire_callbacks init_callbacks = {
                (void*)this,
                GdbwireCallbacks::gdbwire_stream_record,
                GdbwireCallbacks::gdbwire_async_record,
                GdbwireCallbacks::gdbwire_result_record,
                GdbwireCallbacks::gdbwire_prompt,
                GdbwireCallbacks::gdbwire_parse_error
            };

            callbacks = init_callbacks;
            streamRecordKind = (gdbwire_mi_stream_record_kind)-1;
            asyncRecordKind = (gdbwire_mi_async_record_kind)-1;
            asyncClass = GDBWIRE_MI_ASYNC_UNSUPPORTED;
            resultClass = GDBWIRE_MI_UNSUPPORTED;
        }

        static void gdbwire_stream_record(void *context,
                gdbwire_mi_stream_record *stream_record) {
            GdbwireCallbacks *callback = (GdbwireCallbacks *)context;
            callback->gdbwire_mi_stream_record(stream_record);
        }

        void gdbwire_mi_stream_record(gdbwire_mi_stream_record *stream_record) {
            REQUIRE(stream_record);
            streamRecordKind = stream_record->kind;
            streamString = stream_record->cstring;

        }

        static void gdbwire_async_record(void *context,
                gdbwire_mi_async_record *async_record) {
            GdbwireCallbacks *callback = (GdbwireCallbacks *)context;
            callback->gdbwire_mi_async_record(async_record);
        }

        void gdbwire_mi_async_record(gdbwire_mi_async_record *async_record) {
            REQUIRE(async_record);
            asyncRecordKind = async_record->kind;
            asyncClass = async_record->async_class;
        }

        static void gdbwire_result_record(void *context,
                gdbwire_mi_result_record *result_record) {
            GdbwireCallbacks *callback = (GdbwireCallbacks *)context;
            callback->gdbwire_result_record(result_record);
        }

        void gdbwire_result_record(gdbwire_mi_result_record *result_record) {
            REQUIRE(result_record);
            resultClass = result_record->result_class;
        }

        static void gdbwire_prompt(void *context, const char *prompt) {
            GdbwireCallbacks *callback = (GdbwireCallbacks *)context;
            callback->gdbwire_prompt(prompt);
        }

        void gdbwire_prompt(const char *prompt) {
            REQUIRE(prompt);
            promptString = prompt;
        }

        static void gdbwire_parse_error(void *context,
                const char *mi, const char *token,
                gdbwire_mi_position position) {
            GdbwireCallbacks *callback = (GdbwireCallbacks *)context;
            callback->gdbwire_parse_error(mi, token, position);
        }

        void gdbwire_parse_error(const char *mi, const char *token,
                gdbwire_mi_position position) {
            REQUIRE(mi);
            REQUIRE(token);
            parseErrorToken = token;
        }

        gdbwire_callbacks callbacks;

        // Used for the stream callback
        gdbwire_mi_stream_record_kind streamRecordKind;
        std::string streamString;

        // Used for the async callback
        gdbwire_mi_async_record_kind asyncRecordKind;
        gdbwire_mi_async_class asyncClass;

        // Used for result callback
        gdbwire_mi_result_class resultClass;
        
        // Used for prompt callback
        std::string promptString;

        // Used for parse error
        std::string parseErrorToken;
    };

    struct GdbwireTest: public Fixture {
        public:
            GdbwireTest() {
                wire = gdbwire_create(wireCallbacks.callbacks);
                REQUIRE(wire);
                parse(wire, sourceTestPath());
            }

            ~GdbwireTest() {
                gdbwire_destroy(wire);
            }

            /**
             * Read a GDB/MI file and push the characters to gdbwire.
             *
             * @param parser
             * The gdb mi parser to do the parsing
             *
             * @param input
             * The input file to parse
             */
            void parse(gdbwire *wire, const std::string &input) {
                FILE *fd;
                int c;

                fd = fopen(input.c_str(), "r");
                REQUIRE(fd);

                while ((c = fgetc(fd)) != EOF) {
                    char ch = c;
                    REQUIRE(gdbwire_push_data(wire, &ch, 1) == GDBWIRE_OK);
                }
                fclose(fd);
            }

            GdbwireCallbacks wireCallbacks;
            gdbwire *wire;
    };

    struct GdbwireBasicTest: public Fixture {};

    std::string get_file_contents(const std::string &path) {
        std::string result;
        FILE *fd;
        int c;

        fd = fopen(path.c_str(), "r");
        REQUIRE(fd);

        while ((c = fgetc(fd)) != EOF) {
            result.push_back((char)c);
        }
        fclose(fd);

        return result;
    }
}

TEST_CASE_METHOD_N(GdbwireBasicTest, create/normal)
{
    gdbwire_callbacks c = {};
    struct gdbwire *wire = gdbwire_create(c);
    REQUIRE(wire);
    gdbwire_destroy(wire);
}

TEST_CASE_METHOD_N(GdbwireBasicTest, destroy/normal)
{
    gdbwire_callbacks c = {};
    struct gdbwire *wire = gdbwire_create(c);
    REQUIRE(wire);
    gdbwire_destroy(wire);
}

TEST_CASE_METHOD_N(GdbwireBasicTest, destroy/null)
{
    gdbwire_destroy(NULL);
}

TEST_CASE_METHOD_N(GdbwireTest, callbacks/stream_record/console.mi)
{
    std::string expected = "Hello World console output";

    REQUIRE(wireCallbacks.streamRecordKind == GDBWIRE_MI_CONSOLE);
    REQUIRE(wireCallbacks.streamString == expected);
}

TEST_CASE_METHOD_N(GdbwireTest, callbacks/async_record/exec.mi)
{
    REQUIRE(wireCallbacks.asyncRecordKind == GDBWIRE_MI_EXEC);
    REQUIRE(wireCallbacks.asyncClass == GDBWIRE_MI_ASYNC_RUNNING);
}

TEST_CASE_METHOD_N(GdbwireTest, callbacks/result_record/done.mi)
{
    REQUIRE(wireCallbacks.resultClass == GDBWIRE_MI_DONE);
}

TEST_CASE_METHOD_N(GdbwireTest, callbacks/prompt/basic.mi)
{
    std::string expected = "(gdb) \n";
    REQUIRE(wireCallbacks.promptString == expected);
}

TEST_CASE_METHOD_N(GdbwireTest, callbacks/parse_error/basic.mi)
{
    std::string expected = "$";
    REQUIRE(wireCallbacks.parseErrorToken == expected);
}

TEST_CASE_METHOD_N(GdbwireBasicTest, interpreter_exec/basic.mi)
{
    std::string mi = get_file_contents(sourceTestPath());
    enum gdbwire_result result;
    struct gdbwire_mi_command *mi_command = 0;

    result = gdbwire_interpreter_exec(mi.c_str(),
        GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE, &mi_command);
    REQUIRE(result == GDBWIRE_OK);
    REQUIRE(mi_command);
    gdbwire_mi_command_free(mi_command);
}

TEST_CASE_METHOD_N(GdbwireBasicTest, interpreter_exec/error.mi)
{
    std::string mi = get_file_contents(sourceTestPath());
    enum gdbwire_result result;
    struct gdbwire_mi_command *mi_command = 0;

    result = gdbwire_interpreter_exec(mi.c_str(),
        GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE, &mi_command);
    REQUIRE(result == GDBWIRE_ASSERT);
    REQUIRE(!mi_command);
}

TEST_CASE_METHOD_N(GdbwireBasicTest, interpreter_exec/empty_error)
{
    std::string mi = "";
    enum gdbwire_result result;
    struct gdbwire_mi_command *mi_command = 0;

    result = gdbwire_interpreter_exec(mi.c_str(),
        GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE, &mi_command);
    REQUIRE(result == GDBWIRE_LOGIC);
    REQUIRE(!mi_command);
}

TEST_CASE_METHOD_N(GdbwireBasicTest, interpreter_exec/command_and_stream.mi)
{
    std::string mi = get_file_contents(sourceTestPath());
    enum gdbwire_result result;
    struct gdbwire_mi_command *mi_command = 0;

    result = gdbwire_interpreter_exec(mi.c_str(),
        GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE, &mi_command);
    REQUIRE(result == GDBWIRE_LOGIC);
    REQUIRE(!mi_command);
}

TEST_CASE_METHOD_N(GdbwireBasicTest, interpreter_exec/command_and_prompt.mi)
{
    std::string mi = get_file_contents(sourceTestPath());
    enum gdbwire_result result;
    struct gdbwire_mi_command *mi_command = 0;

    result = gdbwire_interpreter_exec(mi.c_str(),
        GDBWIRE_MI_FILE_LIST_EXEC_SOURCE_FILE, &mi_command);
    REQUIRE(result == GDBWIRE_LOGIC);
    REQUIRE(!mi_command);
}

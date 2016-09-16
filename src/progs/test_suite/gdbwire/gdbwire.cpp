#include <stdio.h>
#include <string.h>
#include "catch.hpp"
#include "fixture.h"
#include "gdbwire/gdbwire.h"

namespace {
    struct GdbwireCallbacks {
        GdbwireCallbacks() {
            const static gdbwire_callbacks init_callbacks = {
                (void*)this,
                GdbwireCallbacks::gdbwire_console,
                GdbwireCallbacks::gdbwire_target,
                GdbwireCallbacks::gdbwire_log,
                GdbwireCallbacks::gdbwire_prompt
            };

            callbacks = init_callbacks;
        }

        static void gdbwire_console(void *context, const char *str) {
            GdbwireCallbacks *callback = (GdbwireCallbacks *)context;
            callback->gdbmi_console(str);
        }

        void gdbmi_console(const char *str) {
            REQUIRE(str);
            consoleStr = str;
        }

        static void gdbwire_target(void *context, const char *str) {
            GdbwireCallbacks *callback = (GdbwireCallbacks *)context;
            callback->gdbmi_target(str);
        }

        void gdbmi_target(const char *str) {
            REQUIRE(str);
            targetStr = str;
        }

        static void gdbwire_log(void *context, const char *str) {
            GdbwireCallbacks *callback = (GdbwireCallbacks *)context;
            callback->gdbmi_log(str);
        }

        void gdbmi_log(const char *str) {
            REQUIRE(str);
            logStr = str;
        }

        static void gdbwire_prompt(void *context, const char *str) {
            GdbwireCallbacks *callback = (GdbwireCallbacks *)context;
            callback->gdbmi_prompt(str);
        }

        void gdbmi_prompt(const char *str) {
            REQUIRE(str);
            promptStr = str;
        }

        gdbwire_callbacks callbacks;
        std::string consoleStr, targetStr, logStr, promptStr;
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

    struct GdbwireTestBasic: public Fixture {};
}

TEST_CASE_METHOD_N(GdbwireTestBasic, create/normal)
{
    gdbwire_callbacks c = {};
    struct gdbwire *wire = gdbwire_create(c);
    REQUIRE(wire);
    gdbwire_destroy(wire);
}

TEST_CASE_METHOD_N(GdbwireTestBasic, destroy/normal)
{
    gdbwire_callbacks c = {};
    struct gdbwire *wire = gdbwire_create(c);
    REQUIRE(wire);
    gdbwire_destroy(wire);
}

TEST_CASE_METHOD_N(GdbwireTestBasic, destroy/null)
{
    gdbwire_destroy(NULL);
}

TEST_CASE_METHOD_N(GdbwireTest, callbacks/console/basic.mi)
{
    std::string expected = "Hello World console output";
    REQUIRE(wireCallbacks.consoleStr == expected);
}

TEST_CASE_METHOD_N(GdbwireTest, callbacks/target/basic.mi)
{
    std::string expected = "Hello World target output";
    REQUIRE(wireCallbacks.targetStr == expected);
}

TEST_CASE_METHOD_N(GdbwireTest, callbacks/log/basic.mi)
{
    std::string expected = "Hello World log output";
    REQUIRE(wireCallbacks.logStr == expected);
}

TEST_CASE_METHOD_N(GdbwireTest, callbacks/prompt/basic.mi)
{
    std::string expected = "(gdb) \n";
    REQUIRE(wireCallbacks.promptStr == expected);
}

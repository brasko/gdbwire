#include <errno.h>
#include "catch.hpp"
#include "fixture.h"
#include "gdbmi/gdbmi_pt.h"
#include "gdbmi/gdbmi_parser.h"

namespace {
    struct GdbmiTest : public Fixture {
        GdbmiTest() {
            parser = gdbmi_parser_create();
            REQUIRE(parser);
        }
        
        ~GdbmiTest() {
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
            gdbmi_output *output = 0, *pt = 0;
            FILE *fd;
            int c;

            fd = fopen(input.c_str(), "r");
            REQUIRE(fd);

            while ((c = fgetc(fd)) != EOF) {
                char data[2] = { c, 0 };
                pt = 0;
                REQUIRE(gdbmi_parser_push(parser, data, &pt) == 0);
                if (pt) {
                    output = append_gdbmi_output(output, pt);
                }
            }
            fclose(fd);

            return output;
        }

        gdbmi_parser *parser;
   };
}

TEST_F(GdbmiTest, basic)
{
    gdbmi_output *output = 0;
    int result;
    std::string input = sourceTestDir() + "/input.mi";

    output = parse(parser, input);
    REQUIRE(output);

    //REQUIRE(print_gdbmi_output(output) == 0);

    result = destroy_gdbmi_output(output);
    REQUIRE(result != -1);
}

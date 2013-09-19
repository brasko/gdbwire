#include "catch.hpp"
#include "fixture.h"
#include "gdbmi/gdbmi_pt.h"
#include "gdbmi/gdbmi_parser.h"

namespace {
    class GdbmiTest : public Fixture {
    };
}

TEST_F(GdbmiTest, basic)
{
    gdbmi_parser *parser;
    gdbmi_output *output;
    int result, parse_failed;
    std::string input = sourceTestDir() + "/input.mi";

    parser = gdbmi_parser_create();
    REQUIRE(parser);

    result = gdbmi_parser_parse_file(parser, input.c_str(), &output,
            &parse_failed);

    REQUIRE(result != -1);
    REQUIRE(parse_failed == 0);

    //ASSERT_EQ(0, print_gdbmi_output(output));

    result = destroy_gdbmi_output(output);
    REQUIRE(result != -1);

    gdbmi_parser_destroy(parser);
}

#include <gtest/gtest.h>

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
    ASSERT_TRUE(parser);

    result = gdbmi_parser_parse_file(parser, input.c_str(), &output,
            &parse_failed);

    ASSERT_NE(result, -1);
    ASSERT_EQ(parse_failed, 0);

    result = destroy_gdbmi_output(output);
    ASSERT_NE(result, -1);

    gdbmi_parser_destroy(parser);
}

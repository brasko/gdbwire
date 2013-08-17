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
    gdbmi_parser *parser_ptr;
    gdbmi_output *output_ptr;
    int result, parse_failed;
    std::string input = sourceTestDir() + "/input.mi";

    parser_ptr = gdbmi_parser_create();
    ASSERT_TRUE(parser_ptr);

    result = gdbmi_parser_parse_file(parser_ptr, input.c_str(), &output_ptr,
            &parse_failed);

    ASSERT_NE(result, -1);
    ASSERT_EQ(parse_failed, 0);

    result = destroy_gdbmi_output(output_ptr);
    ASSERT_NE(result, -1);

    gdbmi_parser_destroy(parser_ptr);
}

#include <string.h>
#include "catch.hpp"
#include "fixture.h"
#include "gdbwire/gdbwire.h"

namespace {
    struct GdbwireTest : public Fixture {};
}

TEST_CASE_METHOD_N(GdbwireTest, create)
{
    struct gdbwire *wire = gdbwire_create();
    REQUIRE(wire);
    gdbwire_destroy(wire);
}

TEST_CASE_METHOD_N(GdbwireTest, destroy/normal)
{
    struct gdbwire *wire = gdbwire_create();
    gdbwire_destroy(wire);
}

TEST_CASE_METHOD_N(GdbwireTest, destroy/null)
{
    gdbwire_destroy(NULL);
}

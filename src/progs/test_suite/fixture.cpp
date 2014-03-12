#include "config.h"
#include "catch.hpp"
#include "fixture.h"

std::string
Fixture::testName()
{
    return Catch::getCurrentContext().getResultCapture().getCurrentTestName();
}

std::string
Fixture::src()
{
    std::string srcdir = GDBWIRE_ABS_TOP_SRCDIR;
    srcdir += "/src";
    srcdir += "/progs";
    srcdir += "/test_suite";
    return srcdir;
}

std::string
Fixture::data()
{
    std::string datadir = src();
    datadir += "/data";
    return datadir;
}

std::string
Fixture::sourceTestDir()
{
    std::string datadir = data();
    datadir = datadir + "/" + testName();
    return datadir;
}

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include "logging/gdbwire_logger.h"

int
main(int argc, char *argv[])
{
    int result = gdbwire_logger_open("log.txt");

    if (result == 0) {
        gdbwire_debug("Starting Test Suite");

        result = Catch::Session().run(argc, argv);

        gdbwire_logger_close();
    }

    return result;
}

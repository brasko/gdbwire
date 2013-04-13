ACLOCAL_AMFLAGS = -I build/m4

bin_PROGRAMS =
if WANT_TESTS
    bin_PROGRAMS += test_suite
endif

lib_LTLIBRARIES=libgdbwire.la

# The gdbwire configuration
libgdbwire_la_SOURCES= \
    src/lib/foo.c

# The test suite configuration
test_suite_SOURCES = \
    src/progs/test_suite/main.cpp

ACLOCAL_AMFLAGS = -I build/m4

noinst_PROGRAMS =
if WANT_TESTS
    noinst_PROGRAMS += test_suite
endif

lib_LTLIBRARIES=libgdbwire.la

# The gdbwire configuration
libgdbwire_la_SOURCES= \
    src/lib/foo.c

# The test suite configuration
test_suite_SOURCES = \
    src/progs/test_suite/main.cpp

test_suite_CPPFLAGS = \
    @GTEST_CPPFLAGS@

test_suite_LDFLAGS = \
    @GTEST_LDFLAGS@

test_suite_LDADD = \
    @GTEST_LIBS@

ACLOCAL_AMFLAGS = -I build/m4

noinst_PROGRAMS =
if WANT_TESTS
    noinst_PROGRAMS += test_suite
endif
noinst_PROGRAMS += gdbmi_driver

lib_LTLIBRARIES=libgdbwire.la

# The gdbwire configuration
libgdbwire_la_SOURCES= \
    src/lib/foo.c \
    src/lib/gdbmi/gdbmi_grammar.y \
    src/lib/gdbmi/gdbmi_lexer.l \
    src/lib/gdbmi/gdbmi_oc.c \
    src/lib/gdbmi/gdbmi_parser.c \
    src/lib/gdbmi/gdbmi_pt.c

# The test suite configuration
test_suite_SOURCES = \
    src/progs/test_suite/fixture.cpp \
    src/progs/test_suite/main.cpp

test_suite_CPPFLAGS = \
    @GTEST_CPPFLAGS@ \
    @BOOST_CPPFLAGS@

test_suite_LDFLAGS = \
    @GTEST_LDFLAGS@ \
    @BOOST_LDFLAGS@

test_suite_LDADD = \
    libgdbwire.la \
    @GTEST_LIBS@ \
    @BOOST_LIBS@

# The gdbmi_driver configuration
gdbmi_driver_SOURCES = \
    src/progs/gdbmi/driver.c

gdbmi_driver_CPPFLAGS = \
    -I$(abs_top_srcdir)/src/lib

gdbmi_driver_LDFLAGS = \
    -L$(abs_top_builddir)/src/lib

gdbmi_driver_LDADD = libgdbwire.la

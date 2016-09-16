ACLOCAL_AMFLAGS = -I build/m4

noinst_PROGRAMS =
if WANT_TESTS
    noinst_PROGRAMS += test_suite
endif

if WANT_EXAMPLES
    noinst_PROGRAMS += examples/gdbmi
    noinst_PROGRAMS += examples/gdbwire
endif

lib_LTLIBRARIES=libgdbwire.la

# The gdbwire configuration
libgdbwire_la_SOURCES= \
    src/gdbmi_grammar.h \
    src/gdbmi_grammar.y \
    src/gdbmi_lexer.l \
    src/gdbmi_parser.h \
    src/gdbmi_parser.c \
    src/gdbmi_pt.h \
    src/gdbmi_pt.c \
    src/gdbmi_pt_alloc.h \
    src/gdbmi_pt_alloc.c \
    src/gdbwire.h \
    src/gdbwire.c \
    src/gdbwire_assert.h \
    src/gdbwire_logger.h \
    src/gdbwire_logger.c \
    src/gdbwire_result.h \
    src/gdbwire_string.h \
    src/gdbwire_string.c

libgdbwire_la_CFLAGS= \
	-I@GDBWIRE_ABS_TOP_SRCDIR@/src \
	-I@GDBWIRE_ABS_TOP_BUILDDIR@/src 

# The test suite configuration
nodist_test_suite_SOURCES = \
    src/progs/test_suite/gdbwire_string.cpp \
    src/progs/test_suite/fixture.cpp \
    src/progs/test_suite/gdbmi_parser.cpp \
    src/progs/test_suite/gdbmi_pt.cpp \
    src/progs/test_suite/gdbwire.cpp \
    src/progs/test_suite/main.cpp
test_suite_CPPFLAGS = \
    -I@GDBWIRE_ABS_TOP_SRCDIR@/src/progs/test_suite \
    -I@GDBWIRE_ABS_TOP_SRCDIR@/src
test_suite_LDFLAGS =
test_suite_LDADD = libgdbwire.la

# The gdbmi example configuration
examples_gdbmi_SOURCES = src/progs/examples/gdbmi_example.c
examples_gdbmi_CPPFLAGS = -I@GDBWIRE_ABS_TOP_SRCDIR@/src
examples_gdbmi_LDFLAGS =
examples_gdbmi_LDADD = libgdbwire.la

# The gdbwire example configuration
examples_gdbwire_SOURCES = src/progs/examples/gdbwire_example.c
examples_gdbwire_CPPFLAGS = -I@GDBWIRE_ABS_TOP_SRCDIR@/src
examples_gdbwire_LDFLAGS =
examples_gdbwire_LDADD = libgdbwire.la

BUILT_SOURCES = \
    src/gdbmi_grammar.c \
    src/gdbmi_lexer.c

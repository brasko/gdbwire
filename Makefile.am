ACLOCAL_AMFLAGS = -I build/m4

noinst_PROGRAMS =
if WANT_TESTS
    noinst_PROGRAMS += test_suite
endif

lib_LTLIBRARIES=libgdbwire.la

# The gdbwire configuration
libgdbwire_la_SOURCES= \
    src/lib/gdbmi/gdbmi_grammar.y \
    src/lib/gdbmi/gdbmi_lexer.l \
    src/lib/gdbmi/gdbmi_parser.c \
    src/lib/gdbmi/gdbmi_pt.c \
    src/lib/gdbmi/gdbmi_pt_alloc.c \
    src/lib/gdbmi/gdbmi_pt_print.c \
    src/lib/containers/gdbwire_string.c \
    src/lib/logging/gdbwire_logger.c

libgdbwire_la_CFLAGS= \
    -I@GDBWIRE_ABS_TOP_SRCDIR@/src/lib \
    -I@GDBWIRE_ABS_TOP_BUILDDIR@/src/lib/gdbmi

# The test suite configuration
test_suite_SOURCES = \
    src/progs/test_suite/containers/gdbwire_string.cpp \
    src/progs/test_suite/fixture.cpp \
    src/progs/test_suite/gdbmi/gdbmi_parser.cpp \
    src/progs/test_suite/gdbmi/gdbmi_pt.cpp \
    src/progs/test_suite/main.cpp

BUILT_SOURCES = \
    src/lib/gdbmi/gdbmi_grammar.c \
    src/lib/gdbmi/gdbmi_lexer.c

test_suite_CPPFLAGS = \
    -I@GDBWIRE_ABS_TOP_SRCDIR@/src/progs/test_suite \
    -I@GDBWIRE_ABS_TOP_SRCDIR@/src/lib

test_suite_LDFLAGS =

test_suite_LDADD = \
    libgdbwire.la

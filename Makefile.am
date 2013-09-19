ACLOCAL_AMFLAGS = -I build/m4

noinst_PROGRAMS =
if WANT_TESTS
    noinst_PROGRAMS += test_suite
endif
noinst_PROGRAMS += gdbmi_driver

lib_LTLIBRARIES=libgdbwire.la

# The gdbwire configuration
libgdbwire_la_SOURCES= \
    src/lib/containers/gdbwire_string.c \
    src/lib/gdbmi/gdbmi_grammar.y \
    src/lib/gdbmi/gdbmi_lexer.l \
    src/lib/gdbmi/gdbmi_oc.c \
    src/lib/gdbmi/gdbmi_parser.c \
    src/lib/gdbmi/gdbmi_pt.c

# The test suite configuration
test_suite_SOURCES = \
    src/progs/test_suite/containers/gdbwire_string.cpp \
    src/progs/test_suite/fixture.cpp \
    src/progs/test_suite/gdbmi.cpp \
    src/progs/test_suite/main.cpp

test_suite_CPPFLAGS = \
    -I@GDBWIRE_ABS_TOP_SRCDIR@/src/progs/test_suite \
    -I@GDBWIRE_ABS_TOP_SRCDIR@/src/lib

test_suite_LDFLAGS =

test_suite_LDADD = \
    libgdbwire.la

# The gdbmi_driver configuration
gdbmi_driver_SOURCES = \
    src/progs/gdbmi/driver.c

gdbmi_driver_CPPFLAGS = \
    -I$(abs_top_srcdir)/src/lib

gdbmi_driver_LDFLAGS = \
    -L$(abs_top_builddir)/src/lib

gdbmi_driver_LDADD = libgdbwire.la

ACLOCAL_AMFLAGS = -I build/m4
EXTRA_DIST = 

noinst_PROGRAMS =
if WANT_TESTS
noinst_PROGRAMS += test_suite
endif

if WANT_EXAMPLES
noinst_PROGRAMS += examples/gdbmi
noinst_PROGRAMS += examples/gdbwire

if WANT_AMALGAMATION
noinst_PROGRAMS += examples/gdbwire_amalgamation
endif
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
test_suite_SOURCES = \
    src/progs/test_suite/catch.hpp \
    src/progs/test_suite/gdbwire_string.cpp \
    src/progs/test_suite/fixture.h \
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
EXTRA_DIST += src/progs/test_suite/data

# The gdbmi example configuration
examples_gdbmi_SOURCES = src/progs/examples/gdbmi_example.c
examples_gdbmi_CFLAGS = -I@GDBWIRE_ABS_TOP_SRCDIR@/src
examples_gdbmi_LDFLAGS =
examples_gdbmi_LDADD = libgdbwire.la

# The gdbwire example configuration
examples_gdbwire_SOURCES = src/progs/examples/gdbwire_example.c
examples_gdbwire_CFLAGS = -I@GDBWIRE_ABS_TOP_SRCDIR@/src
examples_gdbwire_LDFLAGS =
examples_gdbwire_LDADD = libgdbwire.la

BUILT_SOURCES = \
    src/gdbmi_grammar.c \
    src/gdbmi_lexer.c

if WANT_AMALGAMATION

# Compile the gdbwire example using the amalgamation if
# both options are present
examples_gdbwire_amalgamation_SOURCES = \
	src/progs/examples/gdbwire_example.c
# Do not want to distribute the generated files.
# Let the user create them if desired.
nodist_examples_gdbwire_amalgamation_SOURCES = \
	src/amalgamation/gdbwire.c
examples_gdbwire_amalgamation_CFLAGS = \
	-I@GDBWIRE_ABS_TOP_BUILDDIR@/src/amalgamation

# This will build both gdbwire.h and gdbwire.c.
BUILT_SOURCES += src/amalgamation/gdbwire.c
EXTRA_DIST += src/amalgamation/mkgdbwire.py

src/amalgamation/gdbwire.c: \
	src/gdbmi_grammar.c \
	src/gdbmi_lexer.c \
	src/amalgamation/mkgdbwire.py
	mkdir -p src/amalgamation && \
	cd src/amalgamation && \
	python \
		"@GDBWIRE_ABS_TOP_SRCDIR@/src/amalgamation/mkgdbwire.py" \
		"@PACKAGE_VERSION@" \
		"@GDBWIRE_ABS_TOP_SRCDIR@" \
		"@GDBWIRE_ABS_TOP_BUILDDIR@"
		
endif

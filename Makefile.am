ACLOCAL_AMFLAGS = -I build/m4
EXTRA_DIST = 

noinst_PROGRAMS =
if WANT_TESTS
noinst_PROGRAMS += test_suite
endif

if WANT_EXAMPLES
noinst_PROGRAMS += examples/gdbwire_mi
noinst_PROGRAMS += examples/gdbwire

if WANT_AMALGAMATION
noinst_PROGRAMS += examples/gdbwire_amalgamation
endif
endif

lib_LTLIBRARIES=libgdbwire.la

# The gdbwire configuration
libgdbwire_la_SOURCES= \
    src/gdbwire_mi_command.h \
    src/gdbwire_mi_command.c \
    src/gdbwire_mi_grammar.h \
    src/gdbwire_mi_grammar.y \
    src/gdbwire_mi_lexer.l \
    src/gdbwire_mi_parser.h \
    src/gdbwire_mi_parser.c \
    src/gdbwire_mi_pt.h \
    src/gdbwire_mi_pt.c \
    src/gdbwire_mi_pt_alloc.h \
    src/gdbwire_mi_pt_alloc.c \
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
    src/progs/test_suite/gdbwire_mi_command.cpp \
    src/progs/test_suite/gdbwire_mi_parser.cpp \
    src/progs/test_suite/gdbwire_mi_pt.cpp \
    src/progs/test_suite/gdbwire.cpp \
    src/progs/test_suite/main.cpp
test_suite_CPPFLAGS = \
    -I@GDBWIRE_ABS_TOP_SRCDIR@/src/progs/test_suite \
    -I@GDBWIRE_ABS_TOP_SRCDIR@/src
test_suite_LDFLAGS =
test_suite_LDADD = libgdbwire.la
EXTRA_DIST += src/progs/test_suite/data

# The gdbwire_mi example configuration
examples_gdbwire_mi_SOURCES = src/progs/examples/gdbwire_mi_example.c
examples_gdbwire_mi_CFLAGS = -I@GDBWIRE_ABS_TOP_SRCDIR@/src
examples_gdbwire_mi_LDFLAGS =
examples_gdbwire_mi_LDADD = libgdbwire.la

# The gdbwire example configuration
examples_gdbwire_SOURCES = src/progs/examples/gdbwire_example.c
examples_gdbwire_CFLAGS = -I@GDBWIRE_ABS_TOP_SRCDIR@/src
examples_gdbwire_LDFLAGS =
examples_gdbwire_LDADD = libgdbwire.la

BUILT_SOURCES = \
    src/gdbwire_mi_grammar.c \
    src/gdbwire_mi_lexer.c

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
	src/gdbwire_mi_grammar.c \
	src/gdbwire_mi_lexer.c \
	src/amalgamation/mkgdbwire.py
	mkdir -p src/amalgamation && \
	cd src/amalgamation && \
	python \
		"@GDBWIRE_ABS_TOP_SRCDIR@/src/amalgamation/mkgdbwire.py" \
		"@PACKAGE_VERSION@" \
		"@GDBWIRE_ABS_TOP_SRCDIR@" \
		"@GDBWIRE_ABS_TOP_BUILDDIR@"
		
endif

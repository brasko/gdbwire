AC_PREREQ(2.5)
AC_INIT([gdbwire], [1.0])
AC_CONFIG_AUX_DIR(build/build-aux)
AC_CONFIG_MACRO_DIR(build/m4)
AM_CONFIG_HEADER(build/config.h)

AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([subdir-objects])

dnl Checks for programs.
AC_LANG_CPLUSPLUS
AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_LEX
AC_PROG_YACC

AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL

dnl Checks for header files.
AC_HEADER_STDC

dnl Add support for automated testing.
dnl
dnl This uses the google test framework for automation.
dnl
dnl It enables the use of the C++ compiler and will create
dnl the extra test_suite program which executes all the
dnl automated tests.
GDBWIRE_ARG_ENABLE_DEFAULT_OFF([tests], [automated testing])

dnl Build the test suite if enable tests is true
AM_CONDITIONAL([WANT_TESTS], [test x$enable_tests = xyes])

dnl Find the absolute srcdir and builddir directories.
dnl Put those in the Makefile and config.h.
GDBWIRE_DIRECTORIES()

AC_OUTPUT(Makefile)

# Display configuration options.
cat <<EOF

$PACKAGE_NAME configuration summary:

    Install prefix ........... : ${prefix}
    C preprocessor ........... : ${CPP}
    C compiler ............... : ${CC}
    C++ preprocessor ......... : ${CXXCPP}
    C++ compiler ............. : ${CXX}
    Linker ................... : ${LD}

    C preprocessor flags ..... : ${CPPFLAGS}
    C compiler flags ......... : ${CFLAGS}
    C++ compiler flags ....... : ${CXXFLAGS}
    Linker flags ............. : ${LDFLAGS}
    Extra libraries .......... : ${LIBS}

    Enabled options:
    --enable-tests ........... : ${enable_tests}

EOF

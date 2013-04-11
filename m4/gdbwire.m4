dnl Add a new enable/disable option to the configure script.
dnl
dnl The first parameter will be the name of the option to add.
dnl For instance, a call to GDBWIRE_ARG_ENABLE([tests],[]) would create
dnl the configure option --enable-tests.
dnl
dnl The second parameter will be the help message provide when the
dnl user types configure --help.
dnl
dnl As the function name suggests, the default for this enable option is
dnl off (no). The cases supported by this macro are,
dnl default          no
dnl --enable-$1      yes
dnl --enable-$1=yes  yes
dnl --enable-$1=no   no
AC_DEFUN([GDBWIRE_ARG_ENABLE_DEFAULT_OFF], [
    AC_ARG_ENABLE([$1],
        AS_HELP_STRING([--enable-$1],
            [Enable $2 (default is no)]),
        [case "${enableval}" in
           yes) enable_$1=yes ;;
           no)  enable_$1=no ;;
           *) AC_MSG_ERROR([bad value ${enableval} for --enable-$1]) ;;
         esac],
        [enable_$1=no])
])

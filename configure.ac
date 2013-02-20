AC_PREREQ(2.61)
AC_INIT(libtclplus, 10.2-002, fox@nscl.msu.edu)

AC_CONFIG_SRCDIR([tclplus/TCLInterpreter.cpp])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.11])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LIBTOOL

# what to generate
AC_CONFIG_FILES([Makefile
                 exception/Makefile
                 tclplus/Makefile])

# do it
AC_OUTPUT

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

LT_INIT
AC_SUBST([LIBTOOL_DEPS])

# Makefile exports:
SOVERSION="10:0:0"
AC_SUBST(SOVERSION)

# specfic checks
AC_HEADER_STDC

#cppunit
### disabled for now, will have fox hook these back up
#AC_CHECK_LIB([cppunit], [_ZN7CppUnit12XmlOutputterD1Ev], [CPPUNIT_LDFLAGS=-lcppunit])

# Locate Tcl/Tk and setup compile/link switches for them too:
# The list below is the set of versions we will look for.  At this
# time, 8.4 is pretty standard and 8.5 is impending which means that
# soon some redhat systems will have it I suppose.
# This list must be updated as time goes by.
#
tcl_versions="8.5 8.4 8.3 8.2 8.1 8.0"     # First match;  so order new -> old.

#  The TCL Shell - used to run tests... must be in path.
#  First check for a version qualified command:
#

AC_MSG_CHECKING([for tcl shell command...])
for version in $tcl_versions
do
  name=tclsh${version}
  AC_CHECK_PROG(TCLSH_CMD, $name, $name)
done
#
#   Could be unqualified by version:
#
AC_CHECK_PROG(TCLSH_CMD, tclsh, tclsh)

if test "$TCLSH_CMD" == ""
then
  AC_MSG_ERROR([Can't find a tcl shell])
else
  AC_MSG_RESULT([found: $TCLSH_CMD])
fi

#  TCL headers:
#    We know of a few places the headers can live:
#       /usr/include              redhat linux.
#       /usr/include/tcl          debian dude that made  link to default tcl.
#       /usr/include/tcl<version> debian linux
#       /usr/local/include        default for source based install.
#       Whatever the user specifies. 
#        For now, the versions we look for are determined by the
#        variable tcl_versions  that variable is a list of version 
#        number specifiers.
#


tcl_header_dir=""
AC_MSG_CHECKING([for tcl headers])

AC_ARG_WITH(tcl-header-dir,
  [  --with-tcl-header-dir=path Path to tcl headers],
  [tcl_header_dir="$withval"
  tcl_headers_found="yes"],[tcl_headers_found="no"])


  # /usr/include

if test $tcl_headers_found == "no"; then
   AC_CHECK_FILE([/usr/include/tcl.h],
  [tcl_header_dir="/usr/include"
  tcl_headers_found="yes"])
fi

  # /usr/include/tcl

if test $tcl_headers_found == "no"; then
   AC_CHECK_FILE([/usr/include/tcl/tcl.h],
  [tcl_header_dir="/usr/include/tcl"
  tcl_headers_found="yes"])
fi

  # /usr/include/tcl<version>

if test $tcl_headers_found == "no" ; then
 for version in $tcl_versions;  do
   if test $tcl_headers_found == "no" ; then   # stop at first match
      testdir=/usr/include/tcl$version
      AC_CHECK_FILE([${testdir}/tcl.h],
  [tcl_header_dir=${testdir}
   tcl_headers_found="yes"])
   fi
 done
fi

      # /usr/local/include
if test $tcl_headers_found == "no"; then
   AC_CHECK_FILE([/usr/local/include/tcl.h],
  [tcl_header_dir="/usr/local/include/tcl"
  tcl_headers_found="yes"])
fi


if test $tcl_headers_found == "yes"; then
   TCL_FLAGS="-I${tcl_header_dir}"
   AC_MSG_RESULT([found in $tcl_header_dir])
else
   AC_MSG_ERROR([can't find tcl.h try using --with-tcl-header-dir to help me])
fi

#  Tcl libraries:
#     We look for the following:
#      /usr/lib/libtcl.a           - Usually a link to a default lib.
#      /usr/lib/libtcl<version>.a  - version of tcl from tcl_versions
#      /usr/local/lib/libtcl.a     - As above but installed from source.
#      /usr/local/lib<version>.a   - As Above but installed from source.
#      If --with-tcl-libdir=dir      is supplied, that directory is 
#                                    searched first.
# ASSUMPTIONS:  libtcl and libtk will have the same naming conventions.
#

tcl_libsearchpath=""
tcl_libfound="no"
AC_MSG_CHECKING([Tcl/Tk library to use])

  # Does the user want us to look somewhere in particular:

AC_ARG_WITH(tcl-libdir,
  [  --with-tcl-libdir=dir Specify where the tcl libraries live.],
  [tcl_libsearchpath=$withval])

tcl_libsearchpath="$tcl_libsearchpath /usr/lib /usr/local/lib"

tcl_libfiles=libtcl.a

for v in ${tcl_versions}
do
   tcl_libfiles="$tcl_libfiles libtcl${v}.a"
   tcl_libfiles="$tcl_libfiles libtcl${v}.so"
done


for dir in ${tcl_libsearchpath} 
do
  for file in ${tcl_libfiles}
  do
    fname=$dir/$file
    if test $tcl_libfound == "no" 
    then
      AC_CHECK_FILE($fname,
  [tcl_libdir=$dir
         tcl_libname=$file
   tcl_libfound="yes"
  ])
    fi
  done
done

if test $tcl_libfound == "yes"
then
  # Find the lib basename:

  tcl_libname=$(echo $tcl_libname|cut -c 4- )  # trim off lib 
  #
  #  Use basename to trim of the .a or .so:
  #

  tcl_libname=$(basename $(basename $tcl_libname .a) .so)
  echo tcl libname $tcl_libname



  tk_libname=tk$(echo $tcl_libname|cut -c 4-)   # trim off tc...and use tk

  TCL_LDFLAGS="-L${tcl_libdir} -l${tk_libname}  -l${tcl_libname}"

else
  AC_MSG_ERROR([Unable to find the tcl library, help me out with --with-tcl-libdir])
fi
#exports for Tcl for makefiles
AC_SUBST(TCL_FLAGS)
AC_SUBST(TCL_LDFLAGS)
AC_SUBST(TCLSH_CMD)

# what to generate
AC_CONFIG_FILES([Makefile
                 exception/Makefile
                 tclplus/Makefile])

# do it
AC_OUTPUT

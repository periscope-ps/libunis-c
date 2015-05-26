#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([libunis-c], [0.1], [https://github.com/periscope-ps/libunis-c])
AC_CONFIG_SRCDIR([src/unis_registration.c])
AM_INIT_AUTOMAKE([1.11 subdir-objects])
LT_INIT()

# Checks for programs.
AC_PROG_CC
AC_CONFIG_MACRO_DIR([config])
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL


# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([gettimeofday mempcpy memset strchr strdup strndup malloc])

# Checks for libraries.
AC_CHECK_LIB([pthread], [pthread_create], [],[AC_MSG_ERROR([Can not buiild without pthread])])

dnl check for libcurl package
AC_ARG_WITH(libcurl, [--with-libcurl=path   path to libcurl], ,
            with_libcurl=no)
if test "$with_libcurl" == "no"; then
   PKG_CHECK_MODULES([LIBCURL], [libcurl], [], [AC_MSG_ERROR([Can not buiild without libcurl])])
   AC_SUBST(LIBCURL_INC, "$LIBCURL_CFLAGS")
   AC_SUBST(LIBCURL_LIB, "$LIBCURL_LIBS")
else
   AC_MSG_CHECKING( for libcurl)
   if test -f "$with_libcurl/include/curl/curl.h"; then
   	   AC_MSG_RESULT(yes)
	   AC_SUBST(LIBCURL_INC, "-I$with_libcurl/include")
	   AC_SUBST(LIBCURL_LIB, "-L$with_libcurl/lib -lcurl")
   else
	   AC_MSG_RESULT(no)
	   AC_MSG_ERROR([Can not buiild without libcurl])
   fi
fi

dnl check for libjansson package
AC_ARG_WITH(libjansson, [--with-libjansson=path   path to libjansson], ,
            with_libjansson=no)
if test "$with_libjansson" == "no"; then
   PKG_CHECK_MODULES([JANSSON], [jansson], ,AC_MSG_ERROR([Can not buiild without libjansson]))		
   AC_SUBST(LIBJANSSON_INC, "$JANSSON_CFLAGS")
   AC_SUBST(LIBJANSSON_LIB, "$JANSSON_LIBS")

else
   AC_MSG_CHECKING( for libjansson)
   if test -f "$with_libjansson/include/jansson.h"; then
   	   AC_MSG_RESULT(yes)
	   AC_SUBST(LIBJANSSON_INC, "-I$with_libjansson/include")
	   AC_SUBST(LIBJANSSON_LIB, "-L$with_libjansson/lib -ljansson")
   else
	   AC_MSG_RESULT(no)
	   AC_MSG_ERROR([Can not buiild without libjansson])
   fi
fi

AC_CONFIG_FILES([Makefile src/Makefile include/Makefile libunis-c.pc])
AC_OUTPUT

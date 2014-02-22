#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([libunis-c], [0.1], [https://github.com/periscope-ps/libunis-c])
AC_CONFIG_SRCDIR([unis_registration.c])
AM_INIT_AUTOMAKE([libunis-c], [0.1])
LT_INIT()
# Checks for programs.
AC_PROG_CC
AC_CONFIG_MACRO_DIR([m4])
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL


# Checks for libraries.
AC_CHECK_LIB([curl], [curl_easy_init], [],[
         echo "curl library is required for this program"
         exit -1])
AC_CHECK_LIB([jansson], [json_object], [],[
         echo "jansson library is required for this program"
         exit -1])
AC_CHECK_LIB([pthread], [pthread_create], [],[
         echo "pthread library is required for this program"
         exit -1])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gettimeofday mempcpy memset strchr strdup strndup])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

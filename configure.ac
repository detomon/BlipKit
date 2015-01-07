#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([BlipKit],[0.11.3],[simon@monoxid.net],[blipkit],[http://blipkit.monoxid.net])

AC_CONFIG_SRCDIR([src/BKBase.c])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_RANLIB
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

check_headers="fcntl.h limits.h math.h stddef.h stdint.h stdlib.h string.h unistd.h"

AC_ARG_ENABLE([64-bit],
  [  --disable-64-bit        use only 32 bit integers],
  [with_64_bit=${enableval}],
  [with_64_bit=yes])

AC_ARG_ENABLE([examples],
  [  --disable-examples      do not build for running examples],
  [sdl_examples=${enableval}],
  [sdl_examples=yes])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T

if test "x${with_64_bit}" = xyes; then
  AC_TYPE_INT64_T
  AC_DEFINE(BK_USE_64_BIT, 1, [Define to 1 if configure had option --enable-64-bit])
else
  AC_DEFINE(BK_USE_64_BIT, 0, [Define to 1 if configure had option --enable-64-bit])
fi

if test "x${sdl_examples}" = xyes; then
  check_headers="$check_headers termios.h"
fi

# Checks for header files.
AC_CHECK_HEADERS($check_headers)

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memmove memset pow])

AC_CONFIG_FILES([
  Makefile
  src/Makefile
  examples/Makefile
  test/Makefile
])

# Define flags.
BK_STD_CFLAGS="-std=c99 -Wall -Wno-unused-function"

# Run tests.
AC_RUN_IFELSE([AC_LANG_SOURCE([[
int main () { return !((-15 >> 2) == -4); }
]])],[],[AC_MSG_ERROR(Host CPU seems not to handle shifting of negative values as expected)
],[])

# Set platform specific values.
AC_CANONICAL_HOST

case $host_os in
  darwin* )
      SDL_CFLAGS="-framework SDL"
      ;;
  *)
      SDL_CFLAGS="`sdl-config --cflags` `sdl-config --libs`"
     ;;
esac

if test "x${sdl_examples}" = xyes; then
  echo "using SDL linking flags: $SDL_CFLAGS"
  AC_SUBST(SDL_CFLAGS)
fi

AC_SUBST(BK_STD_CFLAGS)

AC_OUTPUT

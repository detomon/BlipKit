#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([BlipKit],[0.3.3],[simon@monoxid.net],[blipkit],[http://blipkit.monoxid.net])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

# Checks for programs.
AC_PROG_CC
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_PROG_RANLIB
AM_PROG_CC_C_O

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stddef.h stdint.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memmove memset])

AC_CONFIG_FILES([
 Makefile
 src/Makefile
 examples/Makefile
 dev/bliplay/Makefile
])

# Flags
BK_STD_CFLAGS="-std=c99 -Wall"

AC_SUBST(BK_STD_CFLAGS)

# Tests
AC_RUN_IFELSE([AC_LANG_SOURCE([[
int main () { return !((-15 >> 2) == -4); }
]])],[],[AC_MSG_ERROR(Host CPU seems not to handle shifting of negative values as expected)
],[])

# Platform specific
AC_CANONICAL_HOST

case $host_os in
  darwin* )
      SDL_CFLAGS="-framework SDL"
      ;;
  *)
      SDL_CFLAGS="`sdl-config --cflags` `sdl-config --libs`"
     ;;
esac

echo "using SDL linking flags: $SDL_CFLAGS"

AC_SUBST(SDL_CFLAGS)

AC_OUTPUT

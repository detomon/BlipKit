#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([BlipKit],[0.9],[simon@monoxid.net],[blipkit],[http://blipkit.monoxid.net])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AM_PROG_CC_C_O
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h limits.h stddef.h stdint.h stdlib.h string.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_CHECK_HEADER_STDBOOL
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memmove memset pow])

AC_CONFIG_FILES([
  Makefile
  src/Makefile
  examples/Makefile
  dev/cpp/Makefile
  dev/bfm/Makefile
  dev/fft/Makefile
  dev/step_phases/Makefile
  dev/tone_periods/Makefile
])

# Define flags.
BK_STD_CFLAGS="-std=c99 -Wall -Wno-unused-function"

AC_SUBST(BK_STD_CFLAGS)

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

echo "using SDL linking flags: $SDL_CFLAGS"

AC_SUBST(SDL_CFLAGS)

AC_OUTPUT

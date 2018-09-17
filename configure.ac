#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([BlipKit],[0.17.2],[simon@monoxid.net],[blipkit],[https://blipkit.audio])

AC_CONFIG_SRCDIR([src/BKBase.c])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])

m4_include([m4/common.m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

AM_CFLAGS="$CFLAGS"

# Define flags.
AM_CFLAGS="$AM_CFLAGS -Wall \
	-Wmissing-prototypes \
	-Wmissing-declarations \
	-Wstrict-prototypes \
	-Wformat-security \
	-Winline"

# Use C11 standard.
CHECK_COMPILE_FLAG([-std=c11], [AM_CFLAGS])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h math.h stdarg.h stddef.h stdint.h stdlib.h string.h unistd.h])

AC_ARG_WITH([sdl],
	AS_HELP_STRING([--without-sdl], [do not link agains SDL library]))

# Set default of with_sdl to yes.
AS_IF([test "x$with_sdl" = "x"],
	[with_sdl=yes],
	[])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UINT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_SIZE_T

# Check sizes.
AC_CHECK_SIZEOF(size_t)

if test "x${ac_cv_sizeof_size_t}" = x8; then
	AC_TYPE_INT64_T
	AC_TYPE_UINT64_T
fi

if test "x$with_sdl" = xyes; then
	AC_CHECK_HEADERS(termios.h)
fi

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memmove memset pow strdup])

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	examples/Makefile
	test/Makefile
])

# Run tests.
CHECK_SIGNED_SHIFT()

# Check for SDL.
if test "x$with_sdl" = xyes; then
	CHECK_SDL()
fi

AC_SUBST([AM_CFLAGS])

AC_OUTPUT

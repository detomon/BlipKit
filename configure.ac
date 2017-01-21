#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([BlipKit],[0.15.2],[simon@monoxid.net],[blipkit],[http://blipkit.audio])

AC_CONFIG_SRCDIR([src/BKBase.c])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])

m4_include([common.m4])

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
	-Wmissing-format-attribute \
	-Wno-shift-negative-value \
	-Winline"

CHECK_COMPILE_FLAG([-std=c11], [AM_CFLAGS])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stddef.h stdint.h stdlib.h string.h unistd.h])

AC_ARG_ENABLE([64-bit],
	[  --disable-64-bit        use only 32 bit integers],
	[with_64_bit=${enableval}],
	[with_64_bit=yes])

AC_ARG_ENABLE([examples],
	[  --disable-examples      do not build for running examples],
	[sdl_examples=${enableval}],
	[sdl_examples=yes])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UINT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_SIZE_T

if test "x${with_64_bit}" = xyes; then
	AC_TYPE_INT64_T
	AC_DEFINE(BK_USE_64_BIT, 1, [Define to 1 if configure had option --enable-64-bit])
else
	AC_DEFINE(BK_USE_64_BIT, 0, [Define to 1 if configure had option --enable-64-bit])
fi

if test "x${sdl_examples}" = xyes; then
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

if test "x${sdl_examples}" = xyes; then
	CHECK_SDL()
fi

AC_SUBST([AM_CFLAGS])

AC_OUTPUT

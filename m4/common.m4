# CHECK_COMPILE_FLAG(flags)
# -------------------------------------
# Checks if the compiler supports the given `flags`.
# Append them to the given argument on success.
# Abort with error if not supported.
m4_define([CHECK_COMPILE_FLAG], [dnl
_check_flags=$1
_saved_flags="$CFLAGS"
CFLAGS="$_check_flags"
AC_MSG_CHECKING([whether C compiler supports $_check_flags])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
	[AC_MSG_RESULT([yes])]
	[$2="${$2} $_check_flags"],
	[AC_MSG_FAILURE([C compiler seem not to support $_check_flags])]
)
CFLAGS="$_saved_flags"
])

# CHECK_COMPILE_FLAG_TRY(flags)
# -------------------------------------
# Check if the compiler supports the given `flags`.
# Append them to the given argument on success.
# Ignore error.
m4_define([CHECK_COMPILE_FLAG_TRY], [dnl
_check_flags=$1
_saved_flags="$CFLAGS"
CFLAGS="$_check_flags"
AC_MSG_CHECKING([whether C compiler supports $_check_flags])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
	[AC_MSG_RESULT([yes])]
	[$2="${$2} $_check_flags"],
	[AC_MSG_RESULT([no])]
)
CFLAGS="$_saved_flags"
])

# CHECK_SIGNED_SHIFT()
# --------------------
# Checks if the host CPU handles left shifting negative values correctly.
m4_define([CHECK_SIGNED_SHIFT], [dnl
AC_MSG_CHECKING([whether host CPU handles left shifting negative values as expected])
AC_RUN_IFELSE([AC_LANG_SOURCE([[
	int main () {
		return !((-15 >> 2) == -4);
	}
]])], [
	AC_MSG_RESULT(yes)
], [
	AC_MSG_RESULT(no)
	AC_MSG_FAILURE(Host CPU seems not to handle shifting of negative values as expected)
])
])

# CHECK_SDL()
# -----------
# Search for installed SDL libraries or frameworks.
# Choose the latest version if found.
# Outputs SDL_CFLAGS, SDL_LDADD, SDL_CONFIG_NAME and SDL_VERSION.
m4_define([CHECK_SDL], [dnl
SDL_CFLAGS="$SDL_CFLAGS"
SDL_LDADD="$SDL_LDADD"
SDL_CONFIG_NAME=
SDL_VERSION=

# Set platform specific values.
AC_CANONICAL_HOST

if test "x$SDL_CFLAGS" = x; then
	AC_CHECK_PROG(SDL2_CONFIG_CHECK, sdl2-config, yes)

	# Set SDL config version.
	if test "x$SDL2_CONFIG_CHECK" = xyes; then
		AC_DEFINE(BK_SDL_VERSION, 2, [Defines SDL version])
		SDL_CONFIG_NAME="sdl2-config"
	fi

	# Set SDL flags from sdl-config if found.
	if test "x$SDL_CONFIG_NAME" != x; then
		SDL_CFLAGS="`${SDL_CONFIG_NAME} --cflags`"
		SDL_LDADD="`${SDL_CONFIG_NAME} --libs`"
	fi

	# Output SDL_CFLAGS.
	if test "x$SDL_CFLAGS" != x; then
		echo "using SDL linking flags: $SDL_CFLAGS $SDL_LDADD"
	else
		AC_MSG_ERROR([SDL was requested with --with-sdl, but SDL was not found. Use --without-sdl to disable usage of SDL])
	fi

	AC_SUBST(SDL_CFLAGS)
	AC_SUBST(SDL_LDADD)
	AC_SUBST(SDL_VERSION)
else
	echo "using user defined SDL linking flags: $SDL_CFLAGS"
fi
])

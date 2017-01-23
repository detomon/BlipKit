# CHECK_COMPILE_FLAG(flags)
# -------------------------------------
# Checks if the compiler supports the given `flags`.
# Append them to `CFLAGS` on success.
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
# Outputs SDL_CFLAGS, SDL_CONFIG_NAME, SDL_NAME and SDL_VERSION.
m4_define([CHECK_SDL], [dnl
SDL_CFLAGS="$SDL_CFLAGS"
SDL_CONFIG_NAME=
SDL_NAME=
SDL_VERSION=

# Set platform specific values.
AC_CANONICAL_HOST

if test "x$SDL_CFLAGS" = x; then
	# Set platform specific values.
	AC_CANONICAL_HOST

	# Checks for SDL2.
	AC_CHECK_LIB([SDL2], [SDL_Init], [
		AC_CHECK_PROG(SDL2_CONFIG_CHECK, sdl2-config, yes)
	])

	# Checks for SDL.
	AC_CHECK_LIB([SDL], [SDL_Init], [
		AC_CHECK_PROG(SDL_CONFIG_CHECK, sdl-config, yes)
	])

	# Checks for SDL2 headers.
	AC_CHECK_HEADERS([SDL2/SDL.h], [
		SDL_VERSION=2
		AC_DEFINE(BK_SDL_VERSION, 2, [Defines SDL version])
	])

	# Checks for SDL headers.
	if test "x$SDL_VERSION" = x; then
		AC_CHECK_HEADERS([SDL/SDL.h], [
			SDL_VERSION=1
			AC_DEFINE(BK_SDL_VERSION, 1, [Defines SDL version])
		])
	fi

	# Set SDL version if headers found.
	if test "x$SDL_VERSION" != x; then
		SDL_NAME="SDL$SDL_VERSION"
	fi

	# Search for sdl-config.
	if test "x$SDL2_CONFIG_CHECK" = xyes; then
		SDL_CONFIG_NAME="sdl2-config"
	elif test "x$SDL_CONFIG_CHECK" = xyes; then
		SDL_CONFIG_NAME="sdl-config"
	fi

	# Set SDL flags from sdl-config if found.
	if test "x$SDL_CONFIG_NAME" != x; then
		SDL_CFLAGS="`${SDL_CONFIG_NAME} --cflags` `${SDL_CONFIG_NAME} --libs`"
	fi

	# Guess SDL flags from platform
	if test "x$SDL_CFLAGS" = x; then
		case $host_os in
			darwin*)
				SDL_CFLAGS="-framework $SDL_NAME"
				;;
			*)
				SDL_CFLAGS="-I/usr/local/include/SDL2 -D_THREAD_SAFE -L/usr/local/lib -lSDL2"
				echo "using default linking flags for SDL"
				;;
		esac
	fi

	# Output SDL_CFLAGS.
	if test "x$SDL_CFLAGS" != x; then
		echo "using SDL version: $SDL_NAME"
		echo "using SDL linking flags: $SDL_CFLAGS"
	fi

	AC_SUBST(SDL_CFLAGS)
	AC_SUBST(SDL_CONFIG_NAME)
	AC_SUBST(SDL_NAME)
	AC_SUBST(SDL_VERSION)
else
	echo "using user defined SDL linking flags: $SDL_CFLAGS"
fi
])

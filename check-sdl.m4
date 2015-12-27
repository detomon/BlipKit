SDL_CFLAGS=
SDL_CONFIG_NAME=

# Checks for libraries.
AC_CHECK_LIB([SDL2], [SDL_Init], [
	AC_CHECK_PROG(SDL2_CONFIG_CHECK, sdl2-config, yes)
])

AC_CHECK_LIB([SDL], [SDL_Init], [
	AC_CHECK_PROG(SDL_CONFIG_CHECK, sdl-config, yes)
])

if test "x$SDL2_CONFIG_CHECK" = xyes; then
	SDL_CONFIG_NAME="sdl2-config"
elif test "x$SDL_CONFIG_CHECK" = xyes; then
	SDL_CONFIG_NAME="sdl-config"
fi

if test "x$SDL_CONFIG_NAME" != x; then
	SDL_CFLAGS="`${SDL_CONFIG_NAME} --cflags` `${SDL_CONFIG_NAME} --libs`"
fi

if test "x$SDL_CFLAGS" = x; then
	case $host_os in
		darwin*)
			SDL_CFLAGS="-framework SDL2"
			;;
		*)
			echo "don't know how to link SDL library"
			;;
	esac
fi

if test "x$SDL_CFLAGS" != x; then
	echo "using SDL linking flags: $SDL_CFLAGS"
fi

AC_SUBST(SDL_CFLAGS)

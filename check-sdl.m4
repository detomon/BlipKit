SDL_CFLAGS=
SDL_CONFIG_NAME=
SDL_NAME=
SDL_VERSION=

# Checks for libraries.
AC_CHECK_LIB([SDL2], [SDL_Init], [
	AC_CHECK_PROG(SDL2_CONFIG_CHECK, sdl2-config, yes)
])

AC_CHECK_LIB([SDL], [SDL_Init], [
	AC_CHECK_PROG(SDL_CONFIG_CHECK, sdl-config, yes)
])

AC_CHECK_HEADERS([SDL2/SDL.h], [
	SDL_VERSION=2
	AC_DEFINE(BK_SDL_VERSION, 2, [Defines SDL version])
])

if test "x$SDL_VERSION" = x; then
	AC_CHECK_HEADERS([SDL/SDL.h], [
		SDL_VERSION=1
		AC_DEFINE(BK_SDL_VERSION, 1, [Defines SDL version])
	])
fi

if test "x$SDL_VERSION" != x; then
	SDL_NAME="SDL$SDL_VERSION"
fi

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
			SDL_CFLAGS="-framework $SDL_NAME"
			;;
		*)
			echo "don't know how to link SDL library"
			;;
	esac
fi

if test "x$SDL_CFLAGS" != x; then
	echo "using SDL version: $SDL_NAME"
	echo "using SDL linking flags: $SDL_CFLAGS"
fi

AC_SUBST(SDL_CFLAGS)

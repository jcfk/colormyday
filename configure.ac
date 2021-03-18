# initialize

AC_PREREQ(2.59)
AC_INIT([colormyday], 0.9.0)
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_PROG_CC

AC_CONFIG_SRCDIR([src/colormyday.c])
#AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile docs/Makefile])

# program check

AC_CHECK_PROGS(var, xml2-config)
if test n$var = "n" ; then
	AC_MSG_ERROR([xml2-config is required to build colormyday])

fi

LIBS="${LIBS} $(xml2-config --libs)"
AC_SUBST(xmlcflags, "$(xml2-config --cflags)")

# library check

# fix this
AC_CHECK_LIB(ncurses, initscr, [
		LIBS="${LIBS} -lncurses"

	], [
		AC_MSG_ERROR([ncurses is required to build colormyday])

	]
)

AC_CHECK_LIB(pthread, pthread_create, [
		LIBS="${LIBS} -pthread"

	], [
		AC_MSG_ERROR([pthread is required to build colormyday])

	]
)

AC_CHECK_LIB(m, floor,[
		LIBS="${LIBS} -lm"

	], [
		AC_MSG_ERROR([math is required to build colormyday])

	]
)


# header check

# libxml2/libxml/parser.h libxml2/libxml/tree.h ??
AC_CHECK_HEADERS([ \
	ncurses.h \
	pthread.h \
	math.h \
	stdio.h stdlib.h signal.h time.h unistd.h dirent.h string.h \
	locale.h wchar.h limits.h stdarg.h ctype.h regex.h sys/time.h \
	sys/stat.h sys/types.h
])


# make makefiles

AC_OUTPUT


# epilogue

AC_INIT([canner], m4_esyscmd_s([cat VERSION]), [cliff.snyder@gmail.com])
AC_CHECK_HEADERS(dirent.h limits.h popt.h stdio.h stdlib.h string.h sys/types.h unistd.h,
  [],
  [
    AC_MSG_ERROR([one or more required headers not found])
  ])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
LT_INIT

AC_SEARCH_LIBS([poptGetContext], [popt], [], [
  AC_MSG_ERROR([unable to find libpopt])
])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile examples/Makefile test/Makefile])
AC_CONFIG_SUBDIRS([examples])
AC_OUTPUT

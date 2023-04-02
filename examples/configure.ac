AC_INIT([canner-examples], [0.1])
AC_CHECK_HEADERS(event2/buffer.h event2/event.h event2/http.h getopt.h \
  signal.h stdio.h stdlib.h string.h time.h unistd.h,
  [],
  [
    AC_MSG_ERROR([one or more required headers not found])
  ])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
LT_INIT

AC_SEARCH_LIBS([event_base_dispatch], [event], [], [
  AC_MSG_ERROR([unable to find libevent])
])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

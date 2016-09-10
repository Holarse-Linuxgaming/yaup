dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_INIT(yaup, 0.1)

AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.11])

AM_SILENT_RULES([yes])

AC_PROG_CC


AC_PROG_CPP
AC_PROG_CXX

AC_CHECK_LIB([miniupnpc], [upnpDiscover], [], [AC_MSG_ERROR([miniupnpc was not found])])

dnl ***************************************************************************
dnl Internationalization
dnl ***************************************************************************
IT_PROG_INTLTOOL([0.35.0])

GETTEXT_PACKAGE=yaup
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])

AM_GLIB_GNU_GETTEXT





LT_INIT


PKG_CHECK_MODULES(YAUP, [gtk+-3.0 ])




AC_OUTPUT([
Makefile
src/Makefile
po/Makefile.in
src/gtk/Makefile
])

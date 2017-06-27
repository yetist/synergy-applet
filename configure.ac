dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_INIT(synergy_applet, 0.9, http://code.google.com/p/synergy-applet/)

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC




dnl ***************************************************************************
dnl Internatinalization
dnl ***************************************************************************
GETTEXT_PACKAGE=synergy_applet
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])
AM_GLIB_GNU_GETTEXT
IT_PROG_INTLTOOL([0.35.0])



AM_PROG_LIBTOOL

#PKG_CHECK_MODULES(SYNERGY_APPLET, [libgnome-2.0 >= 2.14 libgnomeui-2.0 >= 2.14 avahi-client >= 0.6.0])
PKG_CHECK_MODULES(SYNERGY_APPLET, [glib-2.0 >= 2.14 gtk+-2.0 avahi-client >= 0.6.0 avahi-glib])
AC_SUBST(SYNERGY_APPLET_CFLAGS)
AC_SUBST(SYNERGY_APPLET_LIBS)




AC_OUTPUT([
Makefile
src/Makefile
po/Makefile.in
])

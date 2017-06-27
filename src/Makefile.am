## Process this file with automake to produce Makefile.in

## Created by Anjuta



AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(SYNERGY_APPLET_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = synergy_applet

synergy_applet_SOURCES = \
	main.c          \
	browse.c          \
	browse.h          \
	publish.c          \
	publish.h          \
	synergy-applet.c          \
	synergy-applet.h          \
	settings.c  \
	settings.h

synergy_applet_LDFLAGS = \
	-Wl,--export-dynamic

synergy_applet_LDADD = $(SYNERGY_APPLET_LIBS)

EXTRA_DIST = $(glade_DATA)

## Process this file with automake to produce Makefile.in

## Created by Anjuta


uidir = $(pkgdatadir)/ui
ui_DATA = yaup.ui \
	yaup-light.png \
	yaup-dark.png \
	gtk/menus.ui


AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(YAUP_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g\
	 -DRESOURCE=FALSE\
	 -DDEBUG=FALSE

bin_PROGRAMS = yaup

yaup_SOURCES = \
	main.c \
	yaup.c \
	yaup.h \
	callbacks.c \
	callbacks.h \
	upnpcommunication.c \
	upnpcommunication.h \
	io.c \
	io.h \
	listboxthings.c \
	listboxthings.h \
	yaup-i18n.h


yaup_LDFLAGS = \
	-Wl,--export-dynamic

yaup_LDADD = $(YAUP_LIBS)




EXTRA_DIST = $(ui_DATA)

# Remove ui directory on uninstall
uninstall-local:
	-rm -r $(uidir)
	-rm -r $(pkgdatadir)


SUBDIRS = \
	gtk

/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * main.c
 *
 * Copyright (C) 2016 Lucki <lucki@holarse-linuxgaming.de>
 *
 * yaup is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * yaup is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>
#include <string.h>
#include <libintl.h>
#include <locale.h>

#include "yaup.h"
#include "callbacks.h"

int
main (int argc, char *argv[])
{
	Yaup *yaup;
	int status;

	/* This is our main handler. Everything important goes into this struct.
	 * We're passing it around through the whole application so we can access
	 * all variables easily at will.
	 */
	yaup = g_new (Yaup, 1);

#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
	debug_print("Getting locale for %s from %s\n", GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
#endif

	/* This is our application. We're using it for some automated stuff like
	 * the app-menu.
	 */
	yaup->application = gtk_application_new ("de.holarse-linuxgaming.yaup",
																					 G_APPLICATION_FLAGS_NONE);

	/* We're connecting the main siganls to our specific functions.
	 * startup = initialization
	 * activate = window stuff
	 * shutdown = save to file
	 */
	g_signal_connect (yaup->application, "startup",
										G_CALLBACK (yaup_init), yaup);
	g_signal_connect (yaup->application, "activate",
										G_CALLBACK (activate), yaup);
	g_signal_connect (yaup->application, "shutdown",
										G_CALLBACK (app_shutdown), yaup);

	/* Setting this for safety, so we can't access GET_UI_ELEMENT before it's
	 * builder is initialized
	 */
	yaup->definitions = NULL;

	/* Starting our application with the command line arguments.
	 * After this point everything gets called from the main gtk loop through
	 * signal handlers.
	 */
	status = g_application_run (G_APPLICATION (yaup->application), argc, argv);
  g_object_unref (yaup->application);

	/* Here we are after shutdown, returning our status.
	 */
	return status;
}
